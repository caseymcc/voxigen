#ifndef _voxigen_coords_h_
#define _voxigen_coords_h_

#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace voxigen
{

inline glm::vec3 getCylindricalCoords(size_t width, size_t height, const glm::vec3 &position)
{
    glm::vec3 cylindricalPos;

    float theta=(glm::two_pi<float>()/width)*position.x;

    cylindricalPos.x=position.z*cos(theta);
    cylindricalPos.y=position.z*sin(theta);
    cylindricalPos.z=position.y;

    return cylindricalPos;
}

}//namespace voxigen

#endif //_voxigen_coords_h_