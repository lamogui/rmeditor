#ifndef RMEDITOR_RENDERFUNCTIONSCACHE_HPP
#define RMEDITOR_RENDERFUNCTIONSCACHE_HPP

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>

typedef QOpenGLFunctions_4_5_Core RenderFunctionsCache;
typedef QOpenGLShaderProgram ShaderProgram;

namespace VertexAttributesIndex
{
  enum Enum
  {
    position = 1,
    texCoord = 2,
    normal = 3
  };

  std::string&& toString(Enum e);

};

#endif //! RMEDITOR_RENDERFUNCTIONSCACHE_HPP