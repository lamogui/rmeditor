#ifndef RMEDITOR_RENDERFUNCTIONSCACHE_HPP
#define RMEDITOR_RENDERFUNCTIONSCACHE_HPP

#include <QOpenGLFunctions_4_4_Core>
#include <QOpenGLShaderProgram>

typedef QOpenGLShaderProgram ShaderProgram;

namespace VertexAttributesIndex
{
  enum Enum
  {
    position = 1,
    texCoord = 2,
    normal = 3
  };

  std::string toString(Enum e);

};

extern QOpenGLFunctions_4_4_Core g_renderFunctions;

#endif //! RMEDITOR_RENDERFUNCTIONSCACHE_HPP
