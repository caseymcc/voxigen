#ifndef _voxigen_regularGrid_h_
#define _voxigen_regularGrid_h_

#include "voxigen/biome.h"
#include "voxigen/chunk.h"
#include "voxigen/segment.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/dataStore.h"
#include "voxigen/entity.h"
#include "voxigen/classFactory.h"

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

template<typename _Cell, size_t _ChunkSizeX=64, size_t _ChunkSizeY=64, size_t _ChunkSizeZ=64, size_t _SegmentSizeX=16, size_t _SegmentSizeY=16, size_t _SegmentSizeZ=16>
class RegularGrid
{
public:
    RegularGrid();
    ~RegularGrid();

    typedef std::integral_constant<size_t, _ChunkSizeX*_SegmentSizeX> segmentCellSizeX;
    typedef std::integral_constant<size_t, _ChunkSizeY*_SegmentSizeY> segmentCellSizeY;
    typedef std::integral_constant<size_t, _ChunkSizeZ*_SegmentSizeZ> segmentCellSizeZ;

    typedef Chunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef Segment<ChunkType, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ> SegmentType;
    typedef SegmentHandle<SegmentType> SegmentHandleType;
    typedef std::shared_ptr<SegmentHandleType> SharedSegmentHandle;
//    typedef std::unordered_map<SegmentHash, SharedSegment> SharedSegmentMap;

//    typedef UniqueChunkMap<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> UniqueChunkMap;
//    typedef std::shared_ptr<ChunkType> SharedChunk;
//    typedef std::unordered_map<ChunkHash, SharedChunk> SharedChunkMap;

////registers default generators
//    typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<ChunkType>> EquiRectWorldGenerator;

    void create(std::string directory, std::string name, glm::ivec3 size, std::string generatorName);
    void load(std::string directory);
    void save();

    SharedSegmentHandle getSegment(const glm::ivec3 &index);
    SharedSegmentHandle getSegment(SegmentHash hash);
//    SegmentHash getSegmentHash(const glm::vec3 &gridPosition);

    glm::ivec3 segmentCellSize();

    SharedChunkHandle getChunk(const glm::ivec3 &index);
    SharedChunkHandle getChunk(SegmentHash segmentHash, ChunkHash chunkHash);
    std::vector<SegmentChunkHash> getUpdatedChunks();

    SegmentHash getSegmentHash(const glm::ivec3 &index);
    glm::ivec3 getSegmentIndex(const glm::vec3 &position);
    glm::ivec3 getSegmentIndex(SegmentHash hash);

    glm::ivec3 getChunkIndex(const glm::vec3 &position);
    ChunkHash chunkHash(const glm::ivec3 &chunkIndex) const;
    ChunkHash getChunkHash(SegmentHash segmentHash, const glm::vec3 &gridPosition);
    ChunkHash getChunkHash(const glm::vec3 &gridPosition);

    SegmentChunkHash getHashes(const glm::vec3 &gridPosition);
    SegmentChunkHash getHashes(const glm::ivec3 &segmentIndex, const glm::ivec3 &chunkIndex);

    

    glm::vec3 gridPosToSegmentPos(SegmentHash segmentHash, const glm::vec3 &gridPosition);

