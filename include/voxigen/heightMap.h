#ifndef _voxigen_heightMap_h_
#define _voxigen_heightMap_h_

#include <glm/glm.hpp>
#include <array>
#include <vector>

namespace voxigen
{

struct HeightMapCell
{
    unsigned int type;
    float height;
};

class HeightMap
{
public:
    HeightMap() {};

private:
  
};

} //namespace voxigen

#endif //_voxigen_heightMap_h_