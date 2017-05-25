#ifndef DEF_FAST2DQUAD
#define DEF_FAST2DQUAD

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#define VERTEX_ATTRIBUTES_POSITION_INDEX 1
#define VERTEX_ATTRIBUTES_NORMAL_INDEX   2
#define VERTEX_ATTRIBUTES_TEXCOORD_INDEX 3


class Fast2DQuad : public QOpenGLFunctions_4_5_Core
{
public:
  Fast2DQuad();
  ~Fast2DQuad() override;

  void draw();

private:
  QOpenGLBuffer vbo;
  QOpenGLVertexArrayObject vao;
};

#endif // !DEF_FAST2DQUAD
