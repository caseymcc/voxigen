#ifndef _voxigen_world_h_
#define _voxigen_world_h_

#include "voxigen/biome.h"
#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"
#include "voxigen/worldGenerator.h"

#include <noise/noise.h>

#include <unordered_map>

namespace voxigen
{

template<typename _Block>
using ChunkMap=std::unordered_map<unsigned int, UniqueChunk<_Block>>;

template<typename _Block>
class World
{
public:
    World(std::string name);
    ~World();

    void load();
    void save();

    Biome &getBiome(glm::ivec3 block);
    
    Chunk<_Block> &getChunk(glm::ivec3 index);
    glm::ivec3 getChunkIndex(glm::vec3 position);
    int chunkHash(glm::ivec3 index) const;
    
    _Block &getBlock(glm::vec3 position);

private:
    std::string m_name;

    WorldDescriptors m_descriptors;
    WorldGenerator m_generator;

    std::vector<Biome> m_biomes;
    ChunkMap<_Block> m_chunks;

    glm::ivec3 m_chunkCount;
    glm::ivec3 m_chunkStride;
};

template<typename _Block>
World<_Block>::World(std::string name):
m_name(name)
{}

template<typename _Block>
World<_Block>::~World()
{}

template<typename _Block>
void World<_Block>::load()
{
    //load if exists, otherwise generate

    //generate
    m_descriptors.m_seed=0;
    m_descriptors.m_size=glm::ivec3(1024, 1024, 256);
    m_descriptors.m_chunkSize=glm::ivec3(64, 64, 16);

    m_chunkCount=m_descriptors.m_size/m_descriptors.m_chunkSize;
    m_chunkStride=glm::ivec3(m_descriptors.m_chunkCount.y*m_descriptors.m_chunkCount.z, m_descriptors.m_chunkCount.z, 1);
}

template<typename _Block>
void World<_Block>::save()
{}

template<typename _Block>
Biome &World<_Block>::getBiome(glm::ivec3 block)
{}

template<typename _Block>
Chunk<_Block> &World<_Block>::getChunk(glm::ivec3 block)
{
    glm::ivec3 chunkIndex=block/m_descriptors.m_chunkSize;

    int hash=chunkHash(chunkIndex);

    auto chunkIter=m_chunks.find(hash);

    if(chunkIter!=m_chunks.end())
        return m_chunksIter.second;

    m_generator.generateChunk(chunkIndex);
}

template<typename _Block>
int World<_Block>::chunkHash(glm::ivec3 index) const
{
    return (m_chunkStride.x*index.x)+(m_chunkStride.y*index.y)+index.z;
}

template<typename _Block>
glm::ivec3 World<_Block>::getChunkIndex(glm::vec3 position)
{

}

template<typename _Block>
_Block &World<_Block>::getBlock(glm::vec3 position)
{}

}//namespace voxigen

#endif //_voxigen_world_h_