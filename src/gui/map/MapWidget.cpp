// ------------------------------------------------
// MapWidget.hpp の実装
// ------------------------------------------------

#include "MapWidget.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <utility>

#include <QBuffer>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPainterPath>
#include <QPdfWriter>
#include <QSaveFile>
#include <QStandardPaths>
#include <QSvgGenerator>
#include <QTemporaryDir>
#include <QTimer>
#include <QWheelEvent>

#include "WebMercator.hpp"

namespace {
    constexpr int MinimumZoom = 9;
    constexpr int MaximumZoom = 18;
    constexpr int MaximumPendingTiles = 32;
    constexpr qint64 MaximumTileBytes = 2 * 1024 * 1024;
    constexpr qint64 RetryDelayMs = 30000;
    constexpr int ExportDpi = 300;
    const QString TileListUrl = "https://maps.gsi.go.jp/development/ichiran.html";

    bool validCoordinate(const QPointF& point) {
        return std::isfinite(point.x()) && std::isfinite(point.y())
            && point.x() >= -180.0 && point.x() <= 180.0 && point.y() >= -90.0 && point.y() <= 90.0;
    }

    QPainterPath coordinatePath(const QVector<QPointF>& coordinates,
                               const std::function<QPointF(const QPointF&)>& project, bool closed) {
        QPainterPath path;
        if (coordinates.isEmpty()) return path;
        path.moveTo(project(coordinates.first()));
        for (qsizetype index = 1; index < coordinates.size(); ++index) path.lineTo(project(coordinates[index]));
        if (closed) path.closeSubpath();
        return path;
    }

    bool saveGeneratedFigure(const QString& generatedPath, const QString& destination, QString& error) {
        QFile generated(generatedPath);
        if (!generated.open(QIODevice::ReadOnly) || generated.size() == 0) {
            error = "Could not read generated map figure: " + generated.errorString();
            return false;
        }
        QSaveFile output(destination);
        if (!output.open(QIODevice::WriteOnly)) {
            error = "Could not save map figure: " + output.errorString();
            return false;
        }
        while (!generated.atEnd()) {
            const auto data = generated.read(64 * 1024);
            if (generated.error() != QFileDevice::NoError || output.write(data) != data.size()) {
                error = "Could not write map figure: "
                    + (generated.error() != QFileDevice::NoError ? generated.errorString() : output.errorString());
                return false;
            }
        }
        if (!output.commit()) {
            error = "Could not replace map figure: " + output.errorString();
            return false;
        }
        return true;
    }
}

gui::MapWidget::MapWidget(QWidget* parent) : QWidget(parent), m_tiles(32768) {
    setMinimumSize(300, 240);
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    m_center = webMercator::project({140.01, 40.24});
    m_network = new QNetworkAccessManager(this);
    m_network->setTransferTimeout(15000);
    m_network->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
    auto* cache = new QNetworkDiskCache(m_network);
    cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/gsi-pale");
    cache->setMaximumCacheSize(64 * 1024 * 1024);
    m_network->setCache(cache);
}

void gui::MapWidget::setCenter(double latitude, double longitude, int zoom) {
    if (!validCoordinate({longitude, latitude})) throw std::invalid_argument("Map center longitude/latitude is invalid.");
    m_center = webMercator::project({longitude, latitude});
    m_zoom = std::clamp(zoom, MinimumZoom, MaximumZoom);
    update();
}

void gui::MapWidget::setSeries(std::vector<MapSeries> series) {
    for (const auto& item : series) {
        for (const auto& point : item.coordinates) {
            if (!validCoordinate(point)) throw std::invalid_argument("Map series contains invalid longitude/latitude.");
        }
    }
    m_series = std::move(series);
    fitAll();
    update();
}

bool gui::MapWidget::loadOverlay(const QString& path, QString* error) {
    try {
        if (m_overlays.size() >= 32) throw std::runtime_error("At most 32 KML/KMZ overlays can be loaded.");
        auto overlay = loadKmlOverlay(path);
        m_overlays.push_back({std::move(overlay), true});
        fitAll();
        update();
        if (error) error->clear();
        reportStatus("Loaded map overlay: " + QFileInfo(path).fileName());
        return true;
    } catch (const std::exception& exception) {
        const auto message = QString::fromUtf8(exception.what());
        if (error) *error = message;
        reportStatus(message);
        return false;
    }
}

