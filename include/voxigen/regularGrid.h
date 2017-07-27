#ifndef _voxigen_regularGrid_h_
#define _voxigen_regularGrid_h_

#include "voxigen/biome.h"
#include "voxigen/chunk.h"
#include "voxigen/segment.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/chunkHandler.h"
#include "voxigen/chunkCache.h"
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
using UniqueChunkMap=std::unordered_map<unsigned int, UniqueChunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>>;

template<typename _Cell, size_t _ChunkSizeX=64, size_t _ChunkSizeY=64, size_t _ChunkSizeZ=64, size_t _SegmentSizeX=16, size_t _SegmentSizeY=16, size_t _SegmentSizeZ=16>
class RegularGrid
{
public:
    RegularGrid();
    ~RegularGrid();

    typedef Chunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef Segment<ChunkType, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ> SegmentType;
    typedef std::shared_ptr<SegmentType> SharedSegment;
    typedef std::unordered_map<unsigned int, SharedSegment> SharedSegmentMap;

    typedef UniqueChunkMap<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> UniqueChunkMap;
    typedef std::shared_ptr<ChunkType> SharedChunk;
    typedef std::unordered_map<unsigned int, SharedChunk> SharedChunkMap;

////registers default generators
//    typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<ChunkType>> EquiRectWorldGenerator;

    void create(std::string directory, std::string name, glm::ivec3 size, std::string generatorName);
    void load(std::string directory);
    void save();

    SharedSegment getSegment(const glm::ivec3 &index);
    SharedSegment getSegment(unsigned int hash);

    SharedChunkHandle getChunk(const glm::ivec3 &index);
    SharedChunkHandle getChunk(unsigned int segmentHash, unsigned int chunkHas);
    std::vector<unsigned int> getUpdatedChunks();

    glm::ivec3 getChunkIndex(const glm::vec3 &position);
    unsigned int chunkHash(const glm::ivec3 &chunkIndex) const;
    unsigned int getChunkHash(const glm::vec3 &position);

    GridDescriptors &getDescriptors() { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

private:
    void loadSegments(std::string directory);

    std::string m_directory;
    std::string m_name;

    GridDescriptors m_descriptors;
//    ChunkHandler<ChunkType> m_chunkHandler;

    SharedGenerator m_generator;
    ChunkCache<ChunkType> m_chunkCache;

    SharedSegmentMap m_segments;

    glm::mat4 m_transform;
};

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::RegularGrid():
m_chunkCache(&m_descriptors)
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

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile);
    
    std::string segmentDirectory=directory+"/segments";
    fs::path segmentPath(segmentDirectory);

    fs::create_directory(segmentPath);

    m_generator=createClass<Generator>(generatorName);

    m_chunkCache.initialize();
//    m_generator->initialize();
    loadSegments(segmentDirectory);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::load(std::string directory)
{
    m_directory=directory;

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.load(configFile);

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

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::loadSegments(std::string directory)
{
    fs::path segmentPath(directory);

    for(auto &entry:fs::directory_iterator(segmentPath))
    {
        if(fs::is_directory(entry.path()))
        {
            std::istringstream fileNameStream(entry.path().string());
            unsigned int hash;

            fileNameStream>>std::hex>>hash;

            SharedSegment segment=std::make_shared<SegmentType>(hash, &m_descriptors, m_generator.get(), &m_chunkCache);
            
            segment->load(entry.path().string());
            m_segments.insert(SharedSegmentMap::value_type(hash, segment));
        }
    }
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
//Biome &RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getBiome(glm::ivec3 cell)
//{}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedSegment RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getSegment(const glm::ivec3 &index)
{
    unsigned int hash=m_descriptors.segmentHash(index);
    auto &iter=m_segments.find(hash);
    SharedSegment segment;

    if(iter==m_segments.end())
    {
        segment=std::make_shared<SegmentType>(hash, &m_descriptors, m_generator.get(), &m_chunkCache);

        m_segments.insert(SharedSegmentMap::value_type(hash, segment));
    }
    else
        segment=iter.second;

    return segment
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedSegment RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getSegment(unsigned int hash)
{
    auto &iter=m_segments.find(hash);
    SharedSegment segment;

    if(iter==m_segments.end())
    {
        segment=std::make_shared<SegmentType>(hash, &m_descriptors, m_generator.get(), &m_chunkCache);

        m_segments.insert(SharedSegmentMap::value_type(hash, segment));
    }
    else
        segment=iter->second;

    return segment;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunk(const glm::ivec3 &cell)
{
    glm::ivec3 chunkIndex=cell/m_descriptors.m_chunkSize;

    unsigned int chunkHash=chunkHash(chunkIndex);

    return m_chunkHandler.getChunk(chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunk(unsigned int segmentHash, unsigned int chunkHash)
{
    SharedSegment segment=getSegment(segmentHash);
    return segment->getChunk(chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
std::vector<unsigned int> RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getUpdatedChunks()
{
    std::vector<unsigned int> updatedChunks;

//TODO: fix
    return updatedChunks;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
unsigned int RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::chunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunkIndex(const glm::vec3 &position)
{
    glm::vec3 chunkSize(m_descriptors.m_chunkSize);

    return glm::floor(position/chunkSize);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _SegmentSizeX, size_t _SegmentSizeY, size_t _SegmentSizeZ>
unsigned int RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _SegmentSizeX, _SegmentSizeY, _SegmentSizeZ>::getChunkHash(const glm::vec3 &position)
{
    glm::vec3 chunkIndex=getChunkIndex(position);

    return chunkHash(chunkIndex);
}

}//namespace voxigen

#endif //_voxigen_regularGrid_h_