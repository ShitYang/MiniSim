#include "TiffLoader.hpp"
#include "cmath"
#include "qimage.h"
#include <algorithm>
#include <memory>
#include <qdir.h>
#include <qopengltexture.h>
// #include "qopenglextrafunctions.h"

namespace wkf
{

bool operator==(const Tiff &src, const Tiff &other)
{
    return src.z == other.z && src.x == other.x && src.y == other.y;
}

TiffLoader::TiffLoader(std::string aTiffPath)
    : mTiffPath(QString::fromStdString(aTiffPath))
{

}

std::vector<Tiff> TiffLoader::GetCurrentViewportTiffs(float x, float y, float z)
{
    const auto &bound = GetViewportBounding(x, y, z);
    
    int level = std::min((int)std::ceil(z), cTILE_MAX_LEVEL);
    int numTiles = 1 << level;
    float sizePerTile = 1.0f / static_cast<float>(numTiles);

    int x1 = std::ceil(bound.p1.first / sizePerTile), x2 = std::ceil(bound.p2.first / sizePerTile);
    int y1 = std::ceil(bound.p1.second / sizePerTile), y2 = std::ceil(bound.p2.second / sizePerTile);

    x1 = std::max(0, x1 - 1), x2 = std::max(0, x2 - 1);
    y1 = std::max(0, y1 - 1), y2 = std::max(0, y2 - 1);

    std::vector<Tiff> res;
    res.reserve(30);
    for (int x = x1; x <= x2; ++x)
    {
        for (int y = y2; y <= y1; ++y)
        {
            res.push_back(Tiff{level, x, y});
        }
    }

    return res;
}


std::vector<Bound> TiffLoader::GetCurrentViewportGrids(float x, float y, float z)
{
    const auto &bound = GetViewportBounding(x, y, z);
    
    int level = std::min((int)std::ceil(z), cTILE_MAX_LEVEL);
    int numTiles = 1 << level;
    float sizePerTile = 1.0f / static_cast<float>(numTiles);

    int x1 = std::ceil(bound.p1.first / sizePerTile), x2 = std::ceil(bound.p2.first / sizePerTile);
    int y1 = std::ceil(bound.p1.second / sizePerTile), y2 = std::ceil(bound.p2.second / sizePerTile);

    x1 = std::max(0, x1 - 1), x2 = std::max(0, x2 - 1);
    y1 = std::max(0, y1 - 1), y2 = std::max(0, y2 - 1);

    std::vector<Bound> res;
    res.reserve(100);

    for (int x = x1; x <= x2; ++x)
    {
        for (int y = y2; y <= y1; ++y)
        {
            float tileX = x * sizePerTile, tileY = y * sizePerTile;
            Bound bound;
            bound.p1 = {tileX, tileY + sizePerTile};
            bound.p2 = {tileX + sizePerTile, tileY};
            res.push_back(bound);
        }
    }

    return res;
}

Bound TiffLoader::GetViewportBounding(float x, float y, float z, float aspect)
{
    // 放缩到 [0, 1]
    x = (x + 1) / 2;
    y = (y + 1) / 2;

    float ylen = 1.0f / std::pow(2.0f, z);
    float xlen = ylen * aspect;

    float x_lt = 0.0f, y_lt = 1.0f, x_rb = 1.0f, y_rb = 0.0f;
    x_lt = std::max(x_lt, x - xlen / 2), y_lt = std::min(y_lt, y + ylen / 2);
    x_rb = std::min(x_rb, x + xlen / 2), y_rb = std::max(y_rb, y - ylen / 2);

    Bound bound;
    bound.p1 = {x_lt, y_lt};
    bound.p2 = {x_rb, y_rb};
    
    return bound;

}


std::shared_ptr<QOpenGLTexture> TiffLoader::LoadTextureFromFile(const QString& path)
{
    QImage img(path);
    if (img.isNull()) return 0;

    std::shared_ptr<QOpenGLTexture> texture = std::make_unique<QOpenGLTexture>(img.convertToFormat(QImage::Format_RGBA8888).mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->generateMipMaps();
    texture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
    texture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
    return texture;
}

std::shared_ptr<QOpenGLTexture> TiffLoader::GetOrCreateTexture(int z, int x, int y)
{
    Tiff key{z, x, y};
    auto it = mCache.find(key);
    if (it != mCache.end()) {
        mLRU.remove(key);
        mLRU.push_back(key);
        return it.value();
    }

    QString fullPath = QString("%1/%2/%3/%4.png").arg(mTiffPath).arg(z).arg(x).arg(y);
    auto tex = LoadTextureFromFile(fullPath);
    if (!tex) return nullptr;

    if (mCache.size() >= cTIFF_MAX_CACHE) {
        EvictOne();
    }
    mCache[key] = tex;
    mLRU.push_back(key);
    return tex;
}

void TiffLoader::EvictOne()
{
    if (mLRU.empty()) return;
    Tiff old = mLRU.front();
    mLRU.pop_front();
    if (mCache.find(old) != mCache.end())
    {
        mCache.erase(mCache.find(old));
    }
    
}

void TiffLoader::Clear()
{
    mCache.clear();
    mLRU.clear();
}

int TiffLoader::GetEstimatedVRAM_MB() const
{
    int tileCount = static_cast<int>(mCache.size());
    int tileKB = tileCount * 340;
    return tileKB / 1024;
}


}