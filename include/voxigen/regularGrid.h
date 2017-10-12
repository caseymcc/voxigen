#ifndef _voxigen_regularGrid_h_
#define _voxigen_regularGrid_h_

#include "voxigen/biome.h"
#include "voxigen/chunk.h"
#include "voxigen/region.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/dataStore.h"
#include "voxigen/entity.h"
#include "voxigen/classFactory.h"
#include "voxigen/updateQueue.h"

#include <noise/noise.h>

#include <unordered_map>
#include <memory>
#include <limits>

#include <boost/filesystem.hpp>
namespace fs=boost::filesystem;

namespace voxigen
{

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
using UniqueChunkMap=std::unordered_map<ChunkHash, UniqueChunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>>;

template<typename _Cell, size_t _ChunkSizeX=64, size_t _ChunkSizeY=64, size_t _ChunkSizeZ=64, size_t _RegionSizeX=16, size_t _RegionSizeY=16, size_t _RegionSizeZ=16>
class RegularGrid
{
public:
    RegularGrid();
    ~RegularGrid();

    typedef std::integral_constant<size_t, _ChunkSizeX*_RegionSizeX> regionCellSizeX;
    typedef std::integral_constant<size_t, _ChunkSizeY*_RegionSizeY> regionCellSizeY;
    typedef std::integral_constant<size_t, _ChunkSizeZ*_RegionSizeZ> regionCellSizeZ;

    typedef Chunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef Region<ChunkType, _RegionSizeX, _RegionSizeY, _RegionSizeZ> RegionType;
    typedef RegionHandle<RegionType> RegionHandleType;
    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;
//    typedef std::unordered_map<RegionHash, SharedRegion> SharedRegionMap;

//    typedef UniqueChunkMap<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> UniqueChunkMap;
//    typedef std::shared_ptr<ChunkType> SharedChunk;
//    typedef std::unordered_map<ChunkHash, SharedChunk> SharedChunkMap;

////registers default generators
//    typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<ChunkType>> EquiRectWorldGenerator;

    void create(std::string directory, std::string name, glm::ivec3 size, std::string generatorName);
    void load(std::string directory);
    void save();

    SharedRegionHandle getRegion(const glm::ivec3 &index);
    SharedRegionHandle getRegion(RegionHash hash);
//    RegionHash getRegionHash(const glm::vec3 &gridPosition);

    glm::ivec3 size();
    glm::ivec3 regionCellSize();

    SharedChunkHandle getChunk(const glm::ivec3 &index);
    SharedChunkHandle getChunk(RegionHash regionHash, ChunkHash chunkHash);
    void loadChunk(SharedChunkHandle chunkHandle, size_t lod);
    std::vector<Key> getUpdatedChunks();

    RegionHash getRegionHash(const glm::ivec3 &index);
    glm::ivec3 getRegionIndex(const glm::vec3 &position);
    glm::ivec3 getRegionIndex(RegionHash hash);

    glm::ivec3 getChunkIndex(const glm::vec3 &position);
    ChunkHash chunkHash(const glm::ivec3 &chunkIndex) const;
    ChunkHash getChunkHash(RegionHash regionHash, const glm::vec3 &gridPosition);
    ChunkHash getChunkHash(const glm::vec3 &gridPosition);

    Key getHashes(const glm::vec3 &gridPosition);
    Key getHashes(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);

    

//Sizes
    glm::ivec3 getChunkSize();

    glm::vec3 gridPosToRegionPos(RegionHash regionHash, const glm::vec3 &gridPosition);

