#ifndef RMEDITOR_FAST2DQUAD_HPP
#define RMEDITOR_FAST2DQUAD_HPP

#include "renderfunctionscache.hpp"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class Fast2DQuad 
{
public:
  Fast2DQuad();
  virtual ~Fast2DQuad();

  void initializeGL(RenderFunctionsCache& gl);
  void draw(RenderFunctionsCache& gl);

private:
  QOpenGLBuffer vbo;
  QOpenGLVertexArrayObject vao;
};

#endif // !RMEDITOR_FAST2DQUAD_HPP
