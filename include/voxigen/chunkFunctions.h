#ifndef _voxigen_chunkFunctions_h_
#define _voxigen_chunkFunctions_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

template<typename _Chunk>
void spiralCube(std::vector<glm::ivec3> &positions, float radius)
{
    glm::ivec3 chunkSize=glm::ivec3(_Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value);
    glm::ivec3 chunkRadius=glm::ceil(glm::vec3(radius/chunkSize.x, radius/chunkSize.y, radius/chunkSize.z));

//    int maxSteps=std::max(std::max(chunkRadius.x, chunkRadius.y), chunkRadius.z);
    int maxSteps=glm::compMax(chunkRadius);
    int index;
    glm::ivec3 position(0, 0, 0);
    int z=0;

    for(int j=0; j<maxSteps; j++)
    {
        z=0;
        while(z>=-j && z<=j)
        {
            position.z=z;

            if((z==j)||(z==-j))
            {
                position.x=0;
                position.y=0;
                float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                if(chunkDistance<=radius)
                    positions.push_back(position);

                index=1;
            }
            else
                index=j;

            while(index<=j)
            {
                //right side
                position.x=index;
                for(int i=-index; i<index; ++i)
                {
                    position.y=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=radius)
                        positions.push_back(position);
                }

                //bottom side
                position.y=index;
                for(int i=index; i>-index; --i)
                {
                    position.x=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=radius)
                        positions.push_back(position);
                }

                //left side
                position.x=-index;
                for(int i=index; i>-index; --i)
                {
                    position.y=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=radius)
                        positions.push_back(position);
                }

                //top side
                position.y=-index;
                for(int i=-index; i<=index; ++i)
                {
                    position.x=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=radius)
                        positions.push_back(position);
                }
                index++;
            }

            if(z>0)
                z=-z;
            else
                z=-z+1;
        }
    }
}

template<typename _Chunk>
void ringCube(std::vector<glm::ivec3> &positions, int radius)
{
    glm::ivec3 position(0, 0, 0);
    int z=0;

    position.z=-radius;
    for(int y=-radius; y<=radius; ++y)
    {
        position.y=y;
        for(int x=-radius; x<=radius; ++x)
        {
            position.x=x;
            positions.push_back(position);
        }
    }

    for(int z=-radius+1; z<radius; ++z)
    {
        position.z=z;

        position.y=-radius;
        for(int x=-radius; x<radius; ++x)
        {
            position.x=x;
            positions.push_back(position);
        }

        position.x=radius;
        for(int y=-radius; y<radius; ++y)
        {
            position.y=y;
            positions.push_back(position);
        }

        position.y=radius;
        for(int x=radius; x>-radius; --x)
        {
            position.x=x;
            positions.push_back(position);
        }

        position.x=-radius;
        for(int y=radius; y>-radius; --y)
        {
            position.y=y;
            positions.push_back(position);
        }
        
    }

    if(radius>0)
    {
        position.z=radius;
        for(int y=-radius; y<=radius; ++y)
        {
            position.y=y;
            for(int x=-radius; x<=radius; ++x)
            {
                position.x=x;
                positions.push_back(position);
            }
        }
    }
}

}//namespace voxigen

#endif //_voxigen_chunkFunctions_h_