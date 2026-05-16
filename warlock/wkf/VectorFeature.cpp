#include "VectorFeature.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QtEndian>

namespace wkf {

static QColor ParseColor(const QString& hex) {
    if (hex.isEmpty())
        return QColor();
    QString s = hex;
    if (s.startsWith('#'))
        s = s.mid(1);
    if (s.length() == 6)
        s = "FF" + s; // default alpha = 255
    if (s.length() == 8) {
        bool ok;
        uint val = s.toUInt(&ok, 16);
        if (ok) {
            // Qt: AARRGGBB — A is high byte, R is next, then G, then B
            return QColor::fromRgba(qFromBigEndian(val));
        }
    }
    qWarning() << "Invalid color format:" << hex;
    return QColor(255, 255, 255, 255);
}

static QVector2D ParseCoord(const QJsonValue& v) {
    QJsonArray arr = v.toArray();
    if (arr.size() < 2)
        return QVector2D();
    return QVector2D(arr[0].toDouble(), arr[1].toDouble());
}

bool VectorFeatureSet::LoadFromJson(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open vector data file:" << path;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in" << path << ":" << err.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray featureArray = root["features"].toArray();

    for (const QJsonValue& featVal : featureArray) {
        QJsonObject featObj = featVal.toObject();
        QJsonObject props = featObj["properties"].toObject();

        VectorFeature feat;
        feat.name = props["name"].toString();

        // LOD range
        feat.minZoom = props.contains("minZoom") ? props["minZoom"].toDouble() : 0.0;
        feat.maxZoom = props.contains("maxZoom") ? props["maxZoom"].toDouble() : 10.0;

        QString typeStr = featObj["type"].toString().toLower();

        if (typeStr == "point") {
            feat.type = VectorFeatureType::Point;
            feat.strokeColor = ParseColor(props["color"].toString());
            if (!feat.strokeColor.isValid())
                feat.strokeColor = QColor(255, 0, 0, 255);
            feat.width = props.contains("size") ? static_cast<float>(props["size"].toDouble()) : 6.0f;
            feat.coordinates.push_back(ParseCoord(featObj["coordinates"]));
        } else if (typeStr == "linestring") {
            feat.type = VectorFeatureType::LineString;
            feat.strokeColor = ParseColor(props["color"].toString());
            if (!feat.strokeColor.isValid())
                feat.strokeColor = QColor(0, 0, 255, 255);
            feat.width = props.contains("width") ? static_cast<float>(props["width"].toDouble()) : 2.0f;
            QJsonArray coords = featObj["coordinates"].toArray();
            for (const QJsonValue& c : coords)
                feat.coordinates.push_back(ParseCoord(c));
        } else if (typeStr == "polygon") {
            feat.type = VectorFeatureType::Polygon;
            feat.strokeColor = ParseColor(props["borderColor"].toString());
            if (!feat.strokeColor.isValid())
                feat.strokeColor = QColor(0, 255, 0, 255);
            feat.fillColor = ParseColor(props["fillColor"].toString());
            if (!feat.fillColor.isValid())
                feat.fillColor = QColor(0, 255, 0, 64);
            feat.width = props.contains("borderWidth") ? static_cast<float>(props["borderWidth"].toDouble()) : 1.5f;
            // coordinates is an array of rings: [[[lon,lat],...], [[lon,lat],...], ...]
            QJsonArray ringsArray = featObj["coordinates"].toArray();
            for (const QJsonValue& ringVal : ringsArray) {
                QJsonArray ringArr = ringVal.toArray();
                std::vector<QVector2D> ring;
                for (const QJsonValue& c : ringArr)
                    ring.push_back(ParseCoord(c));
                feat.rings.push_back(std::move(ring));
            }
        } else {
            qWarning() << "Unknown feature type:" << typeStr;
            continue;
        }

        features.push_back(std::move(feat));
    }

    qDebug() << "Loaded" << features.size() << "vector features from" << path;
    return true;
}

std::vector<const VectorFeature*> VectorFeatureSet::GetVisibleFeatures(double zoomLevel) const {
    std::vector<const VectorFeature*> result;
    for (const auto& f : features) {
        if (zoomLevel >= f.minZoom && zoomLevel <= f.maxZoom)
            result.push_back(&f);
    }
    return result;
}

} // namespace wkf
