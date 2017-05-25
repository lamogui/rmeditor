#ifndef TEXTURE_H
#define TEXTURE_H

#include <QOpenGLFunctions>
#include <iostream>


class AbstractTexture : public QOpenGLFunctions
{
public:
  AbstractTexture();
  virtual ~AbstractTexture();

  virtual void destroyTexture();

  virtual void update(GLvoid* pixels) = 0;
  virtual void bind() = 0;

  inline GLuint getId() const { return id; }

protected:
  GLuint id;
};

class Texture2D : public AbstractTexture
{
public:
    Texture2D();

    bool create(GLsizei width, GLsizei height, const GLvoid *data = nullptr, GLint internalFormat = GL_RGBA, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);
    void update(GLvoid* pixels) override;
    bool load(const std::string &path); //Load color 2D texture
    void bind() override;

    inline GLsizei getWidth() const { return width; }
    inline GLsizei getHeight() const { return height; }

protected:
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
};

#endif // TEXTURE_H
