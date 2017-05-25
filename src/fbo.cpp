#include "fbo.hpp"

#include <iostream>

#include <QColor>


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
FBO::FBO(size_t width, size_t height):
  QOpenGLFunctions(QOpenGLContext::currentContext())
{
    FrameBuffer=0;
    ColorTextureID=0;
    width = width;
    height = height;
    format = GL_RGBA;

    //Create texture.
    glGenTextures(1, &ColorTextureID);
    glBindTexture(GL_TEXTURE_2D, ColorTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);


    glGenTextures(1, &DepthTextureID);
    glBindTexture(GL_TEXTURE_2D, DepthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei)width, (GLsizei)height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE,   GL_LUMINANCE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);


    //Make buffer
    glGenFramebuffers(1, &FrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, FrameBuffer);



    glGenRenderbuffers(1, &DepthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER_EXT, DepthRenderBuffer);

    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, (GLsizei)width, (GLsizei)height);
    glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ColorTextureID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT, DepthRenderBuffer);


    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
FBO::~FBO()
{
    glDeleteTextures(1, &ColorTextureID);
    glDeleteTextures(1, &DepthTextureID);
    glDeleteFramebuffers(1, &FrameBuffer);
    glDeleteRenderbuffers(1, &DepthRenderBuffer);
}


//-----------------------------------------------------------------------------
// Enable
//-----------------------------------------------------------------------------
void FBO::enable(void)
{
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0,0,(GLsizei)width, (GLsizei)height);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, FrameBuffer);
    glPushMatrix();
}

//-----------------------------------------------------------------------------
// Disable
//-----------------------------------------------------------------------------
void FBO::disable(void)
{
    glPopMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    glPopAttrib();
}



//-----------------------------------------------------------------------------
// Set
//-----------------------------------------------------------------------------
void FBO::setSize(size_t width, size_t height )
{
    width = width;
    height = height;

    updateTexture();
}
void FBO::setFormat(GLint format)
{
    format = format;
    updateTexture();
}


//-----------------------------------------------------------------------------
// Get
//-----------------------------------------------------------------------------
GLuint FBO::getColor(void) const
{
    return ColorTextureID;
}

//-----------------------------------------------------------------------------
// updateTexture
//-----------------------------------------------------------------------------
void FBO::updateTexture()
{
    glBindTexture(GL_TEXTURE_2D, ColorTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, DepthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (GLsizei)width, (GLsizei)height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER_EXT, FrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER_EXT, DepthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, (GLsizei)width, (GLsizei)height);
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

//-----------------------------------------------------------------------------
// bind
//-----------------------------------------------------------------------------
void FBO::bind()
{
    glBindTexture(GL_TEXTURE_2D, ColorTextureID);
}

QImage FBO::getImage()
{
  QImage img((int)width,(int)height,QImage::Format_RGBA8888);
  img.fill(QColor(0,0,0,0));
  this->glReadPixels(0,0, (GLsizei)width, (GLsizei)height,GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid*)img.bits());
  return img.transformed(QTransform(1.0,0.0,0.0,-1.0,0.0,0.0));
}
