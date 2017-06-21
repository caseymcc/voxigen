#ifndef _voxigen_object_h_
#define _voxigen_object_h_

#include <memory>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include <glm/gtc/quaternion.hpp>

class Object
{
public:
    Object();
    Object(const glm::vec3 &position);
    Object(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up);
    ~Object();

    const glm::vec3 &getPosition() { return m_position; }
    void setPosition(const glm::vec3 &position) { m_position=m_position; }

    const glm::quat &getOrientation() { return m_orientation; }
    void setOrientation(const glm::vec3 &direction);
    void setOrientation(const glm::vec3 &direction, const glm::vec3 &up);
    void setOrientation(const glm::quat &orientation) { m_orientation=orientation; }

private:
    glm::vec3 m_position;
    glm::quat m_orientation;

    static glm::vec3 m_worldForward;
    static glm::vec3 m_worldUp;
};

#endif //_voxigen_object_h_