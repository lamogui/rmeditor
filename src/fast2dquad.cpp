
#include "fast2dquad.hpp"


// FIXME : remove this ugly structure and define 
struct vec2
{
  float x, y;
};
#define BUFFER_OFFSET(a) ((char*)nullptr + (a))

Fast2DQuad::Fast2DQuad() :
  vbo(QOpenGLBuffer::VertexBuffer)
{
}

void Fast2DQuad::initializeGL(RenderFunctionsCache& gl)
{
  vec2 point[4];
  point[0].x = -1.0f; point[1].x = 1.0f; point[2].x = -1.0f; point[3].x = 1.0f;
  point[0].y = -1.0f; point[1].y = -1.0f; point[2].y = 1.0f; point[3].y = 1.0f;
  vec2 uv[4];
  uv[0].x = 0.0f; uv[1].x = 1.0f; uv[2].x = 0.0f; uv[3].x = 1.0f;
  uv[0].y = 0.0f; uv[1].y = 0.0f; uv[2].y = 1.0f; uv[3].y = 1.0f;

  vao.create();
  vao.bind();
    vbo.create();
    vbo.bind();
      vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
      vbo.allocate(4 * 2 * sizeof(float) * 2);

      gl.glVertexAttribPointer((GLuint)VertexAttributesIndex::position, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
      gl.glEnableVertexAttribArray((GLuint)VertexAttributesIndex::position);
      gl.glVertexAttribPointer((GLuint)VertexAttributesIndex::texCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(4 * 2 * sizeof(float)));
      gl.glEnableVertexAttribArray((GLuint)VertexAttributesIndex::texCoord);

      vbo.write(0, point, 4 * 2 * sizeof(float)); // write positions
      vbo.write(4 * 2 * sizeof(float), uv, 4 * 2 * sizeof(float)); // write uv

    vao.release();
  vbo.release();
}

Fast2DQuad::~Fast2DQuad()
{
  vbo.destroy();
  vao.destroy();
}

void Fast2DQuad::draw(RenderFunctionsCache& gl)
{
  vao.bind();
    gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  vao.release();
}
