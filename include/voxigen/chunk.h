#ifndef _voxigen_chunk_h_
#define _voxigen_chunk_h_

#include "voxigen/boundingBox.h"
#include "voxigen/WorldDescriptors.h"

#include <vector>
#include <memory>

namespace voxigen
{

template<typename _Block>
class Chunk//:public BoundingBox
{
public:
    Chunk(WorldDescriptors *descriptors, const glm::ivec3 &index, glm::vec3 worldOffset);

    typedef std::vector<_Block> Blocks;
    
    unsigned int getHash() const { return m_hash; }
    Blocks &getBlocks() { return m_blocks; }
    
    const glm::ivec3 &getIndex() const{ return m_index; }
    const glm::vec3 &getWorldOffset() const { return m_worldOffset; }

    _Block &getBlock(const glm::vec3 &position);

private:
    WorldDescriptors *m_descriptors;
    bool m_loaded;

    Blocks m_blocks;
    glm::ivec3 m_index;
    glm::vec3 m_worldOffset;
    unsigned int m_hash;
};

template<typename _Block>
using UniqueChunk=std::unique_ptr<Chunk<_Block>>;


template<typename _Block>
Chunk<_Block>::Chunk(WorldDescriptors *descriptors, const glm::ivec3 &index, glm::vec3 worldOffset):
//BoundingBox(dimensions, transform),
m_descriptors(descriptors),
m_index(index),
m_worldOffset(worldOffset)
{
    m_hash=descriptors->chunkHash(m_index);
    glm::ivec3 &chunkSize=m_descriptors->chunkSize;

    m_blocks.resize(chunkSize.x*chunkSize.y*chunkSize.z);
}

template<typename _Block>
_Block &Chunk<_Block>::getBlock(const glm::vec3 &position)
{
    glm::ivec3 &chunkSize=m_descriptors->chunkSize;
    glm::ivec3 &blockPos=glm::floor(position);
    unsigned int index=(chunkSize.x*chunkSize.y)*blockPos.y+chunkSize.x*blockPos.y+blockPos.x;

    assert(index>=0);
    assert(index<m_blocks.size());
    return m_blocks[index];
}

} //namespace voxigen

#endif //_voxigen_chunk_h_