#ifndef DEF_CORE_FBO_H
#define DEF_CORE_FBO_H

#include <QImage>
#include <QOpenGLFunctions>


#include "texture.hpp"

//-----------------------------------------------------------------------------
// Class Frame Buffer Object
//-----------------------------------------------------------------------------
class FBO : public QOpenGLFunctions
{
public:
       FBO(size_t width=1280, size_t height=720);
       ~FBO();

       void enable(void);
       void disable(void);


       void setSize(size_t width, size_t height );
       void setFormat(GLint format);

       size_t  getSizeX() const {return width;}
       size_t  getSizeY() const {return height;}
       GLuint getColor(void) const;
       GLenum getFormat() const {return format;}

       void bind();

       QImage getImage();

       inline size_t getWidth() const { return width; }
       inline size_t getHeight() const { return height; }

private:
       void updateTexture();

       GLuint FrameBuffer;
       GLuint DepthRenderBuffer;
       GLuint ColorTextureID;
       GLuint DepthTextureID;
       size_t  width,height;
       GLint  format;
};


#endif // !DEF_CORE_FBO_H