QStringList gui::MapWidget::overlayNames() const {
    QStringList names;
    for (const auto& overlay : m_overlays) names.push_back(overlay.data.name);
    return names;
}

size_t gui::MapWidget::overlayCount() const { return m_overlays.size(); }

void gui::MapWidget::setOverlayVisible(size_t index, bool visible) {
    if (index >= m_overlays.size()) throw std::out_of_range("Map overlay index is invalid.");
    m_overlays[index].visible = visible;
    update();
}

void gui::MapWidget::clearOverlays() {
    m_overlays.clear();
    update();
}

void gui::MapWidget::setStatusCallback(std::function<void(const QString&)> callback) {
    m_statusCallback = std::move(callback);
}

void gui::MapWidget::reportStatus(const QString& message) const {
    if (m_statusCallback) m_statusCallback(message);
}

void gui::MapWidget::fitAll() {
    bool hasPoints = false;
    double left = 0.0, right = 0.0, top = 0.0, bottom = 0.0;
    const auto include = [&](const QPointF& geographicPoint) {
        const auto point = webMercator::project(geographicPoint);
        if (!hasPoints) {
            left = right = point.x();
            top = bottom = point.y();
            hasPoints = true;
        } else {
            left = std::min(left, point.x());
            right = std::max(right, point.x());
            top = std::min(top, point.y());
            bottom = std::max(bottom, point.y());
        }
    };
    for (const auto& series : m_series) for (const auto& point : series.coordinates) include(point);
    for (const auto& overlay : m_overlays) {
        if (!overlay.visible) continue;
        for (const auto& feature : overlay.data.features) for (const auto& point : feature.coordinates) include(point);
    }
    if (!hasPoints) return;
    m_center = {(left + right) / 2.0, (top + bottom) / 2.0};
    m_zoom = MinimumZoom;
    for (int zoom = MaximumZoom; zoom >= MinimumZoom; --zoom) {
        const auto scale = webMercator::worldPixels(zoom);
        if ((right - left) * scale <= width() * 0.8 && (bottom - top) * scale <= height() * 0.8) {
            m_zoom = zoom;
            break;
        }
    }
    update();
}

void gui::MapWidget::normalizeCenter() {
    m_center.setX(m_center.x() - std::floor(m_center.x()));
    m_center.setY(std::clamp(m_center.y(), 0.0, 1.0));
}

QPointF gui::MapWidget::screenPosition(const QPointF& longitudeLatitude) const {
    auto offset = webMercator::project(longitudeLatitude) - m_center;
    if (offset.x() > 0.5) offset.rx() -= 1.0;
    if (offset.x() < -0.5) offset.rx() += 1.0;
    return QPointF(width() / 2.0, height() / 2.0) + offset * webMercator::worldPixels(m_zoom);
}

std::vector<gui::MapWidget::Tile> gui::MapWidget::visibleTiles() const {
    std::vector<Tile> tiles;
    const int tileCount = 1 << m_zoom;
    const auto origin = m_center * webMercator::worldPixels(m_zoom) - QPointF(width() / 2.0, height() / 2.0);
    const int firstX = static_cast<int>(std::floor(origin.x() / 256.0));
    const int firstY = static_cast<int>(std::floor(origin.y() / 256.0));
    const int lastX = static_cast<int>(std::floor((origin.x() + width() - 1.0) / 256.0));
    const int lastY = static_cast<int>(std::floor((origin.y() + height() - 1.0) / 256.0));
    for (int y = firstY; y <= lastY; ++y) {
        if (y < 0 || y >= tileCount) continue;
        for (int x = firstX; x <= lastX; ++x) {
            const int wrappedX = ((x % tileCount) + tileCount) % tileCount;
            tiles.push_back({wrappedX, y, QString("%1/%2/%3").arg(m_zoom).arg(wrappedX).arg(y),
                             QRectF(x * 256.0 - origin.x(), y * 256.0 - origin.y(), 256.0, 256.0)});
        }
    }
    return tiles;
}

