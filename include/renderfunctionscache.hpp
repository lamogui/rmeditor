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
    position = 0,
    texCoord = 1,
    normal = 2
  };

  std::string&& toString(Enum e)
  {
    switch(e)
    {
    case position:
      return "position";
    case texCoord:
      return "texCoord";
    case normal:
      return "normal";
    default:
      Q_ASSERT(false);
      return "";
    }
  }
};

#endif //! RMEDITOR_RENDERFUNCTIONSCACHE_HPP