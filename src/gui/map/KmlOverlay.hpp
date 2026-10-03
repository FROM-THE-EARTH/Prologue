// ------------------------------------------------
// ローカル KML/KMZ の保安域・点・線を表示用データに変換
// ------------------------------------------------

#pragma once

#include <vector>

#include <QByteArray>
#include <QColor>
#include <QPointF>
#include <QString>
#include <QVector>

namespace gui {
    enum class KmlGeometry { Point, LineString, Polygon };

    struct KmlStyle {
        QColor lineColor = QColor(40, 80, 200);
        QColor fillColor = QColor(80, 110, 220, 80);
        QColor pointColor = QColor(230, 90, 50);
        double lineWidth = 2.0;
        double pointScale = 1.0;
        bool fill = true;
        bool outline = true;
        bool showLabel = true;
    };

    struct KmlFeature {
        QString name;
        KmlGeometry geometry = KmlGeometry::Point;
        QVector<QPointF> coordinates; // longitude, latitude
        QVector<QVector<QPointF>> holes;
        KmlStyle style;
    };

    struct KmlOverlay {
        QString name;
        std::vector<KmlFeature> features;
    };

    // Throws std::runtime_error with a useful filename/field description on failure.
    KmlOverlay loadKmlOverlay(const QString& path);
    KmlOverlay parseKmlOverlay(const QByteArray& xml, const QString& name);
}
