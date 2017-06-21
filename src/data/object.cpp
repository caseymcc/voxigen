#include "voxigen/object.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 Object::m_worldForward=glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Object::m_worldUp=glm::vec3(0.0f, 0.0f, 1.0f);

Object::Object():
m_position(0.0f, 0.0f, 0.0f)
{
    setOrientation(glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
}

Object::Object(glm::vec3 &position):
m_position(position)
{
    setOrientation(glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
}

Object::Object(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up):
m_position(position),
{
    setOrientation(direction, up);
}

Object::~Object()
{}

void Object::setOrientation(const glm::vec3 &direction)
{
    glm::quat orientation=getOrientation();
    glm::vec3 currentDirection=orientation*m_worldForward;

    if(glm::length2(currentDirection+direction) < 0.00005f) //reverse turn use up as rotation axis
        setOrientation(glm::angleAxis(glm::pi<float>(), m_worldUp));
    else
    {
        glm::quat rotation=rotateTo(currentDirection, direction);
        setOrientation(rotation*orientation);
    }
}

void Object::setOrientation(const glm::vec3 &direction, const glm::vec3 &up)
{
    glm::quat orientation=getOrientation();
    glm::vec3 currentDirection=orientation*m_worldForward;
    glm::quat forwardOrientation;

    if(glm::length2(currentDirection+direction) < 0.00005f) //reverse turn use up as rotation axis
        forwardOrientation=glm::angleAxis(glm::pi<float>(), m_worldUp);
    else
    {
        glm::quat rotation=rotateTo(currentDirection, direction);
        forwardOrientation=rotation*orientation;
    }

    glm::vec3 currentUp=forwardOrientation*m_worldUp;

    glm::quat upRotation=rotateTo(currentUp, up);
    setOrientation(upRotation*forwardOrientation);
}

