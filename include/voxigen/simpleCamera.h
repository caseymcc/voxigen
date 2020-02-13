#ifndef _voxigen_simpleFpsCamera_h_
#define _voxigen_simpleFpsCamera_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"
#include <memory>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace voxigen
{

class VOXIGEN_EXPORT Position
{
public:
    void setPosition(const glm::ivec3 &regionIndex, const glm::vec3 &position) { m_regionIndex=regionIndex; m_position=position; }
    void setPosition(const glm::vec3 &position) { m_position=position; }
    const glm::vec3 &getPosition() { return m_position; }
    const glm::ivec3 &getRegionIndex() { return m_regionIndex; }

    void move(const glm::vec3 &delta);

protected:
//    float m_yaw;
//    float m_pitch;
    glm::ivec3 m_regionIndex;
    glm::vec3 m_position;

    glm::vec3 m_worldUp;
};

class VOXIGEN_EXPORT SimpleFpsCamera
{
public:
    SimpleFpsCamera(glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 direction=glm::vec3(0.0f, 1.0f, 0.0f));
    ~SimpleFpsCamera();

    void forceUpdate();

    void setFov(float fov);
    void setClipping(float near, float far);
    void setView(size_t width, size_t height);

    void setYaw(float angle);
    void setPitch(float angle);

    const RegionHash getRegionHash() { return m_regionHash; }
    const glm::vec3 &getPosition() { return m_position; }
    
    void setPosition(const glm::vec3 &position) { m_position=position; }
    void setPosition(RegionHash regionHash, const glm::vec3 &position) { m_regionHash=regionHash;  m_position=position; }
    void moveDirection(const glm::vec3 &velocity, glm::vec3 &delta);

    bool isDirty() { return (m_projectionDirty||m_viewDirty); }
    void clearDirty() { m_projectionDirty=false; m_viewDirty=false; }
    glm::mat4x4 &getProjectionViewMat() { updateMatrix(); return m_projectionViewMatrix; }

    glm::vec3 getDirection();
    void setDirection(const glm::vec3 &direction);

private:
    void updateMatrix();

    float m_fov;
    float m_near;
    float m_far;

    RegionHash m_regionHash;
    glm::vec3 m_position;
    float m_yaw;
    float m_pitch;

    glm::vec3 m_worldUp;

    size_t m_width;
    size_t m_height;

    bool m_projectionDirty;
    glm::mat4 m_projectionMatrix;
    bool m_viewDirty;
    glm::mat4 m_viewMatrix;

    glm::mat4 m_projectionViewMatrix;
};

}//namespace voxigen

#endif //_voxigen_simpleFpsCamera_h_