bool gui::MapWidget::tilesReady() const {
    const auto tiles = visibleTiles();
    if (tiles.empty()) return false;
    return std::all_of(tiles.begin(), tiles.end(), [&](const Tile& tile) { return m_tiles.contains(tile.key); });
}

void gui::MapWidget::requestVisibleTiles() {
    const auto now = QDateTime::currentMSecsSinceEpoch();
    for (const auto& tile : visibleTiles()) {
        if (m_pendingTiles.size() >= MaximumPendingTiles) break;
        if (m_tiles.contains(tile.key) || m_pendingTiles.contains(tile.key)) continue;
        if (m_failedTiles.contains(tile.key) && now - m_failedTiles.value(tile.key) < RetryDelayMs) continue;
        requestTile(tile);
    }
}

void gui::MapWidget::requestTile(const Tile& tile) {
    m_pendingTiles.insert(tile.key);
    QNetworkRequest request(QUrl("https://cyberjapandata.gsi.go.jp/xyz/pale/" + tile.key + ".png"));
    request.setRawHeader("User-Agent", "Prologue-GUI-Demo");
    auto* reply = m_network->get(request);
    connect(reply, &QNetworkReply::readyRead, this, [reply] {
        if (reply->bytesAvailable() > MaximumTileBytes) reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, key = tile.key] {
        m_pendingTiles.remove(key);
        bool loaded = false;
        if (reply->error() == QNetworkReply::NoError
            && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
            auto data = reply->readAll();
            if (data.size() <= MaximumTileBytes) {
                QBuffer buffer(&data);
                buffer.open(QIODevice::ReadOnly);
                QImageReader reader(&buffer, "PNG");
                if (reader.size() == QSize(256, 256)) {
                    auto image = reader.read();
                    if (!image.isNull()) {
                        const auto cost = static_cast<int>((image.sizeInBytes() + 1023) / 1024);
                        m_tiles.insert(key, new QImage(std::move(image)), cost);
                        m_failedTiles.remove(key);
                        loaded = true;
                    }
                }
            }
        }
        if (!loaded) {
            if (m_failedTiles.size() >= 1024) m_failedTiles.clear();
            m_failedTiles.insert(key, QDateTime::currentMSecsSinceEpoch());
            reportStatus("GSI map tiles could not be loaded. Check the connection; map export waits for complete tiles.");
            QTimer::singleShot(static_cast<int>(RetryDelayMs), this, [this] { update(); });
        } else if (tilesReady()) {
            reportStatus("GSI map tiles are ready.");
        }
        reply->deleteLater();
        update();
    });
}

void gui::MapWidget::drawSeries(QPainter& painter) const {
    const auto project = [&](const QPointF& point) { return screenPosition(point); };
    for (const auto& series : m_series) {
        painter.setPen(QPen(series.color, 2.0));
        painter.setBrush(Qt::NoBrush);
        if (series.points) {
            for (const auto& point : series.coordinates) {
                const auto position = project(point);
                painter.drawLine(position + QPointF(-4, -4), position + QPointF(4, 4));
                painter.drawLine(position + QPointF(-4, 4), position + QPointF(4, -4));
            }
        } else {
            painter.drawPath(coordinatePath(series.coordinates, project, series.closed));
        }
    }
}

