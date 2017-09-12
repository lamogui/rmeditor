
#include "renderfunctionscache.hpp"
#include <cassert>

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
    assert(false);
    return "";
  }
}