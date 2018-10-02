#include "voxigen/simpleCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace voxigen
{

void Position::move(const glm::vec3 &delta)
{
    m_position+=delta;
}

SimpleFpsCamera::SimpleFpsCamera(glm::vec3 position, glm::vec3 direction):
m_projectionDirty(true),
m_viewDirty(true),
m_fov(glm::radians(60.0f)),
m_near(0.1f),
m_far(1000.0f),
m_width(640),
m_height(480),
m_worldUp(0.0f, 0.0f, 1.0f)
{
}

SimpleFpsCamera::~SimpleFpsCamera()
{}

void SimpleFpsCamera::setFov(float fov)
{
    m_fov=fov;
    m_projectionDirty=true;
}

void SimpleFpsCamera::setClipping(float localNear, float localFar)
{
    m_near=localNear;
    m_far=localFar;
    m_projectionDirty=true;
}

void SimpleFpsCamera::setView(size_t width, size_t height)
{
    m_width=width;
    m_height=height;
    m_projectionDirty=true;
}

void SimpleFpsCamera::setYaw(float angle)
{
    m_yaw=angle;
    
    if(m_yaw<0.0)
        m_yaw+=glm::two_pi<float>();
    else if(m_yaw>glm::two_pi<float>())
        m_yaw-=glm::two_pi<float>();
    m_viewDirty=true;
}

void SimpleFpsCamera::setPitch(float angle)
{
    m_pitch=angle;

//    if(m_pitch<0.0)
//        m_pitch+=glm::two_pi<float>();
//    else if(m_pitch>glm::two_pi<float>())
//        m_pitch-=glm::two_pi<float>();
    m_viewDirty=true;
}

void SimpleFpsCamera::moveDirection(const glm::vec3 &velocity, glm::vec3 &delta)
{
    glm::vec3 direction=getDirection();
    glm::vec3 right=glm::normalize(glm::cross(direction, m_worldUp));
    
    delta=(direction*velocity.x)+(right*velocity.y)+(m_worldUp*velocity.z);
    m_position+=delta;
    m_viewDirty=true;
}


glm::vec3 SimpleFpsCamera::getDirection()
{
    glm::vec3 direction;
    float r=cos(m_pitch);

    direction.x=r*cos(m_yaw);
    direction.y=r*sin(m_yaw);
    direction.z=sin(m_pitch);
    direction=glm::normalize(direction);

    return direction;
}

void SimpleFpsCamera::setDirection(const glm::vec3 &direction)
{
    m_yaw=atan(direction.y/direction.x);
    m_pitch=asin(direction.z);
    m_viewDirty=true;
}

void SimpleFpsCamera::updateMatrix()
{
    bool dirty=false;

    if(m_projectionDirty)
    {
        m_projectionDirty=false;

        float ratio=(float)m_width/m_height;

        m_projectionMatrix=glm::perspective(m_fov, ratio, m_near, m_far);
        dirty=true;
    }

    if(m_viewDirty)
    {
        m_viewDirty=false;

        glm::vec3 direction=getDirection();

        m_viewMatrix=glm::lookAt(m_position, m_position+direction, m_worldUp);
        dirty=true;
    }

    if(dirty)
        m_projectionViewMatrix=m_projectionMatrix*m_viewMatrix;
}

}//namespace voxigen