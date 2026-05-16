#ifndef WKF_SPRITE_ATTACHMENT_HPP
#define WKF_SPRITE_ATTACHMENT_HPP

#include "qstring.h"
#include <qimage.h>
#include "QOpenGLTexture"

namespace wkf
{

class SpriteAttachment
{
public:

    SpriteAttachment() = default;
    SpriteAttachment(SpriteAttachment &&) = default;
    SpriteAttachment &operator=(SpriteAttachment &&) = default;

    QString mName{""};
    QImage mImage{""};
    std::unique_ptr<QOpenGLTexture> mTexturePtr{nullptr};
};

}

#endif