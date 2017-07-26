#ifndef _voxigen_worldDescriptors_h_
#define _voxigen_worldDescriptors_h_

#include "voxigen/voxigen_export.h"
#include <glm/glm.hpp>

#include <string>

namespace voxigen
{

struct VOXIGEN_EXPORT WorldDescriptors
{
    WorldDescriptors();
    
    void create(std::string name, int seed, const glm::ivec3 &size, const glm::ivec3 &chunkSize);
    void load(std::string fileName);
    void save(std::string fileName);

    void init();

    unsigned int chunkHash(const glm::ivec3 &chunkIndex) const;
    glm::ivec3 chunkIndex(unsigned int chunkHash) const;
    glm::vec3 chunkOffset(unsigned int chunkHash) const;

    std::string m_name;
    unsigned int m_seed;

    glm::ivec3 m_size;
    glm::ivec3 m_chunkSize;
    glm::ivec3 m_chunkCount;//calculated
    glm::ivec3 m_chunkStride;//calculated

    float m_noiseScale;
    double m_contientFrequency;
    int m_contientOctaves;
    double m_contientLacunarity;

    double m_seaLevel;
    double m_continentaShelf;
};

}//namespace voxigen

#endif //_voxigen_worldDescriptors_h_