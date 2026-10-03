// ------------------------------------------------
// 地理院タイルと飛行結果・保安域を表示する GUI 専用地図
// ------------------------------------------------

#pragma once

#include <functional>
#include <vector>

#include <QCache>
#include <QColor>
#include <QHash>
#include <QImage>
#include <QPointF>
#include <QSet>
#include <QStringList>
#include <QWidget>

#include "KmlOverlay.hpp"

class QNetworkAccessManager;
class QPainter;

namespace gui {
    struct MapSeries {
        QString name;
        QColor color = QColor(220, 50, 50);
        QVector<QPointF> coordinates; // longitude, latitude
        bool closed = false;
        bool points = false;
    };

    class MapWidget : public QWidget {
    public:
        explicit MapWidget(QWidget* parent = nullptr);

        void setCenter(double latitude, double longitude, int zoom = 14);
        void setSeries(std::vector<MapSeries> series);
        bool loadOverlay(const QString& path, QString* error = nullptr);
        QStringList overlayNames() const;
        size_t overlayCount() const;
        void setOverlayVisible(size_t index, bool visible);
        void clearOverlays();
        void fitAll();
        bool tilesReady() const;
        bool exportFigure(const QString& path, QSize targetSize, QString* error = nullptr);
        void setStatusCallback(std::function<void(const QString&)> callback);

    protected:
        void paintEvent(QPaintEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;

    private:
        struct Tile {
            int x;
            int y;
            QString key;
            QRectF rectangle;
        };

        struct Overlay {
            KmlOverlay data;
            bool visible = true;
        };

        std::vector<Tile> visibleTiles() const;
        void requestVisibleTiles();
        void requestTile(const Tile& tile);
        QPointF screenPosition(const QPointF& longitudeLatitude) const;
        void normalizeCenter();
        void drawMap(QPainter& painter, bool exporting) const;
        void drawSeries(QPainter& painter) const;
        void drawOverlays(QPainter& painter) const;
        void drawLegend(QPainter& painter) const;
        void drawScale(QPainter& painter) const;
        void drawAttribution(QPainter& painter, bool exporting) const;
        void reportStatus(const QString& message) const;

        QNetworkAccessManager* m_network = nullptr;
        QCache<QString, QImage> m_tiles;
        QSet<QString> m_pendingTiles;
        QHash<QString, qint64> m_failedTiles;
        QPointF m_center;
        int m_zoom = 14;
        QPointF m_dragPosition;
        bool m_dragging = false;
        std::vector<MapSeries> m_series;
        std::vector<Overlay> m_overlays;
        std::function<void(const QString&)> m_statusCallback;
    };
}
