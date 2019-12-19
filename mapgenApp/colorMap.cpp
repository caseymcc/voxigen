
#include "colorMap.h"

namespace voxigen
{

ColorMap generateColorMap(const ColorScale &scale, size_t size)
{
    ColorMap colorMap(size);
    std::vector<glm::ivec4> &colors=colorMap.colors;

    colors.resize(size);

    if(scale.colorPositions.size()==0)
    {
        const glm::ivec4 color(0, 0, 0, 0);

        for(size_t i=0; i<size; ++i)
            colors[i]=color;
    }
    else if(scale.colorPositions.size()==1)
    {
        const glm::ivec4 &color=scale.colorPositions[0].color;

        for(size_t i=0; i<size; ++i)
            colors[i]=color;
    }
    else
    {
        const ColorPosition *prev=&scale.colorPositions[0];
        const ColorPosition *current=&scale.colorPositions[1];
        size_t nextIndex=2;

        for(size_t i=0; i<size; ++i)
        {
            float pos=(float)i/(size-1);

            while((pos > current->position) && (nextIndex < scale.colorPositions.size()))
            {
                prev=current;
                current=&scale.colorPositions[nextIndex];
                nextIndex++;
            }

            glm::ivec4 &color=colors[i];
            float range=current->position-prev->position;
            
            pos=(pos-prev->position)/range;
            color.r=(current->color.r-prev->color.r)*pos+prev->color.r;
            color.g=(current->color.g-prev->color.g)*pos+prev->color.g;
            color.b=(current->color.b-prev->color.b)*pos+prev->color.b;
            color.a=(current->color.a-prev->color.a)*pos+prev->color.a;
        }
    }
    return colorMap;
}

//ColorMap generateTemperatureColorMap(size_t size)
//{
//    ColorMap colorMap(size);
//    std::vector<glm::ivec4> &colors=colorMap.colors;
//
//    colors.resize(size);
//    
//    size_t divider=size/5;
//    size_t i=0;
//
//    for(; i<divider; ++i)
//    {
//        float value=(float)i/divider;
//
//        colors[i].r=255;
//        colors[i].g=255-(255*value);
//        colors[i].b=255;
//        colors[i].a=255;
//    }
//
//    for(; i<2*divider; ++i)
//    {
//        float value=(((float)i-(1*divider))/divider);
//
//        colors[i].r=255-(255*value);
//        colors[i].g=0;
//        colors[i].b=255;
//        colors[i].a=255;
//    }
//    for(; i<3*divider; ++i)
//    {
//        float value=(((float)i-(2*divider))/divider);
//
//        colors[i].r=0;
//        colors[i].g=(255*value);
//        colors[i].b=255-(255*value);
//        colors[i].a=255;
//    }
//    for(; i<4*divider; ++i)
//    {
//        float value=(((float)i-(3*divider))/divider);
//
//        colors[i].r=(255*value);
//        colors[i].g=255-(255*value);
//        colors[i].b=0;
//        colors[i].a=255;
//    }
//    for(; i<size; ++i)
//    {
//        float value=(((float)i-(4*divider))/(size-(4*divider)));
//
//        colors[i].r=255-(255*value);
//        colors[i].g=0;
//        colors[i].b=0;
//        colors[i].a=255;
//    }
//
//    return colorMap;
//}

ColorMap2D generateHieghtMoistureColorMap(size_t width, size_t height)
{
    ColorMap2D colorMap(width, height);
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