void gui::MapWidget::drawOverlays(QPainter& painter) const {
    const auto project = [&](const QPointF& point) { return screenPosition(point); };
    for (const auto& overlay : m_overlays) {
        if (!overlay.visible) continue;
        // Draw filled areas first, so their own boundary lines and markers stay visible.
        for (const auto geometry : {KmlGeometry::Polygon, KmlGeometry::LineString, KmlGeometry::Point}) {
            for (const auto& feature : overlay.data.features) {
                if (feature.geometry != geometry) continue;
                const auto& style = feature.style;
                painter.setPen(style.outline && style.lineWidth > 0.0 ? QPen(style.lineColor, style.lineWidth) : QPen(Qt::NoPen));
                painter.setBrush(Qt::NoBrush);
                if (feature.geometry == KmlGeometry::Point) {
                    const auto position = project(feature.coordinates.first());
                    painter.setPen(QPen(style.pointColor.darker(), 1.0));
                    painter.setBrush(style.pointColor);
                    const double radius = std::clamp(4.0 * style.pointScale, 0.0, 32.0);
                    painter.drawEllipse(position, radius, radius);
                    if (style.showLabel && !feature.name.isEmpty()) {
                        painter.setPen(Qt::black);
                        painter.drawText(position + QPointF(radius + 4, -radius - 2), feature.name);
                    }
                } else if (feature.geometry == KmlGeometry::LineString) {
                    painter.setPen(style.lineWidth > 0.0 ? QPen(style.lineColor, style.lineWidth) : QPen(Qt::NoPen));
                    painter.drawPath(coordinatePath(feature.coordinates, project, false));
                } else {
                    auto polygon = coordinatePath(feature.coordinates, project, true);
                    polygon.setFillRule(Qt::OddEvenFill);
                    for (const auto& hole : feature.holes) polygon.addPath(coordinatePath(hole, project, true));
                    painter.setBrush(style.fill ? QBrush(style.fillColor) : QBrush(Qt::NoBrush));
                    painter.drawPath(polygon);
                }
            }
        }
    }
}

void gui::MapWidget::drawLegend(QPainter& painter) const {
    std::vector<const MapSeries*> entries;
    QSet<QString> names;
    for (const auto& series : m_series) {
        if (series.coordinates.isEmpty() || series.name.isEmpty() || names.contains(series.name)) continue;
        names.insert(series.name);
        entries.push_back(&series);
    }
    if (entries.empty()) return;
    painter.save();
    auto font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    constexpr int RowHeight = 20;
    const int columnWidth = std::min(220, width() - 16);
    const int maximumRows = std::max(1, (height() - 96) / RowHeight);
    const int maximumColumns = std::max(1, (width() - 16) / columnWidth);
    const int count = static_cast<int>(entries.size());
    const int columns = std::min(maximumColumns, (count + maximumRows - 1) / maximumRows);
    const int rows = std::min(maximumRows, (count + columns - 1) / columns);
    const int capacity = rows * columns;
    painter.fillRect(QRect(8, 8, columnWidth * columns, rows * RowHeight + 8), QColor(255, 255, 255, 225));
    for (int index = 0; index < std::min(count, capacity); ++index) {
        const int x = 14 + (index / rows) * columnWidth;
        const int y = 12 + (index % rows) * RowHeight;
        if (count > capacity && index == capacity - 1) {
            painter.setPen(Qt::black);
            painter.drawText(QRect(x, y, columnWidth - 12, RowHeight), Qt::AlignVCenter,
                             QString("+ %1 more series").arg(count - capacity + 1));
            break;
        }
        const auto& series = *entries[static_cast<size_t>(index)];
        painter.setPen(QPen(series.color, 2.0));
        if (series.points) {
            painter.drawLine(x + 6, y + 6, x + 14, y + 14);
            painter.drawLine(x + 6, y + 14, x + 14, y + 6);
        } else {
            painter.drawLine(x, y + 10, x + 20, y + 10);
        }
        painter.setPen(Qt::black);
        const auto label = painter.fontMetrics().elidedText(series.name, Qt::ElideRight, columnWidth - 40);
        painter.drawText(QRect(x + 26, y, columnWidth - 40, RowHeight), Qt::AlignVCenter, label);
    }
    painter.restore();
}

