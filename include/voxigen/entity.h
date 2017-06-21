#ifndef _voxigen_entity_h_
#define _voxigen_entity_h_

#include <memory>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include <glm/gtc/quaternion.hpp>

namespace voxigen
{

template<_Object>
class Entity
{
public:
    Entity(glm::vec3 direction=glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3 up=glm::vec3(0.0f, 0.0f, 1.0f)):
        m_object(direction, up) {}
    ~Entity() {}

    glm::vec3 getPosition() { return m_object.getPosition(); }
    void setPosition(const glm::vec3 &position) { m_object.setPosition(position); }

    void move(glm::vec3 &direction, float time)
    {
        float distance=(time-m_lastUpdate)*m_speed;
        glm::vec3 position=m_object.getPosition()+(direction*distance);

        m_object->setPosition(position);
    }

    glm::quat getOrientation() { return m_object.getOrientation(); }
    void setOrientation(const glm::vec3 &direction) { m_object.setOrientation(direction); }
    void setOrientation(const glm::vec3 &direction, const glm::vec3 &up) { m_object.setOrientation(direction, up); }
    void setOrientation(const glm::quat &orientation) { m_object.setOrientation(orientation); }

private:
    _Object m_object;

    float m_speed;
    float m_lastUpdate;
};

}//namespace voxigen

#endif //_voxigen_entity_h_