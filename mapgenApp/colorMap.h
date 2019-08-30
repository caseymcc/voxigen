#ifndef _voxigen_colorMap_h_
#define _voxigen_colorMap_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/sortedVector.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>

namespace voxigen
{

struct ColorPosition
{
    ColorPosition(glm::ivec4 color, float position):color(color), position(position) {}

    friend bool operator<(const ColorPosition &pos0, const ColorPosition &pos1)
    {   return pos0.position<pos1.position;}

    glm::ivec4 color;
    float position;
};

struct ColorScale
{
    void addColorPosition(const glm::ivec4 &color, float position)
    {
        ColorPosition colorPosition(color, position);

        insert_sorted(colorPositions, colorPosition);
    }

    std::vector<ColorPosition> colorPositions;
};

struct ColorMap
{
    ColorMap():size(0){}
    ColorMap(size_t size):size(size){}

    glm::ivec4 &color(size_t i) { return colors[i]; }

    size_t size;
    std::vector<glm::ivec4> colors;
};

struct ColorMap2D
{
    ColorMap2D():width(0), height(0) {}
    ColorMap2D(size_t width, size_t height):width(width), height(height) {}

    glm::ivec4 &color(size_t x, size_t y) { return colors[width*y+x]; }

    size_t width;
    size_t height;
    std::vector<glm::ivec4> colors;
};

ColorMap generateColorMap(const ColorScale &scale, size_t size);
//ColorMap generateTemperatureColorMap(size_t size);
ColorMap2D generateHieghtMoistureColorMap(size_t width, size_t height);


}//namespace voxigen

#endif //_voxigen_colorMap_h_