// ------------------------------------------------
// KmlOverlay.hpp の実装
// ------------------------------------------------

#include "KmlOverlay.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QXmlStreamReader>

#include <miniz.h>

namespace {
    constexpr qint64 MaximumContainerBytes = 32 * 1024 * 1024;
    constexpr qint64 MaximumKmlBytes = 16 * 1024 * 1024;
    constexpr mz_uint MaximumArchiveEntries = 2048;
    constexpr size_t MaximumFeatures = 4096;
    constexpr size_t MaximumCoordinates = 200000;
    constexpr int MaximumXmlDepth = 64;

    [[noreturn]] void fail(const QString& message) {
        throw std::runtime_error(message.toStdString());
    }

    QString tag(const QDomElement& element) {
        return element.localName().isEmpty() ? element.tagName().section(':', -1) : element.localName();
    }

    QDomElement child(const QDomElement& parent, const QString& name) {
        for (auto element = parent.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {
            if (tag(element) == name) return element;
        }
        return {};
    }

    QString childText(const QDomElement& parent, const QString& name) {
        return child(parent, name).text().trimmed();
    }

    void collectElements(const QDomElement& parent, const QString& name,
                         QVector<QDomElement>& output, int depth = 0) {
        if (depth > MaximumXmlDepth) fail("KML nesting is too deep.");
        if (tag(parent) == name) output.push_back(parent);
        if (output.size() > static_cast<qsizetype>(MaximumFeatures)) fail("KML contains too many features or styles.");
        for (auto element = parent.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {
            collectElements(element, name, output, depth + 1);
        }
    }

    QColor readColor(const QString& text) {
        bool valid = false;
        const auto value = text.toUInt(&valid, 16);
        if (!valid || text.size() != 8) fail("KML color must use eight hexadecimal aabbggrr digits.");
        return QColor(static_cast<int>(value & 0xff), static_cast<int>((value >> 8) & 0xff),
                      static_cast<int>((value >> 16) & 0xff), static_cast<int>((value >> 24) & 0xff));
    }

    double readNonnegative(const QString& text, const QString& field) {
        bool valid = false;
        const double value = text.toDouble(&valid);
        if (!valid || !std::isfinite(value) || value < 0.0 || value > 100.0) {
            fail("KML " + field + " must be a finite value between 0 and 100.");
        }
        return value;
    }

    gui::KmlStyle readStyle(const QDomElement& element, gui::KmlStyle style = {}) {
        const auto line = child(element, "LineStyle");
        if (!child(line, "color").isNull()) style.lineColor = readColor(childText(line, "color"));
        if (!child(line, "width").isNull()) style.lineWidth = readNonnegative(childText(line, "width"), "line width");
        const auto polygon = child(element, "PolyStyle");
        if (!child(polygon, "color").isNull()) style.fillColor = readColor(childText(polygon, "color"));
        if (!child(polygon, "fill").isNull()) style.fill = childText(polygon, "fill") != "0";
        if (!child(polygon, "outline").isNull()) style.outline = childText(polygon, "outline") != "0";
        const auto icon = child(element, "IconStyle");
        if (!child(icon, "color").isNull()) style.pointColor = readColor(childText(icon, "color"));
        if (!child(icon, "scale").isNull()) style.pointScale = readNonnegative(childText(icon, "scale"), "icon scale");
        const auto label = child(element, "LabelStyle");
        if (!child(label, "scale").isNull()) style.showLabel = readNonnegative(childText(label, "scale"), "label scale") != 0.0;
        // Icons and NetworkLinks are not downloaded. Point markers provide a local fallback.
        return style;
    }

    using StyleElements = QHash<QString, QDomElement>;

    gui::KmlStyle resolveStyle(const QString& reference, const StyleElements& styles,
                              const StyleElements& maps, int depth = 0) {
        if (depth > 8) fail("KML StyleMap has a circular or overly deep reference.");
        if (!reference.startsWith('#')) return {}; // external style URLs are never fetched
        const QString id = reference.mid(1);
        if (styles.contains(id)) return readStyle(styles.value(id));
        const auto map = maps.value(id);
        for (auto pair = map.firstChildElement(); !pair.isNull(); pair = pair.nextSiblingElement()) {
            if (tag(pair) == "Pair" && childText(pair, "key") == "normal") {
                if (!child(pair, "Style").isNull()) return readStyle(child(pair, "Style"));
                return resolveStyle(childText(pair, "styleUrl"), styles, maps, depth + 1);
            }
        }
        return {};
    }

    QVector<QPointF> readCoordinates(const QDomElement& element, size_t& coordinateCount) {
        QVector<QPointF> output;
        // Read one coordinate at a time: oversized input cannot allocate an unbounded list.
        auto values = QRegularExpression("\\S+").globalMatch(childText(element, "coordinates"));
        while (values.hasNext()) {
            if (coordinateCount >= MaximumCoordinates) fail("KML contains too many coordinates.");
            ++coordinateCount;
            const auto value = values.next().captured();
            const auto parts = value.split(',');
            if (parts.size() < 2 || parts.size() > 3) fail("KML coordinate must use longitude,latitude[,altitude].");
            bool longitudeValid = false, latitudeValid = false;
            const double longitude = parts[0].toDouble(&longitudeValid);
            const double latitude = parts[1].toDouble(&latitudeValid);
            if (!longitudeValid || !latitudeValid || !std::isfinite(longitude) || !std::isfinite(latitude)
                || longitude < -180.0 || longitude > 180.0 || latitude < -90.0 || latitude > 90.0) {
                fail("KML longitude/latitude is invalid or outside its supported range.");
            }
            if (parts.size() == 3) {
                bool altitudeValid = false;
                const double altitude = parts[2].toDouble(&altitudeValid);
                if (!altitudeValid || !std::isfinite(altitude)) fail("KML altitude must be finite.");
            }
            output.push_back({longitude, latitude});
        }
        return output;
    }

    QVector<QPointF> readRing(const QDomElement& boundary, size_t& coordinateCount) {
        auto ring = readCoordinates(child(boundary, "LinearRing"), coordinateCount);
        if (ring.size() < 4 || ring.first() != ring.last()) fail("KML polygon rings must contain at least four coordinates and be closed.");
        return ring;
    }

    void readGeometry(const QDomElement& element, const QString& name, const gui::KmlStyle& style,
                      gui::KmlOverlay& overlay, size_t& coordinateCount, int depth = 0) {
        if (depth > MaximumXmlDepth) fail("KML geometry nesting is too deep.");
        const auto geometryName = tag(element);
        if (geometryName == "MultiGeometry") {
            for (auto geometry = element.firstChildElement(); !geometry.isNull(); geometry = geometry.nextSiblingElement()) {
                readGeometry(geometry, name, style, overlay, coordinateCount, depth + 1);
            }
            return;
        }
        gui::KmlFeature feature;
        feature.name = name;
        feature.style = style;
        if (geometryName == "Point") {
            feature.geometry = gui::KmlGeometry::Point;
            feature.coordinates = readCoordinates(element, coordinateCount);
            if (feature.coordinates.size() != 1) fail("KML Point must contain exactly one coordinate.");
        } else if (geometryName == "LineString") {
            feature.geometry = gui::KmlGeometry::LineString;
            feature.coordinates = readCoordinates(element, coordinateCount);
            if (feature.coordinates.size() < 2) fail("KML LineString must contain at least two coordinates.");
        } else if (geometryName == "Polygon") {
            feature.geometry = gui::KmlGeometry::Polygon;
            feature.coordinates = readRing(child(element, "outerBoundaryIs"), coordinateCount);
            for (auto boundary = element.firstChildElement(); !boundary.isNull(); boundary = boundary.nextSiblingElement()) {
                if (tag(boundary) == "innerBoundaryIs") feature.holes.push_back(readRing(boundary, coordinateCount));
            }
        } else {
            return;
        }
        if (overlay.features.size() >= MaximumFeatures) fail("KML contains too many geometries.");
        overlay.features.push_back(std::move(feature));
    }

    struct ZipReader {
        mz_zip_archive archive{};
        bool initialized = false;
        ~ZipReader() { if (initialized) mz_zip_reader_end(&archive); }
    };

    QByteArray readKmz(const QByteArray& container) {
        ZipReader zip;
        zip.initialized = mz_zip_reader_init_mem(&zip.archive, container.constData(),
                                                static_cast<size_t>(container.size()), 0) != 0;
        if (!zip.initialized) fail("KMZ is not a readable ZIP archive.");
        const auto count = mz_zip_reader_get_num_files(&zip.archive);
        if (count > MaximumArchiveEntries) fail("KMZ contains too many archive entries.");
        int selected = -1;
        for (mz_uint index = 0; index < count; ++index) {
            mz_zip_archive_file_stat entry{};
            if (!mz_zip_reader_file_stat(&zip.archive, index, &entry)) fail("KMZ archive directory is invalid.");
            const auto name = QString::fromUtf8(entry.m_filename);
            if (!entry.m_is_directory && name.endsWith(".kml", Qt::CaseInsensitive)) {
                if (selected < 0) selected = static_cast<int>(index);
                if (name.compare("doc.kml", Qt::CaseInsensitive) == 0) {
                    selected = static_cast<int>(index);
                    break;
                }
            }
        }
        if (selected < 0) fail("KMZ contains no KML document.");
        mz_zip_archive_file_stat entry{};
        if (!mz_zip_reader_file_stat(&zip.archive, static_cast<mz_uint>(selected), &entry)
            || entry.m_is_encrypted || !entry.m_is_supported) fail("KMZ KML entry is encrypted or uses an unsupported compression method.");
        if (entry.m_uncomp_size == 0 || entry.m_uncomp_size > static_cast<mz_uint64>(MaximumKmlBytes)) {
            fail("KMZ KML document is empty or exceeds 16 MiB.");
        }
        QByteArray xml(static_cast<qsizetype>(entry.m_uncomp_size), Qt::Uninitialized);
        if (!mz_zip_reader_extract_to_mem(&zip.archive, static_cast<mz_uint>(selected), xml.data(),
                                         static_cast<size_t>(xml.size()), 0)) fail("KMZ KML decompression or checksum validation failed.");
        return xml;
    }
}

gui::KmlOverlay gui::parseKmlOverlay(const QByteArray& xml, const QString& name) {
    if (xml.isEmpty() || xml.size() > MaximumKmlBytes) fail("KML document is empty or exceeds 16 MiB.");
    QXmlStreamReader validation(xml);
    validation.setEntityExpansionLimit(4096);
    int depth = 0;
    while (!validation.atEnd()) {
        const auto token = validation.readNext();
        if (token == QXmlStreamReader::DTD) fail("KML documents containing a DTD are unsupported.");
        if (token == QXmlStreamReader::StartElement && ++depth > MaximumXmlDepth) fail("KML nesting is too deep.");
        if (token == QXmlStreamReader::EndElement) --depth;
    }
    if (validation.hasError()) fail("Invalid KML XML: " + validation.errorString());
    QDomDocument document;
    const auto parsed = document.setContent(xml, QDomDocument::ParseOption::UseNamespaceProcessing);
    if (!parsed) fail(QString("Invalid KML at line %1: %2").arg(parsed.errorLine).arg(parsed.errorMessage));
    const auto root = document.documentElement();
    if (tag(root) != "kml" || (!root.namespaceURI().isEmpty() && root.namespaceURI() != "http://www.opengis.net/kml/2.2")) {
        fail("Expected a KML 2.2 document.");
    }
    StyleElements styles, maps;
    QVector<QDomElement> elements;
    collectElements(root, "Style", elements);
    for (const auto& element : elements) if (!element.attribute("id").isEmpty()) styles.insert(element.attribute("id"), element);
    elements.clear();
    collectElements(root, "StyleMap", elements);
    for (const auto& element : elements) if (!element.attribute("id").isEmpty()) maps.insert(element.attribute("id"), element);
    elements.clear();
    collectElements(root, "Placemark", elements);
    KmlOverlay overlay{name, {}};
    size_t coordinateCount = 0;
    for (const auto& placemark : elements) {
        auto style = resolveStyle(childText(placemark, "styleUrl"), styles, maps);
        if (!child(placemark, "Style").isNull()) style = readStyle(child(placemark, "Style"), style);
        for (auto geometry = placemark.firstChildElement(); !geometry.isNull(); geometry = geometry.nextSiblingElement()) {
            readGeometry(geometry, childText(placemark, "name"), style, overlay, coordinateCount);
        }
    }
    if (overlay.features.empty()) fail("KML contains no supported Point, LineString, or Polygon geometry.");
    return overlay;
}

gui::KmlOverlay gui::loadKmlOverlay(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) fail("Cannot read " + path + ": " + file.errorString());
    if (file.size() > MaximumContainerBytes) fail("KML/KMZ file exceeds 32 MiB: " + path);
    const auto content = file.readAll();
    if (file.error() != QFileDevice::NoError) fail("Cannot read " + path + ": " + file.errorString());
    try {
        const auto xml = path.endsWith(".kmz", Qt::CaseInsensitive) ? readKmz(content) : content;
        return parseKmlOverlay(xml, QFileInfo(path).fileName());
    } catch (const std::exception& error) {
        fail(QFileInfo(path).fileName() + ": " + QString::fromUtf8(error.what()));
    }
}
