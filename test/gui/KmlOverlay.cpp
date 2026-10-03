#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

#include <QFile>
#include <QTemporaryDir>

#include <miniz.h>

#include "gui/map/KmlOverlay.hpp"

namespace {
    QByteArray document(const QByteArray& content) {
        return "<kml xmlns=\"http://www.opengis.net/kml/2.2\"><Document>" + content + "</Document></kml>";
    }

    const QByteArray Point = "<Placemark><name>Launcher</name><Point><coordinates>140.01,40.24,5</coordinates></Point></Placemark>";

    // Generate a tiny archive in memory, so tests never need the user's KMZ files.
    QByteArray archive(const std::vector<std::pair<const char*, QByteArray>>& entries) {
        mz_zip_archive zip{};
        if (!mz_zip_writer_init_heap(&zip, 0, 0)) throw std::runtime_error("Test ZIP initialization failed.");
        for (const auto& [name, data] : entries) {
            if (!mz_zip_writer_add_mem(&zip, name, data.constData(), static_cast<size_t>(data.size()), MZ_BEST_COMPRESSION)) {
                mz_zip_writer_end(&zip);
                throw std::runtime_error("Test ZIP entry creation failed.");
            }
        }
        void* memory = nullptr;
        size_t size = 0;
        const bool completed = mz_zip_writer_finalize_heap_archive(&zip, &memory, &size) != 0;
        mz_zip_writer_end(&zip);
        if (!completed) throw std::runtime_error("Test ZIP finalization failed.");
        QByteArray result(static_cast<const char*>(memory), static_cast<qsizetype>(size));
        mz_free(memory);
        return result;
    }

    QString write(const QTemporaryDir& directory, const QString& name, const QByteArray& content) {
        const auto path = directory.filePath(name);
        QFile file(path);
        REQUIRE(file.open(QIODevice::WriteOnly));
        REQUIRE(file.write(content) == content.size());
        return path;
    }
}

TEST_CASE("KML preserves normal styles, polygon holes and MultiGeometry", "[gui][map]") {
    const auto xml = document(R"(
        <Style id="normal">
          <LineStyle><color>ff0000ff</color><width>3</width></LineStyle>
          <PolyStyle><color>8000ff00</color><fill>1</fill><outline>0</outline></PolyStyle>
          <IconStyle><color>ffff0000</color><scale>2</scale></IconStyle>
          <LabelStyle><scale>0</scale></LabelStyle>
        </Style>
        <Style id="highlight"><LineStyle><width>9</width></LineStyle></Style>
        <StyleMap id="mapped"><Pair><key>highlight</key><styleUrl>#highlight</styleUrl></Pair>
          <Pair><key>normal</key><styleUrl>#normal</styleUrl></Pair></StyleMap>
        <Placemark><name>Safety area</name><styleUrl>#mapped</styleUrl><MultiGeometry>
          <Point><coordinates>140,40,0</coordinates></Point>
          <LineString><coordinates>140,40 140.1,40.1</coordinates></LineString>
          <Polygon><outerBoundaryIs><LinearRing><coordinates>
            140,40 141,40 141,41 140,40
          </coordinates></LinearRing></outerBoundaryIs>
          <innerBoundaryIs><LinearRing><coordinates>
            140.1,40.1 140.2,40.1 140.2,40.2 140.1,40.1
          </coordinates></LinearRing></innerBoundaryIs></Polygon>
        </MultiGeometry></Placemark>)");
    const auto overlay = gui::parseKmlOverlay(xml, "synthetic.kml");
    REQUIRE(overlay.name == "synthetic.kml");
    REQUIRE(overlay.features.size() == 3);
    const auto& point = overlay.features[0];
    REQUIRE(point.geometry == gui::KmlGeometry::Point);
    REQUIRE(point.coordinates[0] == QPointF(140.0, 40.0));
    REQUIRE(point.style.lineColor == QColor(255, 0, 0, 255));
    REQUIRE(point.style.fillColor == QColor(0, 255, 0, 128));
    REQUIRE(point.style.pointColor == QColor(0, 0, 255, 255));
    REQUIRE(point.style.lineWidth == 3.0);
    REQUIRE(point.style.pointScale == 2.0);
    REQUIRE_FALSE(point.style.outline);
    REQUIRE_FALSE(point.style.showLabel);
    REQUIRE(overlay.features[1].geometry == gui::KmlGeometry::LineString);
    REQUIRE(overlay.features[2].holes.size() == 1);
    REQUIRE(overlay.features[2].holes[0].size() == 4);
}

