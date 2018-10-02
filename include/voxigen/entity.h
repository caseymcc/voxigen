#ifndef _voxigen_entity_h_
#define _voxigen_entity_h_

#include <memory>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include <glm/gtc/quaternion.hpp>

namespace voxigen
{

template<typename _Object>
class Entity
{
public:
    Entity(glm::vec3 direction=glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3 up=glm::vec3(0.0f, 0.0f, 1.0f)):
        m_object(direction, up) {}
    ~Entity() {}

    glm::vec3 getPosition() { return m_object.getPosition(); }
    void setPosition(const glm::vec3 &position) { m_object.setPosition(position); }
    void move(const glm::vec3 &direction);
    glm::quat getOrientation() { return m_object.getOrientation(); }
    void setOrientation(const glm::vec3 &direction) { m_object.setOrientation(direction); }
    void setOrientation(const glm::vec3 &direction, const glm::vec3 &up) { m_object.setOrientation(direction, up); }
    void setOrientation(const glm::quat &orientation) { m_object.setOrientation(orientation); }

private:
    _Object m_object;

    float m_speed;
    float m_lastUpdate;
};


template<typename _Object>
void Entity<_Object>::move(const glm::vec3 &velocity)
{
//    glm::vec3 direction=getDirection();
//    glm::vec3 right=glm::normalize(glm::cross(direction, m_worldUp));
//    glm::vec3 delta=(direction*velocity.x)+(right*velocity.y)+(m_worldUp*velocity.z);
//
//    m_position+=delta;
//    m_viewDirty=true;
}

}//namespace voxigen

#endif //_voxigen_entity_h_