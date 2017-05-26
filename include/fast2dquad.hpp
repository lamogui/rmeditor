#ifndef RMEDITOR_FAST2DQUAD_HPP
#define RMEDITOR_FAST2DQUAD_HPP

#include "renderfunctionscache.hpp"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class Fast2DQuad : public RenderFunctionsCache // FIXME : do not inherit from this !!! 
{
public:
  Fast2DQuad();
  ~Fast2DQuad() override;

  void draw();

private:
  QOpenGLBuffer vbo;
  QOpenGLVertexArrayObject vao;
};

#endif // !RMEDITOR_FAST2DQUAD_HPP