TEST_CASE("KML supports prefixes and inline style overrides without fetching links", "[gui][map]") {
    const auto xml = QByteArray(R"(<k:kml xmlns:k="http://www.opengis.net/kml/2.2"><k:Document>
      <k:NetworkLink><k:Link><k:href>https://example.invalid/map.kml</k:href></k:Link></k:NetworkLink>
      <k:Placemark><k:styleUrl>https://example.invalid/style.kml#remote</k:styleUrl>
        <k:Style><k:LineStyle><k:width>5</k:width></k:LineStyle></k:Style>
        <k:LineString><k:coordinates>140,40 141,41</k:coordinates></k:LineString>
      </k:Placemark></k:Document></k:kml>)");
    const auto overlay = gui::parseKmlOverlay(xml, "prefixed");
    REQUIRE(overlay.features.size() == 1);
    REQUIRE(overlay.features[0].style.lineWidth == 5.0);
}

TEST_CASE("KMZ selects doc.kml and checks archive integrity", "[gui][map]") {
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const auto content = archive({{"other.kml", "invalid"}, {"doc.kml", document(Point)},
                                   {"files/icon.png", "unused icon"}});
    const auto path = write(directory, "overlay.KMZ", content);
    const auto overlay = gui::loadKmlOverlay(path);
    REQUIRE(overlay.features.size() == 1);
    REQUIRE(overlay.features[0].name == "Launcher");
    REQUIRE_THROWS_AS(gui::loadKmlOverlay(write(directory, "broken.kmz", "invalid ZIP")), std::runtime_error);
    REQUIRE_THROWS_AS(gui::loadKmlOverlay(write(directory, "empty.kmz", archive({{"readme.txt", "no KML"}}))), std::runtime_error);
    REQUIRE_THROWS_AS(gui::loadKmlOverlay(write(directory, "truncated.kmz", content.left(content.size() / 2))), std::runtime_error);
}

TEST_CASE("KML rejects invalid coordinates, unsafe XML and circular styles", "[gui][map]") {
    for (const auto& coordinates : {"181,40", "140,91", "nan,40", "140,40,nan", "140,40 141,41"}) {
        const auto xml = document(QByteArray("<Placemark><Point><coordinates>") + coordinates
                                  + "</coordinates></Point></Placemark>");
        REQUIRE_THROWS_AS(gui::parseKmlOverlay(xml, "invalid point"), std::runtime_error);
    }
    REQUIRE_THROWS_AS(gui::parseKmlOverlay("<!DOCTYPE kml [<!ENTITY x 'test'>]>" + document(Point), "DTD"), std::runtime_error);
    REQUIRE_THROWS_AS(gui::parseKmlOverlay(document(R"(
      <StyleMap id="a"><Pair><key>normal</key><styleUrl>#a</styleUrl></Pair></StyleMap>
      <Placemark><styleUrl>#a</styleUrl><Point><coordinates>140,40</coordinates></Point></Placemark>)"), "cycle"), std::runtime_error);
    REQUIRE_THROWS_AS(gui::parseKmlOverlay(document(R"(
      <Placemark><Polygon><outerBoundaryIs><LinearRing><coordinates>140,40 141,40 141,41 140,41</coordinates>
      </LinearRing></outerBoundaryIs></Polygon></Placemark>)"), "open ring"), std::runtime_error);
    REQUIRE_THROWS_AS(gui::parseKmlOverlay("<kml/>", "no geometry"), std::runtime_error);
    REQUIRE_THROWS_AS(gui::parseKmlOverlay(QByteArray(16 * 1024 * 1024 + 1, ' '), "large XML"), std::runtime_error);
    QByteArray nested;
    for (int depth = 0; depth < 65; ++depth) nested += "<Folder>";
    nested += Point;
    for (int depth = 0; depth < 65; ++depth) nested += "</Folder>";
    REQUIRE_THROWS_AS(gui::parseKmlOverlay(document(nested), "deep XML"), std::runtime_error);
    QByteArray manyCoordinates;
    manyCoordinates.reserve(800010);
    for (int index = 0; index <= 200000; ++index) manyCoordinates += "0,0 ";
    REQUIRE_THROWS_AS(gui::parseKmlOverlay(document("<Placemark><LineString><coordinates>" + manyCoordinates
        + "</coordinates></LineString></Placemark>"), "many coordinates"), std::runtime_error);
}
