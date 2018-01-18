#ifndef _voxigen_object_h_
#define _voxigen_object_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"

#include <memory>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include <glm/gtc/quaternion.hpp>

namespace voxigen
{

class VOXIGEN_EXPORT Object
{
public:
    Object();
    Object(const glm::vec3 &position);
    Object(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up);
    ~Object();

    const RegionHash getRegionHash() { return m_regionHash; }
    void setRegionHash(RegionHash &hash) { m_regionHash=hash; }

    const glm::vec3 &getPosition() { return m_position; }
    void setPosition(const glm::vec3 &position) { m_position=m_position; }

    const glm::quat &getOrientation() { return m_orientation; }
    void setOrientation(const glm::vec3 &direction);
    void setOrientation(const glm::vec3 &direction, const glm::vec3 &up);
    void setOrientation(const glm::quat &orientation) { m_orientation=orientation; }

private:
    RegionHash m_regionHash;

    glm::vec3 m_position;
    glm::quat m_orientation;

    static glm::vec3 m_worldForward;
    static glm::vec3 m_worldUp;
};

}//namespace voxigen

#endif //_voxigen_object_h_