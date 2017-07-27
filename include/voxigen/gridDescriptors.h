#ifndef _voxigen_gridDescriptors_h_
#define _voxigen_gridDescriptors_h_

#include "voxigen/voxigen_export.h"
#include <glm/glm.hpp>

#include <string>

namespace voxigen
{

struct VOXIGEN_EXPORT GridDescriptors
{
    GridDescriptors();
    
    void create(std::string name, int seed, const glm::ivec3 &size, const glm::ivec3 &segmentSize, const glm::ivec3 &chunkSize);
    void load(std::string fileName);
    void save(std::string fileName);

    void init();

    unsigned int segmentHash(const glm::ivec3 &index) const;
    glm::ivec3 segmentIndex(unsigned int hash) const;
    glm::vec3 segmentOffset(unsigned int hash) const;

    unsigned int chunkHash(const glm::ivec3 &index) const;
    glm::ivec3 chunkIndex(unsigned int hash) const;
    glm::vec3 chunkOffset(unsigned int hash) const;

    std::string m_name;
    unsigned int m_seed;

    glm::ivec3 m_size;
    glm::ivec3 m_segmentSize; //from compile time value
    glm::ivec3 m_segmentCount; //calculated
    glm::ivec3 m_segmentStride; //calculated
    glm::ivec3 m_chunkSize; //from compile time value
    glm::ivec3 m_chunkCount; //calculated
    glm::ivec3 m_chunkStride; //calculated

    float m_noiseScale;
    double m_contientFrequency;
    int m_contientOctaves;
    double m_contientLacunarity;

    double m_seaLevel;
    double m_continentaShelf;
};

}//namespace voxigen

#endif //_voxigen_gridDescriptors_h_