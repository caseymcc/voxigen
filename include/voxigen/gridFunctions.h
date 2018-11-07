#ifndef _voxigen_gridFunctions_h_
#define _voxigen_gridFunctions_h_

#include "voxigen/defines.h"
#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>

namespace voxigen
{

namespace details
{

template<typename _Chunk>
constexpr glm::ivec3 chunkSize() { return glm::ivec3(_Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value); }

template<typename _Region>
constexpr glm::ivec3 regionSize() { return glm::ivec3(_Region::sizeX::value, _Region::sizeY::value, _Region::sizeZ::value); }

template<typename _Region, typename _Chunk>
constexpr glm::ivec3 regionCellSize() { return regionSize<_Region>()*chunkSize<_Chunk>(); }

template<typename _Region>
void offsetIndexes(const glm::ivec3 &startRegionIndex, const glm::ivec3 &startChunkIndex, glm::ivec3 delta, glm::ivec3 &regionIndex, glm::ivec3 &chunkIndex)
{
    regionIndex=startRegionIndex;
    chunkIndex=startChunkIndex+delta;

    glm::ivec3 regionDelta=chunkIndex/regionSize<_Region>();

    regionIndex+=regionDelta;
    chunkIndex-=regionDelta*regionSize<_Region>();

    if(chunkIndex.x<0)
    {
        regionIndex.x--;
        chunkIndex.x+=regionSize<_Region>().x;
    }
    if(chunkIndex.y<0)
    {
        regionIndex.y--;
        chunkIndex.y+=regionSize<_Region>().y;
    }
    if(chunkIndex.z<0)
    {
        regionIndex.z--;
        chunkIndex.z+=regionSize<_Region>().z;
    }
}

inline glm::ivec3 difference(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 &regionSize)
{
    glm::ivec3 offset(0.0f, 0.0f, 0.0f);

    if(regionIndex1!=regionIndex2)
        offset=(regionIndex2-regionIndex1)*regionSize;

    return chunkIndex2+offset-chunkIndex1;
}

template<typename _Region>
inline glm::ivec3 difference(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2)
{
    return difference(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2, regionSize<_Region>());
}

inline float distance(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 &regionSize, const glm::ivec3 &chunkSize)
{
    glm::vec3 offset=glm::vec3(difference(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2, regionSize)*chunkSize);

    return glm::length(offset);
}

template<typename _Region, typename _Chunk>
float distance(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2)
{
    glm::vec3 offset=glm::vec3(difference<_Region>(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2)*chunkSize<_Chunk>());

    return glm::length(offset);
}

}//namespace details

}//namespace voxigen

#endif //_voxigen_gridFunctions_h_