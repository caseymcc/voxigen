#ifndef _voxigen_gridFunctions_h_
#define _voxigen_gridFunctions_h_

#include "voxigen/defines.h"
#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>

namespace voxigen
{

namespace details
{



template<size_t dim>
void wrapDim(const glm::ivec3 &size, glm::ivec3 &index)
{
    if(index[dim] < 0)
        index[dim]=size[dim]+index[dim];
    else if(index[dim] >= size[dim])
        index[dim]=index[dim]-size[dim];
}

template<bool wrapX=true, bool wrapY=true, bool wrapZ=true>
void wrap(const glm::ivec3 &size, glm::ivec3 &index)
{
    if(wrapX)
        wrapDim<0>(size, index);
    if(wrapY)
        wrapDim<1>(size, index);
    if(wrapZ)
        wrapDim<2>(size, index);
}

template<typename _Chunk>
constexpr glm::ivec3 chunkSize() { return glm::ivec3(_Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value); }

template<typename _Region>
constexpr glm::ivec3 regionSize() { return glm::ivec3(_Region::sizeX::value, _Region::sizeY::value, _Region::sizeZ::value); }

template<typename _Grid>
constexpr glm::ivec3 regionCount() { return glm::ivec3(_Region::sizeX::value, _Region::sizeY::value, _Region::sizeZ::value); }

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

    if(chunkIndex.x < 0)
    {
        regionIndex.x--;
        chunkIndex.x+=regionSize<_Region>().x;
    }
    if(chunkIndex.y < 0)
    {
        regionIndex.y--;
        chunkIndex.y+=regionSize<_Region>().y;
    }
    if(chunkIndex.z < 0)
    {
        regionIndex.z--;
        chunkIndex.z+=regionSize<_Region>().z;
    }
}

template<size_t dim, bool wrap=false>
int differenceWrapDim(const glm::ivec3 &size, const glm::ivec3 &index1, const glm::ivec3 &index2)
{
    int diff=index2[dim]-index1[dim];

    if(wrap)
    {
        int wrapDiff;

        if(index1[dim] < index2[dim])
            wrapDiff=(index2[dim]-size[dim])-index1[dim];
        else
            wrapDiff=(size[dim]-index1[dim])+index2[dim];

        if(abs(diff)>abs(wrapDiff))
            return wrapDiff;
    }
    return diff;
}

template<bool wrapX=false, bool wrapY=false, bool wrapZ=false>
inline glm::ivec3 difference(const glm::ivec3 &size, const glm::ivec3 &index1, const glm::ivec3 &index2)
{
    glm::ivec3 diff;

    diff[0]=differenceWrapDim<0, wrapX>(size, index1, index2);
    diff[1]=differenceWrapDim<1, wrapY>(size, index1, index2);
    diff[2]=differenceWrapDim<2, wrapZ>(size, index1, index2);

    return diff;
}
//inline glm::ivec3 difference(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 &regionSize)
//{
//    glm::ivec3 offset(0.0f, 0.0f, 0.0f);
//
//    if(regionIndex1!=regionIndex2)
//        offset=(regionIndex2-regionIndex1)*regionSize;
//
//    return chunkIndex2+offset-chunkIndex1;
//}
//
//template<typename _Region>
//glm::ivec3 difference(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2)
//{
//    return difference(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2, regionSize<_Region>());
//}

template<size_t dim, bool wrap=false>
int differenceDim(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 &regionSize, const glm::ivec3 &regionCount)
{
    int diff=(regionIndex2[dim]-regionIndex1[dim])*regionSize[dim];

    diff+=-chunkIndex1[dim]+chunkIndex2[dim];
    if(wrap)
    {
        int wrapDiff;
        if(regionIndex1[dim] < regionIndex2[dim])
        {
            wrapDiff=((regionIndex2[dim]-regionCount[dim])-regionIndex1[dim])*regionSize[dim];
            wrapDiff+=-chunkIndex1[dim]+chunkIndex2[dim];
        }
        else
        {
            wrapDiff=((regionCount[dim]-regionIndex1[dim])+regionIndex2[dim])*regionSize[dim];
            wrapDiff+=chunkIndex1[dim]-chunkIndex2[dim];
        }

        if(abs(diff)>abs(wrapDiff))
            return wrapDiff;
    }

    return diff;
}

template<bool wrapX=false, bool wrapY=false, bool wrapZ=false>
glm::ivec3 difference(const glm::ivec3 &regionIndex1, const glm::ivec3 &index1, const glm::ivec3 &regionIndex2, const glm::ivec3 &index2, const glm::ivec3 &regionSize, const glm::ivec3 *regionCount=nullptr)
{
    glm::ivec3 diff;

    diff[0]=differenceDim<0, wrapX>(regionIndex1, index1, regionIndex2, index2, regionSize, *regionCount);
    diff[1]=differenceDim<1, wrapY>(regionIndex1, index1, regionIndex2, index2, regionSize, *regionCount);
    diff[2]=differenceDim<2, wrapZ>(regionIndex1, index1, regionIndex2, index2, regionSize, *regionCount);

    return diff; 
}

template<typename _Region, bool wrapX=false, bool wrapY=false, bool wrapZ=false>
glm::ivec3 difference(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 *regionCount=nullptr)
{
    return difference<wrapX, wrapY, wrapZ>(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2, regionSize<_Region>(), regionCount);
}

template<bool wrapX=false, bool wrapY=false, bool wrapZ=false>
float distance(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2, const glm::ivec3 &regionSize, const glm::ivec3 &chunkSize)
{
    glm::vec3 offset=glm::vec3(difference<wrapX, wrapY, wrapZ>(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2, regionSize)*chunkSize);

    return glm::length(offset);
}

template<typename _Region, typename _Chunk, bool wrapX=false, bool wrapY=false, bool wrapZ=false>
float distance(const glm::ivec3 &regionIndex1, const glm::ivec3 &chunkIndex1, const glm::ivec3 &regionIndex2, const glm::ivec3 &chunkIndex2)
{
    glm::vec3 offset=glm::vec3(difference<_Region, wrapX, wrapY, wrapZ>(regionIndex1, chunkIndex1, regionIndex2, chunkIndex2)*chunkSize<_Chunk>());

    return glm::length(offset);
}

inline bool validIndex(const glm::ivec3 &index, const glm::ivec3 size)
{
    if(index.x<0)
        return false;
    if(index.x>=size.x)
        return false;
    if(index.y<0)
        return false;
    if(index.y>=size.y)
        return false;
    if(index.z<0)
        return false;
    if(index.z>=size.z)
        return false;
    return true;
}

}//namespace details

}//namespace voxigen

#endif //_voxigen_gridFunctions_h_