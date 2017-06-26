#ifndef _voxigen_chunk_h_
#define _voxigen_chunk_h_

#include "voxigen/boundingBox.h"
#include "voxigen/WorldDescriptors.h"

#include <vector>
#include <memory>
#include <type_traits>

namespace voxigen
{

template<typename _Block, size_t _x, size_t _y, size_t _z>
class Chunk//:public BoundingBox
{
public:
    Chunk(unsigned int, const glm::ivec3 &index, glm::vec3 worldOffset);

    typedef std::vector<_Block> Blocks;
    typedef std::integral_constant<size_t, _x> sizeX;
    typedef std::integral_constant<size_t, _y> sizeY;
    typedef std::integral_constant<size_t, _z> sizeZ;
    
    unsigned int getHash() const { return m_hash; }
    Blocks &getBlocks() { return m_blocks; }
    
    const glm::ivec3 &getIndex() const{ return m_index; }
    const glm::vec3 &getWorldOffset() const { return m_worldOffset; }

    _Block &getBlock(const glm::vec3 &position);

private:
    bool m_loaded;

    Blocks m_blocks;
    glm::ivec3 m_index;
    glm::vec3 m_worldOffset;
    unsigned int m_hash;
};

template<typename _Block, size_t _x, size_t _y, size_t _z>
using UniqueChunk=std::unique_ptr<Chunk<_Block, _x, _y, _z>>;


template<typename _Block, size_t _x, size_t _y, size_t _z>
Chunk<_Block, _x, _y, _z>::Chunk(unsigned int hash, const glm::ivec3 &index, glm::vec3 worldOffset):
//BoundingBox(dimensions, transform),
m_hash(hash),
m_index(index),
m_worldOffset(worldOffset)
{
    m_blocks.resize(_x*_y*_z);
}

template<typename _Block, size_t _x, size_t _y, size_t _z>
_Block &Chunk<_Block, _x, _y, _z>::getBlock(const glm::vec3 &position)
{
    glm::ivec3 &blockPos=glm::floor(position);
    unsigned int index=(_x*_y)*blockPos.y+_x*blockPos.y+blockPos.x;

    assert(index>=0);
    assert(index<m_blocks.size());
    return m_blocks[index];
}

} //namespace voxigen

#endif //_voxigen_chunk_h_