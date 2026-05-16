#pragma once

#include <QColor>
#include <QString>
#include <QVector2D>
#include <vector>

namespace wkf {

enum class VectorFeatureType {
    Point,
    LineString,
    Polygon
};

struct VectorFeature {
    VectorFeatureType type;
    std::vector<QVector2D> coordinates; // lon,lat pairs
    std::vector<std::vector<QVector2D>> rings; // polygon rings (first = outer)
    QColor strokeColor;
    QColor fillColor;
    float width = 2.0f;  // point size or line width
    double minZoom = 0.0;
    double maxZoom = 10.0;
    QString name;
};

struct VectorFeatureSet {
    std::vector<VectorFeature> features;

    bool LoadFromJson(const QString& path);
    std::vector<const VectorFeature*> GetVisibleFeatures(double zoomLevel) const;
};

} // namespace wkf
