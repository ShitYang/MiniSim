#ifndef WKF_TIFF_LOADER_HPP
#define WKF_TIFF_LOADER_HPP

#include <list>
#include <memory>
#include <qopengltexture.h>
#include <string>
#include <utility>
#include <vector>
#include "qhash.h"
#include "qstring.h"
#include "qopengl.h"

namespace wkf
{

struct Tiff
{
    int z;
    int x, y;

};

bool operator==(const Tiff &src, const Tiff &other);

inline uint qHash(const wkf::Tiff& key, uint seed = 0) noexcept
{
    seed = ::qHash(key.z, seed);
    seed = ::qHash(key.x, seed);
    seed = ::qHash(key.y, seed);
    return seed;
}

struct Bound
{ 
    using Point = std::pair<float, float>; 
    Point p1;
    Point p2;
};

class TiffLoader
{
public:

    TiffLoader(std::string aTiffPath);
     ~TiffLoader() { Clear(); };

    std::vector<Tiff> GetCurrentViewportTiffs(float x, float y, float z);
    std::vector<Bound> GetCurrentViewportGrids(float x, float y, float z);
    Bound GetViewportBounding(float x, float y, float z, float aspect = 1920.0 / 1080.0);


    std::shared_ptr<QOpenGLTexture> GetOrCreateTexture(int z, int x, int y);
    int GetCacheSize() const { return mCache.size(); }
    int GetEstimatedVRAM_MB() const;
    void Clear();

private:
    QString mTiffPath;
    const int cTIFF_MAX_CACHE = 100;
    const int cTILE_MAX_LEVEL = 6;
    
    std::shared_ptr<QOpenGLTexture> LoadTextureFromFile(const QString& path);
    void EvictOne();

    QHash<Tiff, std::shared_ptr<QOpenGLTexture>> mCache;
    std::list<Tiff> mLRU;
};

}

#endif