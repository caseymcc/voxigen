#ifndef _voxigen_gridDescriptors_h_
#define _voxigen_gridDescriptors_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"

#include <glm/glm.hpp>

#include <string>

namespace voxigen
{

struct VOXIGEN_EXPORT GridDescriptors
{
    GridDescriptors();
    
    void create(std::string name, int seed, const glm::ivec3 &size, const glm::ivec3 &regionSize, const glm::ivec3 &chunkSize);
    void load(std::string fileName);
    void save(std::string fileName);

    void init();

    RegionHash regionHash(const glm::ivec3 &index) const;
    glm::ivec3 regionIndex(RegionHash hash) const;
    glm::ivec3 regionIndex(const glm::vec3 &pos) const;
    glm::vec3 regionOffset(RegionHash hash) const;

    glm::vec3 adjustRegion(glm::ivec3 &regionIndex, glm::ivec3 &chunkIndex) const;
    float distance(glm::ivec3 &regionIndex1, glm::ivec3 &chunkIndex1, glm::ivec3 &regionIndex2, glm::ivec3 &chunkIndex2) const;

    ChunkHash chunkHash(const glm::ivec3 &index) const;
    glm::ivec3 chunkIndex(ChunkHash hash) const;
    glm::vec3 chunkOffset(ChunkHash hash) const;

    std::string m_name;
    unsigned int m_seed;
    std::string m_generator;

    glm::ivec3 m_size;
    glm::ivec3 m_regionSize; //from compile time value
    glm::ivec3 m_regionCellSize; //calculated
    glm::ivec3 m_regionCount; //calculated
    glm::ivec3 m_regionStride; //calculated
    glm::ivec3 m_chunkSize; //from compile time value
    glm::ivec3 m_chunkCount; //calculated
    glm::ivec3 m_chunkStride; //calculated

    float m_noiseScale;
    float m_contientFrequency;
    int m_contientOctaves;
    float m_contientLacunarity;

    float m_seaLevel;
    float m_continentaShelf;
};

}//namespace voxigen

#endif //_voxigen_gridDescriptors_h_