    GridDescriptors &getDescriptors() { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

private:
    void loadSegments(std::string directory);

    std::string m_directory;
    std::string m_name;

    GridDescriptors m_descriptors;
//    ChunkHandler<ChunkType> m_chunkHandler;

    GeneratorQueue<ChunkType> m_generatorQueue;
    SharedGenerator m_generator;
    DataStore<SegmentType, ChunkType> m_dataStore;

    glm::mat4 m_transform;
};

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::RegularGrid():
m_dataStore(&m_descriptors, &m_generatorQueue),
m_generatorQueue(&m_descriptors)
//m_chunkHandler(&m_descriptors)
{
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::~RegularGrid()
{
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::create(std::string directory, std::string name, glm::ivec3 size, std::string generatorName)
{
    m_name=name;
    m_directory=directory;

    m_descriptors.create(name, 0, size, glm::ivec3(_SegmentSizeX, _SegmentSizeY, _SegmentSizeZ), glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ));
    m_descriptors.init();
    m_descriptors.m_generator=generatorName;

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile);
    
    std::string segmentDirectory=directory+"/segments";
    fs::path segmentPath(segmentDirectory);

    fs::create_directory(segmentPath);

    m_generator=createClass<Generator>(generatorName);
    m_generatorQueue.setGenerator(m_generator.get());

    m_generator->initialize(&m_descriptors);
    m_dataStore.initialize();
    m_generatorQueue.initialize();

}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::load(std::string directory)
{
    m_directory=directory;

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.load(configFile);

    m_generator=createClass<Generator>(m_descriptors.m_generator);
    m_generatorQueue.setGenerator(m_generator.get());

    std::string segmentDirectory=directory+"/segments";
    m_dataStore.load(segmentDirectory);

    m_generator->initialize(&m_descriptors);
    m_dataStore.initialize();
    m_generatorQueue.initialize();

//    std::string chunkDirectory=directory+"/chunks";
//
//    m_chunkHandler.load(chunkDirectory);
//    m_chunkHandler.initialize();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::save()
{
    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile)
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
//Biome &RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getBiome(glm::ivec3 cell)
//{}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedSegmentHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getSegment(const glm::ivec3 &index)
{
    SegmentHash hash=m_descriptors.segmentHash(index);

    return m_dataStore.getSegment(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedSegmentHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getSegment(SegmentHash hash)
{
    return m_dataStore.getSegment(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::segmentCellSize()
{
    return glm::ivec3(segmentCellSizeX::value, segmentCellSizeY::value, segmentCellSizeZ::value);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunk(const glm::ivec3 &cell)
{
    glm::ivec3 chunkIndex=cell/m_descriptors.m_chunkSize;

    ChunkHash chunkHash=chunkHash(chunkIndex);

    return m_chunkHandler.getChunk(chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunk(SegmentHash segmentHash, ChunkHash chunkHash)
{
    return m_dataStore.getChunk(segmentHash, chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
std::vector<SegmentChunkHash> RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getUpdatedChunks()
{
    std::vector<SegmentChunkHash> updatedChunks;

//TODO: fix
    updatedChunks=m_generatorQueue.getUpdated();
    return updatedChunks;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
SegmentHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getSegmentHash(const glm::ivec3 &index)
{
    return m_descriptors.segmentHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getSegmentIndex(const glm::vec3 &position)
{
    return m_descriptors.segmentIndex(position);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getSegmentIndex(SegmentHash hash)
{
    return m_descriptors.segmentIndex(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::chunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunkIndex(const glm::vec3 &position)
{
    glm::ivec3 pos=glm::floor(position);

    return pos/m_descriptors.m_chunkSize;
}


template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
SegmentChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getHashes(const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 segmentCellSize(segmentCellSizeX::value, segmentCellSizeY::value, segmentCellSizeZ::value);
    glm::ivec3 segmentIndex=position/segmentCellSize;
    glm::ivec3 chunkIndex=position-(segmentIndex*segmentCellSize);

    return SegmentChunkHash(m_descriptors.segmentHash(segmentIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
SegmentChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getHashes(const glm::ivec3 &segmentIndex, const glm::ivec3 &chunkIndex)
{
    return SegmentChunkHash(m_descriptors.segmentHash(segmentIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunkHash(SegmentHash segmentHash, const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);

    return m_descriptors.chunkHash(position);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunkHash(const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 segmentCellSize(segmentCellSizeX::value, segmentCellSizeY::value, segmentCellSizeZ::value);
    glm::ivec3 segmentIndex=position/segmentCellSize;
    glm::ivec3 chunkIndex=position-(segmentIndex*segmentCellSize);

    return m_descriptors.chunkHash(chunkIndex);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
glm::vec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::gridPosToSegmentPos(SegmentHash segmentHash, const glm::vec3 &gridPosition)
{
    glm::vec3 pos=gridPosition-m_descriptors.segmentOffset(playerSegment);

    return pos;
}

}//namespace voxigen

#endif //_voxigen_regularGrid_h_