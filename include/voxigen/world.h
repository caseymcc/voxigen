#ifndef _voxigen_world_h_
#define _voxigen_world_h_

#include "voxigen/biome.h"
#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"
#include "voxigen/worldGenerator.h"
#include "voxigen/chunkHandler.h"
#include "voxigen/entity.h"

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
    World();
    ~World();

    typedef Chunk<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef UniqueChunkMap<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> UniqueChunkMap;
    typedef std::shared_ptr<ChunkType> SharedChunk;
    typedef std::unordered_map<unsigned int, SharedChunk> SharedChunkMap;

    void create(std::string directory, std::string name);
    void load(std::string directory);
    void save();

    Biome &getBiome(glm::ivec3 block);
    
    SharedChunkHandle getChunk(const glm::ivec3 &index);
    SharedChunkHandle getChunk(unsigned int chunkHash);
    std::vector<unsigned int> getUpdatedChunks();

    glm::ivec3 getChunkIndex(const glm::vec3 &position);
    unsigned int chunkHash(const glm::ivec3 &chunkIndex) const;
    unsigned int getChunkHash(const glm::vec3 &position);
    
    WorldDescriptors &getDescriptors() { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

private:
    std::string m_directory;
    std::string m_name;

    WorldDescriptors m_descriptors;
    ChunkHandler<ChunkType> m_chunkHandler;

    glm::mat4 m_transform;

//    Entity m_player;
    std::vector<Biome> m_biomes;
    SharedChunkMap m_chunks;
};

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::World():
m_chunkHandler(&m_descriptors)
{
    m_transform=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::~World()
{
    m_chunkHandler.terminate();

    //If handles are still in use, then the handler will be destroyed before they are returned
    // and program will crash. Destroy items that use handles before the world is destroyed,
    // anything holding onto the handle from getChunk
    assert(m_chunkHandler.handlesInUse()==0);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
void World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::create(std::string directory, std::string name)
{
    m_name=name;
    m_directory=directory;

    m_descriptors.create(name, 0, glm::ivec3(1024, 1024, 256), glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ));
    m_descriptors.init();

    std::string configFile=directory+"/worldConfig.json";
    m_descriptors.save(configFile);
    
    std::string chunksDirectory=directory+"/chunks";
    fs::path chunksPath(chunksDirectory);

    fs::create_directory(chunksPath);
    m_chunkHandler.load(chunksDirectory);
    m_chunkHandler.initialize();
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
void World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::load(std::string directory)
{
    m_directory=directory;

    std::string configFile=directory+"/worldConfig.json";
    m_descriptors.load(configFile);

    std::string chunkDirectory=directory+"/chunks";

    m_chunkHandler.load(chunkDirectory);
    m_chunkHandler.initialize();
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
void World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::save()
{
    std::string configFile=directory+"/worldConfig.json";
    m_descriptors.save(configFile)
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
Biome &World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getBiome(glm::ivec3 block)
{}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
typename World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::SharedChunkHandle World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getChunk(const glm::ivec3 &block)
{
    glm::ivec3 chunkIndex=block/m_descriptors.m_chunkSize;

    unsigned int chunkHash=chunkHash(chunkIndex);

    return m_chunkHandler.getChunk(chunkHash);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
typename World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::SharedChunkHandle World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getChunk(unsigned int chunkHash)
{
    return m_chunkHandler.getChunk(chunkHash);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
std::vector<unsigned int> World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getUpdatedChunks()
{
    return m_chunkHandler.getUpdatedChunks();
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
unsigned int World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::chunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
glm::ivec3 World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getChunkIndex(const glm::vec3 &position)
{
    glm::vec3 chunkSize(m_descriptors.m_chunkSize);

    return glm::floor(position/chunkSize);
}

template<typename _Block, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
unsigned int World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>::getChunkHash(const glm::vec3 &position)
{
    glm::vec3 chunkIndex=getChunkIndex(position);

    return chunkHash(chunkIndex);
}

}//namespace voxigen

#endif //_voxigen_world_h_