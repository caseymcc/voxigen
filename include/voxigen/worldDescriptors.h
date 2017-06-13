#ifndef _voxigen_worldDescriptors_h_
#define _voxigen_worldDescriptors_h_

#include "voxigen/voxigen_export.h"
#include <glm/glm.hpp>

namespace voxigen
{

struct VOXIGEN_EXPORT WorldDescriptors
{
    unsigned int m_seed;

    glm::ivec3 m_size;
    glm::ivec3 m_chunkSize;

    double m_contientFrequency;
    double m_contientLacunarity;

    double m_seaLevel;
    double m_continentaShelf;
};

}//namespace voxigen

#endif //_voxigen_worldDescriptors_h_