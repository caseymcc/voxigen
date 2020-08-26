#ifndef _voxigen_indexing_h_
#define _voxigen_indexing_h_

#include <glm/glm.hpp>

namespace voxigen
{

template<typename _Chunk>
glm::ivec3 chunkIndexToCoords(size_t index)
{
    const size_t pitchY=_Chunk::sizeX::value;
    const size_t pitchZ=_Chunk::sizeX::value*_Chunk::sizeX::value;

    glm::ivec3 coords;

    coords.z=index/pitchZ;
    index-=coords.z*pitchZ;
    coords.y=index/pitchY;
    index-=coords.y*pitchY;
    coords.x=index;

    return coords;
}

template<typename _Chunk>
size_t chunkCoordsToIndex(const glm::ivec3 &coords)
{
    const size_t pitchY=_Chunk::sizeX::value;
    const size_t pitchZ=_Chunk::sizeX::value*_Chunk::sizeX::value;

    size_t index=(coords.z*pitchZ)+(coords.y*pitchY)+coords.x;

    return index;
}

}//namespace voxigen

#endif//_voxigen_indexing_h_