#ifndef _voxigen_worldDescriptors_h_
#define _voxigen_worldDescriptors_h_

#include "voxigen/voxigen_export.h"
#include <glm/glm.hpp>

namespace voxigen
{

struct VOXIGEN_EXPORT WorldDescriptors
{
    WorldDescriptors();
    void init();

    unsigned int chunkHash(const glm::ivec3 &chunkIndex) const;
    glm::ivec3 chunkIndex(unsigned int chunkHash) const;

    unsigned int seed;

    glm::ivec3 size;
    glm::ivec3 chunkSize;
    glm::ivec3 chunkCount;//calculated
    glm::ivec3 chunkStride;//calculated

    float noiseScale;
    double contientFrequency;
    int contientOctaves;
    double contientLacunarity;

    double seaLevel;
    double continentaShelf;
};

}//namespace voxigen

#endif //_voxigen_worldDescriptors_h_