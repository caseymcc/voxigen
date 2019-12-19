#ifndef _voxigen_colorMap_h_
#define _voxigen_colorMap_h_

#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>

namespace voxigen
{

struct ColorMap
{
    ColorMap():width(0), height(0) {}
    ColorMap(size_t width, size_t height):width(width), height(height) {}

    glm::ivec4 &color(size_t x, size_t y) { return colors[width*y+x]; }

    size_t width;
    size_t height;
    std::vector<glm::ivec4> colors;
};

ColorMap generateColorMap(size_t width, size_t height);

}//namespace voxigen

#endif //_voxigen_colorMap_h_