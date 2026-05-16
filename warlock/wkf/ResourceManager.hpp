#ifndef WKF_RESOURCE_MANAGER_HPP
#define WKF_RESOURCE_MANAGER_HPP

#include "SpriteAttachment.hpp"
#include "qmap.h"
#include <memory>
#include <string>
#include "TiffLoader.hpp"

namespace wkf
{

using SpriteMap = std::map<QString, std::unique_ptr<SpriteAttachment>>;

class ResourceManager
{
public:
    ResourceManager(std::string aTiffPath = "") : mTiffLoader(aTiffPath) {};
    TiffLoader &GetTileLoader() { return mTiffLoader; }

    SpriteMap mSpriteMap;

private:
    TiffLoader mTiffLoader;

};

}


#endif