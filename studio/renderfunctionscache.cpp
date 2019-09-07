
#include "renderfunctionscache.hpp"

std::string VertexAttributesIndex::toString(Enum e)
{
  switch (e)
  {
  case position:
    return "position";
  case texCoord:
    return "texCoord";
  case normal:
    return "normal";
  default:
    jassertfalse;
    return "";
  }
}