void gui::MapWidget::drawScale(QPainter& painter) const {
    // Mercator scale changes with latitude. This bar describes the map center.
    constexpr double EarthCircumference = 40075016.68557849;
    const double latitude = webMercator::unproject(m_center).y();
    const double metersPerPixel = EarthCircumference * std::cos(latitude * std::numbers::pi / 180.0)
        / webMercator::worldPixels(m_zoom);
    const double maximumMeters = std::min(140.0, width() / 4.0) * metersPerPixel;
    const double power = std::pow(10.0, std::floor(std::log10(maximumMeters)));
    const double scaled = maximumMeters / power;
    const double distance = (scaled >= 5.0 ? 5.0 : scaled >= 2.0 ? 2.0 : 1.0) * power;
    const int barWidth = static_cast<int>(std::round(distance / metersPerPixel));
    const auto label = distance >= 1000.0 ? QString::number(distance / 1000.0, 'g', 3) + " km"
                                         : QString::number(distance, 'g', 3) + " m";
    painter.save();
    auto font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    const int labelWidth = painter.fontMetrics().horizontalAdvance(label);
    painter.fillRect(QRect(8, height() - 50, std::max(barWidth, labelWidth) + 16, 42), QColor(255, 255, 255, 225));
    painter.setPen(QPen(Qt::black, 2.0));
    const int y = height() - 18;
    painter.drawLine(16, y, 16 + barWidth, y);
    painter.drawLine(16, y - 5, 16, y + 1);
    painter.drawLine(16 + barWidth, y - 5, 16 + barWidth, y + 1);
    painter.drawText(QRect(16, height() - 46, std::max(barWidth, labelWidth), 20), Qt::AlignCenter, label);
    painter.restore();
}

void gui::MapWidget::drawAttribution(QPainter& painter, bool exporting) const {
    painter.save();
    QFont font = painter.font();
    font.setPixelSize(exporting ? 11 : 12);
    painter.setFont(font);
    const QString text = exporting
        ? QStringLiteral("地理院タイルを加工して作成 / GSI tiles (processed)\n") + TileListUrl
        : QStringLiteral("地理院タイル / GSI · click for source");
    const int alignment = Qt::AlignRight | Qt::AlignBottom | Qt::TextWordWrap;
    const auto textBounds = painter.fontMetrics().boundingRect(QRect(0, 0, width() - 16, 90), alignment, text);
    const QRect bounds(width() - textBounds.width() - 12, height() - textBounds.height() - 8,
                       textBounds.width() + 8, textBounds.height() + 4);
    painter.fillRect(bounds, QColor(255, 255, 255, 220));
    painter.setPen(QColor(30, 50, 70));
    painter.drawText(bounds.adjusted(4, 2, -4, -2), alignment, text);
    painter.restore();
}

void gui::MapWidget::drawMap(QPainter& painter, bool exporting) const {
    painter.save();
    painter.setFont(font());
    painter.setClipRect(rect());
    painter.fillRect(rect(), QColor(230, 234, 237));
    for (const auto& tile : visibleTiles()) {
        if (const auto* image = m_tiles.object(tile.key)) {
            painter.drawImage(tile.rectangle, *image);
        } else {
            painter.setPen(QColor(200, 206, 210));
            painter.drawRect(tile.rectangle);
        }
    }
    painter.setRenderHint(QPainter::Antialiasing);
    drawOverlays(painter);
    drawSeries(painter);
    drawLegend(painter);
    drawScale(painter);
    drawAttribution(painter, exporting);
    painter.restore();
}

void gui::MapWidget::paintEvent(QPaintEvent*) {
    requestVisibleTiles();
    QPainter painter(this);
    drawMap(painter, false);
}

void gui::MapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    if (event->position().x() > width() - 280 && event->position().y() > height() - 30) {
        QDesktopServices::openUrl(QUrl(TileListUrl));
        return;
    }
    m_dragging = true;
    m_dragPosition = event->position();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void gui::MapWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_dragging) return;
    m_center -= (event->position() - m_dragPosition) / webMercator::worldPixels(m_zoom);
    normalizeCenter();
    m_dragPosition = event->position();
    update();
    event->accept();
}

void gui::MapWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::OpenHandCursor);
        event->accept();
    }
}

