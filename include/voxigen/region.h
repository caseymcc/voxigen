//Regions are the basis for the physics of the game engine
//All physics events act per region, and are centered on the region
//There is an overlap region where the regions will pass information 
//to maintain a full simulation across the entire world

#ifndef _voxigen_region_h_
#define _voxigen_region_h_

#include "voxigen/defines.h"
#include "voxigen/chunk.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/dataStore.h"
#include "voxigen/entity.h"

#include <noise/noise.h>

#include <unordered_map>
#include <memory>
#include <limits>

namespace voxigen
{

template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
class Region
{
public:
//defines
    typedef std::integral_constant<size_t, _ChunksX> sizeX;
    typedef std::integral_constant<size_t, _ChunksY> sizeY;
    typedef std::integral_constant<size_t, _ChunksZ> sizeZ;

    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef std::shared_ptr<ChunkType> SharedChunk;
    typedef std::unordered_map<ChunkHash, SharedChunk> SharedChunkMap;

//Class
    Region(RegionHash hash, GridDescriptors *descriptors);
    ~Region();

//    SharedChunkHandle getChunk(const glm::ivec3 &index);
//    SharedChunkHandle getChunk(ChunkHash chunkHash);
//    std::vector<Key> getUpdatedChunks();
//
//    glm::ivec3 getChunkIndex(const glm::vec3 &position);
//    ChunkHash chunkHash(const glm::ivec3 &chunkIndex) const;
//    ChunkHash getChunkHash(const glm::vec3 &position);

private:
    std::string m_directory;
    std::string m_name;

    RegionHash m_hash;
};

template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::Region(RegionHash hash, GridDescriptors *descriptors):
m_hash(hash),
m_descriptors(descriptors)
{
}

template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::~Region()
{
}

//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//void Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::load(std::string directory)
//{
//}
//
//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//void Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::save()
//}
//
//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//typename Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::SharedChunkHandle Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::getChunk(const glm::ivec3 &cell)
//{
//    glm::ivec3 chunkIndex=cell/m_descriptors.m_chunkSize;
//
//    ChunkHash chunkHash=chunkHash(chunkIndex);
//
//    return m_chunkHandler.getChunk(chunkHash);
//}
//
//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//typename Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::SharedChunkHandle Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::getChunk(ChunkHash chunkHash)
//{
//    return m_chunkHandler.getChunk(chunkHash);
//}
//
//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//std::vector<Key> Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::getUpdatedChunks()
//{
//    return m_chunkHandler.getUpdatedChunks();
//}
//
//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//ChunkHash Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::chunkHash(const glm::ivec3 &index) const
//{
//    return m_descriptors.chunkHash(index);
//}
//
//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//glm::ivec3 Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::getChunkIndex(const glm::vec3 &position)
//{
//    glm::vec3 chunkSize(m_descriptors.m_chunkSize);
//
//    return glm::floor(position/chunkSize);
//}
//
//template<typename _Chunk, size_t _ChunksX, size_t _ChunksY, size_t _ChunksZ>
//ChunkHash Region<_Chunk, _ChunksX, _ChunksY, _ChunksZ>::getChunkHash(const glm::vec3 &position)
//{
//    glm::vec3 chunkIndex=getChunkIndex(position);
//
//    return chunkHash(chunkIndex);
//}

}//namespace voxigen

#endif //_voxigen_worldRegion_h_