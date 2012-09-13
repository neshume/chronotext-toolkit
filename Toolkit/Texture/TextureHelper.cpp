#include "TextureHelper.h"
#include "PVRHelper.h"

#include "cinder/app/App.h"
#include "cinder/ImageIo.h"

using namespace std;
using namespace ci;
using namespace ci::app;

gl::Texture* TextureHelper::loadTexture(const string &resourceName, bool useMipmap, int filter, GLenum wrapS, GLenum wrapT)
{
    return loadTexture(InputSource::getResource(resourceName), useMipmap, filter, wrapS, wrapT);
}

gl::Texture* TextureHelper::loadTexture(InputSourceRef inputSource, bool useMipmap, int filter, GLenum wrapS, GLenum wrapT)
{
    if (inputSource->getFilePathHint().rfind(".pvr.gz") != string::npos)
    {
        if (inputSource->isFile())
        {
            Buffer buffer = PVRHelper::decompressPVRGZ(inputSource->getFilePath());
            return PVRHelper::getPVRTexture(buffer, useMipmap, wrapS, wrapT);
        }
        else
        {
            throw runtime_error("PVR.GZ TEXTURES CAN ONLY BE LOADED FROM FILES");
        }
    }
    else if (inputSource->getFilePathHint().rfind(".pvr.ccz") != string::npos)
    {
        Buffer buffer = PVRHelper::decompressPVRCCZ(inputSource->loadDataSource());
        return PVRHelper::getPVRTexture(buffer, useMipmap, wrapS, wrapT);
    }
    else if (inputSource->getFilePathHint().rfind(".pvr") != string::npos)
    {
        Buffer buffer = inputSource->loadDataSource()->getBuffer();
        return PVRHelper::getPVRTexture(buffer, useMipmap, wrapS, wrapT);
    }
    
    // ---
    
    gl::Texture::Format format;
    format.setWrap(wrapS, wrapT);
    
    if (useMipmap)
    {
        format.enableMipmapping(true);
        format.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
    }

    switch (filter)
    {
        case FILTER_TRANSLUCENT:
        {
            Surface surface(loadImage(inputSource->loadDataSource()));
            Channel8u channel = surface.getChannel(0);
            
            GLenum dataFormat = GL_ALPHA;
            format.setInternalFormat(GL_ALPHA);
            
            // if the data is not already contiguous, we'll need to create a block of memory that is
            if ( ( channel.getIncrement() != 1 ) || ( channel.getRowBytes() != channel.getWidth() * sizeof(uint8_t) ) )
            {
                boost::shared_ptr<uint8_t> data( new uint8_t[channel.getWidth() * channel.getHeight()], checked_array_deleter<uint8_t>() );
                uint8_t *dest = data.get();
                const int8_t inc = channel.getIncrement();
                const int32_t width = channel.getWidth();
                for ( int y = 0; y < channel.getHeight(); ++y )
                {
                    const uint8_t *src = channel.getData( 0, y );
                    for ( int x = 0; x < width; ++x )
                    {
                        *dest++ = *src;
                        src += inc;
                    }
                }
                
                return new gl::Texture(data.get(), dataFormat, channel.getWidth(), channel.getHeight(), format);
            }
            else
            {
                return new gl::Texture(channel.getData(), dataFormat, channel.getWidth(), channel.getHeight(), format);
            }
        }
    }
    
    return new gl::Texture(loadImage(inputSource->loadDataSource()), format);
}

void TextureHelper::bindTexture(gl::Texture *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->getId());
}

void TextureHelper::beginTexture(gl::Texture *texture)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, texture->getId());
}

void TextureHelper::endTexture()
{
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void TextureHelper::drawTextureFromCenter(gl::Texture *texture)
{
    drawTexture(texture, texture->getWidth() * 0.5, texture->getHeight() * 0.5);
}

void TextureHelper::drawTexture(gl::Texture *texture, float rx, float ry)
{
    float x1 = -rx;
    float y1 = -ry;
    
    float x2 = x1 + texture->getWidth();
    float y2 = y1 + texture->getHeight();
    
    const GLfloat vertices[] =
    {
        x1, y1,
        x2, y1,
        x2, y2,
        x1, y2
    };
    
    const GLfloat coords[] =
    {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };
    
    glTexCoordPointer(2, GL_FLOAT, 0, coords);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
