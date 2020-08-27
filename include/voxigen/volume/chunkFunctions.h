#ifndef _voxigen_chunkFunctions_h_
#define _voxigen_chunkFunctions_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/volume/gridFunctions.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

#include <vector>
#include <functional>

namespace voxigen
{

inline glm::ivec3 difference(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 &regionSize)
{
    return details::difference(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2, regionSize);
}

inline float distance(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 &regionSize, const glm::ivec3 &chunkSize)
{
    return details::distance(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2, regionSize, chunkSize);
}

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

inline int ringSize(int radius)
{
    if(radius<=0)
        return 1;

    int r=2*radius+1;

    return (6*r*r)-(12*r)+8;
}

template<typename _Chunk>
void ringCube(std::vector<glm::ivec3> &positions, int radius)
{
    glm::ivec3 position(0, 0, 0);
    int z=0;
    int index=0;

    positions.resize(ringSize(radius));
    position.z=-radius;
    for(int y=-radius; y<=radius; ++y)
    {
        position.y=y;
        for(int x=-radius; x<=radius; ++x)
        {
            position.x=x;
            positions[index++]=position;
        }
    }

    for(int z=-radius+1; z<radius; ++z)
    {
        position.z=z;

        position.y=-radius;
        for(int x=-radius; x<radius; ++x)
        {
            position.x=x;
            positions[index++]=position;
        }

        position.x=radius;
        for(int y=-radius; y<radius; ++y)
        {
            position.y=y;
            positions[index++]=position;
        }

        position.y=radius;
        for(int x=radius; x>-radius; --x)
        {
            position.x=x;
            positions[index++]=position;
        }

        position.x=-radius;
        for(int y=radius; y>-radius; --y)
        {
            position.y=y;
            positions[index++]=position;
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
                positions[index++]=position;
            }
        }
    }
}

template<typename _ChunkType>
std::vector<std::vector<glm::ivec3>> buildRadiusRingMap(float radius)
{
    std::vector<std::vector<glm::ivec3>> radiusMap;

    int maxRing=std::ceil(radius/std::max(std::max(_ChunkType::sizeX::value, _ChunkType::sizeY::value), _ChunkType::sizeZ::value));

    radiusMap.resize(maxRing);
    for(size_t i=0; i<maxRing; ++i)
        ringCube<_ChunkType>(radiusMap[i], i);

    return radiusMap;
}


template<typename _Grid, typename _Node>
std::vector<std::vector<_Node *>> buildRingSearchMap(_Grid *grid, const glm::ivec3 &region, const glm::ivec3 &chunk, std::vector<std::vector<glm::ivec3>> &radiusRingMap, std::function<_Node *(Key)> createNode)
{
    std::vector<std::vector<_Node *>> searchMap;
    int chunkIndicesSize=radiusRingMap.size();

    glm::ivec3 index;
    glm::ivec3 currentRegionIndex;

    searchMap.resize(radiusRingMap.size());
    for(size_t i=0; i<radiusRingMap.size(); ++i)
    {
        std::vector<glm::ivec3> &chunkIndices=radiusRingMap[i];
        searchMap[i].resize(chunkIndices.size());

        for(size_t j=0; j<chunkIndices.size(); ++j)
        {
            index=chunk+chunkIndices[j];
            currentRegionIndex=region;

            glm::vec3 regionOffset=grid->getDescriptors().adjustRegion(currentRegionIndex, index);
            Key key=grid->getHashes(currentRegionIndex, index);

            searchMap[i][j]=createNode(key);
        }
    }

    return searchMap;
}

//template<typename _Grid, typename _Node>
//void moveSearchMap_posX(std::vector<std::vector<_Node *>> &searchMap, glm::ivec3 direction, std::vector<_Node *> &removedNodes, std::function<_Node *(Key)> createNode)
//{
//    for(size_t i=radiusRingMap.size()-1; i>=0; --i)
//    {
//        int radius=i;
//
//        //move bottom
//        index.z=searchMap[0].index.z;
//        for(int y=-radius; y<radius; ++y)
//        {
//            for(int x=-radius; x<radius-1; ++x)
//            {
//                searchMap[index]=searchMap[index+1];
//                index++;
//            }
//            searchMap[index]=createNode(key);
//            index++;
//        }
//
//        //move top
//        index.z=searchMap[0].index.z+radius;
//        for(int y=-radius; y<radius; ++y)
//        {
//            for(int x=-radius; x<radius-1; ++x)
//            {
//                searchMap[index]=searchMap[index+1];
//                index++;
//            }
//            searchMap[index]=createNode(key);
//            index++;
//        }
//    }
//}
//
//template<typename _Grid, typename _Node>
//void moveSearchMap(std::vector<std::vector<_Node *>> &searchMap, glm::ivec3 direction, std::vector<_Node *> &removedNodes, std::function<_Node *(Key)> createNode)
//{
//    glm::ivec3 index;
//
//    if(direction.x>0)
//        moveSearchMap_posX(searchMap, direction, removedNodes, createNode);
//}

VOXIGEN_EXPORT void spiralIndex(glm::ivec3 &volumeSize, std::vector<size_t> &indexes);
VOXIGEN_EXPORT void spiralIndexXYPlane(int z, int pos, glm::ivec3 &volumeSize, glm::ivec3 &center, std::vector<size_t> &indexes);

}//namespace voxigen

#endif //_voxigen_chunkFunctions_h_