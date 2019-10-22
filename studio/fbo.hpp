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

       size_t  getSizeX() const {return m_width;}
       size_t  getSizeY() const {return m_height;}
       GLuint getColor(void) const;
       GLenum getFormat() const {return m_format;}

       void bind();

       QImage getImage();

       inline size_t width() const { return m_width; }
       inline size_t height() const { return m_height; }

private:
       void updateTexture();

       GLuint m_FrameBuffer;
       GLuint m_DepthRenderBuffer;
       GLuint m_ColorTextureID;
       GLuint m_DepthTextureID;
       size_t    m_width,m_height;
       GLint  m_format;
};


#endif