    GridDescriptors &getDescriptors() { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

private:
    void loadRegions(std::string directory);

    std::string m_directory;
    std::string m_name;

    GridDescriptors m_descriptors;
//    ChunkHandler<ChunkType> m_chunkHandler;

    GeneratorQueue<ChunkType> m_generatorQueue;
    SharedGenerator m_generator;
    DataStore<RegionType, ChunkType> m_dataStore;
    UpdateQueue m_updateQueue;

    glm::mat4 m_transform;
};

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::RegularGrid():
m_dataStore(&m_descriptors, &m_generatorQueue, &m_updateQueue),
m_generatorQueue(&m_descriptors, &m_updateQueue)
//m_chunkHandler(&m_descriptors)
{
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::~RegularGrid()
{
    m_dataStore.terminate();
    m_generatorQueue.terminate();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::create(std::string directory, std::string name, glm::ivec3 size, std::string generatorName)
{
    m_name=name;
    m_directory=directory;

    m_descriptors.create(name, 0, size, glm::ivec3(_RegionSizeX, _RegionSizeY, _RegionSizeZ), glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ));
    m_descriptors.init();
    m_descriptors.m_generator=generatorName;

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile);
    
    std::string regionDirectory=directory+"/regions";
    fs::path regionPath(regionDirectory);

    fs::create_directory(regionPath);

    m_generator=createClass<Generator>(generatorName);
    m_generatorQueue.setGenerator(m_generator.get());

    m_generator->initialize(&m_descriptors);
    m_dataStore.initialize();
    m_generatorQueue.initialize();

}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::load(std::string directory)
{
    m_directory=directory;

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.load(configFile);

    m_generator=createClass<Generator>(m_descriptors.m_generator);
    m_generatorQueue.setGenerator(m_generator.get());

    std::string regionDirectory=directory+"/regions";
    m_dataStore.load(regionDirectory);

    m_generator->initialize(&m_descriptors);
    m_dataStore.initialize();
    m_generatorQueue.initialize();

//    std::string chunkDirectory=directory+"/chunks";
//
//    m_chunkHandler.load(chunkDirectory);
//    m_chunkHandler.initialize();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::save()
{
    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile)
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
//Biome &RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getBiome(glm::ivec3 cell)
//{}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::SharedRegionHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getRegion(const glm::ivec3 &index)
{
    RegionHash hash=m_descriptors.regionHash(index);

    return m_dataStore.getRegion(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::SharedRegionHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getRegion(RegionHash hash)
{
    return m_dataStore.getRegion(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::size()
{
    return m_descriptors.m_size;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::regionCellSize()
{
    return glm::ivec3(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getChunk(const glm::ivec3 &cell)
{
    glm::ivec3 chunkIndex=cell/m_descriptors.m_chunkSize;

    ChunkHash chunkHash=chunkHash(chunkIndex);

    return m_chunkHandler.getChunk(chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getChunk(RegionHash regionHash, ChunkHash chunkHash)
{
    return m_dataStore.getChunk(regionHash, chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::loadChunk(SharedChunkHandle chunkHandle, size_t lod)
{
    m_dataStore.loadChunk(chunkHandle, lod);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
std::vector<Key> RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getUpdatedChunks()
{
    std::vector<Key> updatedChunks;

    updatedChunks=m_updateQueue.get();
    return updatedChunks;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
RegionHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getRegionHash(const glm::ivec3 &index)
{
    return m_descriptors.regionHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getRegionIndex(const glm::vec3 &position)
{
    return m_descriptors.regionIndex(position);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getRegionIndex(RegionHash hash)
{
    return m_descriptors.regionIndex(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::chunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getChunkIndex(const glm::vec3 &position)
{
    glm::ivec3 pos=glm::floor(position);

    return pos/m_descriptors.m_chunkSize;
}


template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
Key RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getHashes(const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=position/regionCellSize;
    glm::ivec3 chunkIndex=position-(regionIndex*regionCellSize);

    return Key(m_descriptors.regionHash(regionIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
Key RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getHashes(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    return Key(m_descriptors.regionHash(regionIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getChunkHash(RegionHash regionHash, const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);

    return m_descriptors.chunkHash(position);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getChunkHash(const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=position/regionCellSize;
    glm::ivec3 chunkIndex=position-(regionIndex*regionCellSize);

    return m_descriptors.chunkHash(chunkIndex);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::getChunkSize()
{
    return glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ>
glm::vec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ>::gridPosToRegionPos(RegionHash regionHash, const glm::vec3 &gridPosition)
{
    glm::vec3 pos=gridPosition-m_descriptors.regionOffset(playerRegion);

    return pos;
}

}//namespace voxigen

#endif //_voxigen_regularGrid_h_