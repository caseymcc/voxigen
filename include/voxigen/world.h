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

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
using UniqueChunkMap=std::unordered_map<unsigned int, UniqueChunk<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>>;

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
class World
{
public:
    World(std::string name);
    ~World();

    typedef Chunk<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> ChunkType;
    typedef UniqueChunkMap<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> UniqueChunkMap;

    void load();
    void save();

    Biome &getBiome(glm::ivec3 block);
    
    ChunkType &getChunk(const glm::ivec3 &index);
    ChunkType &getChunk(unsigned int chunkHash);

    glm::ivec3 getChunkIndex(const glm::vec3 &position);
    unsigned int chunkHash(const glm::ivec3 &chunkIndex) const;
    
    _Block &getBlock(const glm::vec3 &position);

    WorldDescriptors &getDescriptors() { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

private:
    typename UniqueChunkMap::iterator generateChunk(glm::ivec3 chunkIndex);

    std::string m_name;

    WorldDescriptors m_descriptors;
    std::unique_ptr<WorldGenerator<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>> m_generator;

    glm::mat4 m_transform;

    std::vector<Biome> m_biomes;
    UniqueChunkMap m_chunks;
};

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::World(std::string name):
m_name(name)
{
    m_transform=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::~World()
{}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
void World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::load()
{
    //load if exists, otherwise generate

    //generate
    m_descriptors.seed=0;
    m_descriptors.size=glm::ivec3(1024, 1024, 256);
    m_descriptors.chunkSize=glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ);

    m_descriptors.init();
    
    m_generator=std::make_unique<WorldGenerator<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>>(this);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
void World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::save()
{}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
Biome &World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getBiome(glm::ivec3 block)
{}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
typename World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::ChunkType &World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getChunk(const glm::ivec3 &block)
{
    glm::ivec3 chunkIndex=block/m_descriptors.m_chunkSize;

    unsigned int chunkHash=chunkHash(chunkIndex);

    auto chunkIter=m_chunks.find(chunkHash);

    if(chunkIter==m_chunks.end())
        chunkIter=generateChunk(chunkIndex);

    return *(chunkIter->second.get());
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
typename World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::ChunkType &World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getChunk(unsigned int chunkHash)
{
    auto chunkIter=m_chunks.find(chunkHash);

    if(chunkIter==m_chunks.end())
        chunkIter=generateChunk(m_descriptors.chunkIndex(chunkHash));

    return *(chunkIter->second.get());
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
unsigned int World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::chunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
glm::ivec3 World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getChunkIndex(const glm::vec3 &position)
{
    glm::vec3 chunkSize(m_descriptors.chunkSize);

    return glm::floor(position/chunkSize);
    
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
_Block &World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getBlock(const glm::vec3 &position)
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

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
typename World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::UniqueChunkMap::iterator World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::generateChunk(glm::ivec3 chunkIndex)
{
    UniqueChunk<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> chunk=m_generator->generateChunk(chunkIndex);

    return m_chunks.insert(m_chunks.end(), {chunk->getHash(), std::move(chunk)});
}

}//namespace voxigen

#endif //_voxigen_world_h_