void gui::MapWidget::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() == 0) return;
    const int nextZoom = std::clamp(m_zoom + (event->angleDelta().y() > 0 ? 1 : -1), MinimumZoom, MaximumZoom);
    const auto cursorOffset = event->position() - QPointF(width() / 2.0, height() / 2.0);
    const auto anchor = m_center + cursorOffset / webMercator::worldPixels(m_zoom);
    m_zoom = nextZoom;
    m_center = anchor - cursorOffset / webMercator::worldPixels(m_zoom);
    normalizeCenter();
    update();
    event->accept();
}

bool gui::MapWidget::exportFigure(const QString& path, QSize targetSize, QString* error) {
    const auto reject = [&](const QString& message) {
        if (error) *error = message;
        reportStatus(message);
        return false;
    };
    if (targetSize.width() <= 0 || targetSize.height() <= 0 || targetSize.width() > 6000 || targetSize.height() > 6000) {
        return reject("Map export dimensions must be between 1 and 6000 pixels.");
    }
    if (!tilesReady()) {
        requestVisibleTiles();
        return reject("Visible GSI tiles are incomplete. Wait for the map to load before exporting.");
    }
    const auto extension = QFileInfo(path).suffix().toLower();
    if (extension != "png" && extension != "svg" && extension != "pdf") {
        return reject("Map export supports PNG, SVG, and PDF.");
    }
    // Render to a fresh file first. A failed export must preserve an existing figure.
    QTemporaryDir temporary;
    if (!temporary.isValid()) return reject("Could not create temporary map export directory.");
    const auto generatedPath = temporary.filePath("map." + extension);
    const auto draw = [&](QPainter& painter, QSize deviceSize) {
        painter.fillRect(QRect(QPoint(0, 0), deviceSize), Qt::white);
        const double scale = std::min(static_cast<double>(deviceSize.width()) / width(),
                                      static_cast<double>(deviceSize.height()) / height());
        painter.translate((deviceSize.width() - width() * scale) / 2.0, (deviceSize.height() - height() * scale) / 2.0);
        painter.scale(scale, scale);
        drawMap(painter, true);
    };
    if (extension == "png") {
        QImage image(targetSize, QImage::Format_ARGB32_Premultiplied);
        if (image.isNull()) return reject("Could not allocate the map export image.");
        image.setDotsPerMeterX(static_cast<int>(ExportDpi / 0.0254));
        image.setDotsPerMeterY(static_cast<int>(ExportDpi / 0.0254));
        QPainter painter(&image);
        draw(painter, targetSize);
        painter.end();
        if (!image.save(generatedPath, "PNG")) return reject("Could not generate map PNG.");
    } else if (extension == "svg") {
        QSvgGenerator generator;
        generator.setFileName(generatedPath);
        generator.setSize(targetSize);
        generator.setViewBox(QRect(QPoint(0, 0), targetSize));
        generator.setResolution(ExportDpi);
        generator.setTitle("Prologue map / GSI tiles (processed)");
        generator.setDescription(TileListUrl);
        QPainter painter;
        if (!painter.begin(&generator)) return reject("Could not generate map SVG.");
        draw(painter, targetSize);
        if (!painter.end()) return reject("Could not finish map SVG.");
    } else if (extension == "pdf") {
        QPdfWriter writer(generatedPath);
        writer.setResolution(ExportDpi);
        writer.setPageSize(QPageSize(QSizeF(targetSize.width() * 25.4 / ExportDpi,
                                          targetSize.height() * 25.4 / ExportDpi), QPageSize::Millimeter));
        writer.setPageMargins(QMarginsF(0, 0, 0, 0));
        writer.setTitle("Prologue map / GSI tiles (processed)");
        QPainter painter;
        if (!painter.begin(&writer)) return reject("Could not generate map PDF.");
        draw(painter, QSize(writer.width(), writer.height()));
        if (!painter.end()) return reject("Could not finish map PDF.");
    }
    QString saveError;
    if (!saveGeneratedFigure(generatedPath, path, saveError)) return reject(saveError);
    if (error) error->clear();
    reportStatus("Saved map figure: " + QFileInfo(path).fileName());
    return true;
}
