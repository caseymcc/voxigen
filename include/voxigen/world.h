#ifndef _voxigen_world_h_
#define _voxigen_world_h_

#include "voxigen/biome.h"
#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"
#include "voxigen/worldGenerator.h"

#include <noise/noise.h>

#include <unordered_map>
#include <memory>
#include <limits>

namespace voxigen
{

template<typename _Block>
using UniqueChunkMap=std::unordered_map<unsigned int, UniqueChunk<_Block>>;

template<typename _Block>
class World
{
public:
    World(std::string name);
    ~World();

    void load();
    void save();

    Biome &getBiome(glm::ivec3 block);
    
    Chunk<_Block> &getChunk(const glm::ivec3 &index);
    Chunk<_Block> &getChunk(unsigned int chunkHash);

    glm::ivec3 getChunkIndex(const glm::vec3 &position);
    unsigned int chunkHash(const glm::ivec3 &chunkIndex) const;
    
    _Block &getBlock(const glm::vec3 &position);

    WorldDescriptors &getDescriptors() { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

private:
    typename UniqueChunkMap<_Block>::iterator generateChunk(glm::ivec3 chunkIndex);

    std::string m_name;

    WorldDescriptors m_descriptors;
    std::unique_ptr<WorldGenerator<_Block>> m_generator;

    glm::mat4 m_transform;

    std::vector<Biome> m_biomes;
    UniqueChunkMap<_Block> m_chunks;
};

template<typename _Block>
World<_Block>::World(std::string name):
m_name(name)
{
    m_transform=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

template<typename _Block>
World<_Block>::~World()
{}

template<typename _Block>
void World<_Block>::load()
{
    //load if exists, otherwise generate

    //generate
    m_descriptors.seed=0;
    m_descriptors.size=glm::ivec3(1024, 1024, 256);
    m_descriptors.chunkSize=glm::ivec3(16, 16, 16);

    m_descriptors.init();
    
    m_generator=std::make_unique<WorldGenerator<_Block>>(this);
}

template<typename _Block>
void World<_Block>::save()
{}

template<typename _Block>
Biome &World<_Block>::getBiome(glm::ivec3 block)
{}

template<typename _Block>
Chunk<_Block> &World<_Block>::getChunk(const glm::ivec3 &block)
{
    glm::ivec3 chunkIndex=block/m_descriptors.m_chunkSize;

    unsigned int chunkHash=chunkHash(chunkIndex);

    auto chunkIter=m_chunks.find(chunkHash);

    if(chunkIter==m_chunks.end())
        chunkIter=generateChunk(chunkIndex);

    return *(chunkIter->second.get());
}

template<typename _Block>
Chunk<_Block> &World<_Block>::getChunk(unsigned int chunkHash)
{
    auto chunkIter=m_chunks.find(chunkHash);

    if(chunkIter==m_chunks.end())
        chunkIter=generateChunk(m_descriptors.chunkIndex(chunkHash));

    return *(chunkIter->second.get());
}

template<typename _Block>
unsigned int World<_Block>::chunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Block>
glm::ivec3 World<_Block>::getChunkIndex(const glm::vec3 &position)
{
    glm::vec3 chunkSize(m_descriptors.chunkSize);

    return glm::floor(position/chunkSize);
    
}

template<typename _Block>
_Block &World<_Block>::getBlock(const glm::vec3 &position)
{
    glm::ivec3 chunkIndex=getChunkIndex(glm::vec3 position);

    auto chunkIter=m_chunks.find(hash);

    if(chunkIter==m_chunks.end())
    {
        assert(false);
        chunkIter=generateChunk(chunkIndex);
    }

    Chunk<_Block> *chunk=chunkIter.second.get();

    return chunkIter.second;
}

template<typename _Block>
typename UniqueChunkMap<_Block>::iterator World<_Block>::generateChunk(glm::ivec3 chunkIndex)
{
    UniqueChunk<_Block> chunk=m_generator->generateChunk(chunkIndex);

    return m_chunks.insert(m_chunks.end(), {chunk->getHash(), std::move(chunk)});
}

}//namespace voxigen

#endif //_voxigen_world_h_