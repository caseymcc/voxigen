#ifndef _voxigen_fill_h_
#define _voxigen_fill_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/wrap.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace voxigen
{

inline const std::array<glm::ivec2, 3> &getFloodPoints(const glm::vec2 &direction)
{
    static std::vector<std::array<glm::ivec2, 3>> points=
    {
        {{
            {-1,  1},
            { 0,  1},
            { 1,  1}
        }},
        {{
            { 0,  1},
            { 1,  1},
            { 1,  0}
        }},
        {{
            { 1,  1},
            { 1,  0},
            { 1, -1}
        }},
        {{
            { 1,  0},
            { 1, -1},
            { 0, -1}
        }},
        {{
            { 1, -1},
            { 0, -1},
            {-1, -1}
        }},
        {{
            { 0, -1},
            {-1, -1},
            {-1,  0}
        }},
        {{
            {-1,  1},
            {-1,  0},
            {-1, -1}
        }},
        {{
            {-1,  0},
            {-1,  1},
            { 0,  1}
        }}
    };

    if(direction.x==0.0f)
    {
        if(direction.y<0.0f)
            return points[4];
        return points[0];
    }

    size_t index=0;
    float  slope=direction.y/abs(direction.x);

    if(slope<3.0f)
        index++;
    if(slope<0.5f/1.5f)
        index++;
    if(slope<-0.5f/1.5f)
        index++;

    if(direction.x<0.0f)
        index=index+4;
    else if(slope<-3.0f)
        index++;

    return points[index];
}

inline void fillPoints(const glm::ivec2 &point, const glm::vec2 &direction, std::vector<float> &map, const glm::ivec2 &size, float value)
{
    //keep in mind that in world coords +y is up, in image coords +y is down
    glm::ivec2 step(0, 0);
    glm::ivec2 step2(0, 0);
    float scale;

    if(direction.x==0.0f)
    {
        if(direction.y>0.0f)
            step.y=-1;
        else
            step.y=1;
        scale=0.0f;
    }
    else if(direction.y==0.0f)
    {
        if(direction.x>0.0f)
            step.x=1;
        else
            step.x=-1;
        scale=0.0f;
    }
    else
    {
        scale=abs(direction.y/direction.x);

        if(scale>1.0f)
        {
            if(direction.y>0.0f)
                step.y=-1;
            else
                step.y=1;

            if(direction.x>0.0f)
                step2.x=1;
            else
                step2.x=-1;

            scale=1.0f/scale;
        }
        else
        {
            if(direction.x>0.0f)
                step.x=1;
            else
                step.x=-1;

            if(direction.y>0.0f)
                step2.y=-1;
            else
                step2.y=1;
        }
    }

    glm::ivec2 pt=point;
    glm::ivec2 lower(0, 0);
    glm::ivec2 upper(size.x-1, size.y-1);

    {
        pt=pt+step;
        glm::ivec2 pt2=wrap(pt, lower, upper);
        size_t index=(pt2.y*size.x)+pt2.x;
        map[index]=std::min(map[index]+(value*(1.0f-scale)), 1.0f);
    }

    if(scale>0.0f)
    {
        pt=pt+step2;
        glm::ivec2 pt2=wrap(pt, lower, upper);
        size_t index=(pt2.y*size.x)+pt2.x;
        map[index]=std::min(map[index]+(value*scale), 1.0f);
    }
}

}//namespace voxigen

#endif //_voxigen_fill_h_