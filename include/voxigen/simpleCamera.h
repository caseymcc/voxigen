#ifndef _voxigen_simpleCamera_h_
#define _voxigen_simpleCamera_h_

#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>

namespace voxigen
{

struct VOXIGEN_EXPORT SimpleCamera
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
};

}//namespace voxigen

#endif //_voxigen_simpleCamera_h_