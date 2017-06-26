#ifndef _voxigen_coords_h_
#define _voxigen_coords_h_

#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>

namespace voxigen
{

VOXIGEN_EXPORT glm::vec3 getCylindricalCoords(size_t width, size_t height, const glm::vec3 &position);

}//namespace voxigen

#endif //_voxigen_coords_h_