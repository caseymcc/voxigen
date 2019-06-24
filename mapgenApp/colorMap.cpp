
#include "colorMap.h"

namespace voxigen
{

ColorMap generateColorMap(size_t width, size_t height)
{
    ColorMap colorMap(width, height);
    std::vector<glm::ivec4> &colors=colorMap.colors;

    colors.resize(width*height*4);

    size_t index=0;
    size_t waterWidth=width/2-3;
    size_t halfWidth=width/2;
    float elevation=0.0f;
    float waterDelta=1.0f/waterWidth;
    float elevationDelta=1.0f/halfWidth;

    float moisture=0.0f;
    float moistureDelta=1.0f/height;

    glm::ivec4 white(255, 255, 255, 255);

    //handle underwater
    for(size_t y=0; y<height; y++)
    {
        elevation=0.0f;
        size_t x=0;

        for(; x<waterWidth; x++)
        {
            glm::ivec4 &color=colors[index];

            color.r=48*elevation;
            color.g=64*elevation;
            color.b=127*elevation;
            color.a=255;

            elevation+=waterDelta;
            index++;
        }

        size_t i=0;
        for(; x<halfWidth; x++)
        {
            glm::ivec4 &color=colors[index];

            color.r=48;
            color.g=80+i*20;
            color.b=140+i*10;
            color.a=255;

            index++;
        }

        elevation=0.0f;

        for(; x<width; x++)
        {
            glm::ivec4 &color=colors[index];
            glm::ivec4 landColor;

            int r=210-100*moisture;
            int g=185-45*moisture;
            int b=139-45*moisture;
            
            color.r=255*elevation+r*(1.0f-elevation);
            color.g=255*elevation+g*(1.0f-elevation);
            color.b=255*elevation+b*(1.0f-elevation);
            color.a=255;
            
            elevation+=elevationDelta;
            index++;
        }

        moisture+=moistureDelta;
    }

    return colorMap;
}

}//namespace voxigen