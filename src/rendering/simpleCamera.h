#ifndef _Camera_h_
#define _Camera_h_

#include <memory>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include "node3d.h"



//#include <gl/gl.h>
//#include <gl/glext.h>

class Camera:public Node3d
{
public:
    Camera(glm::vec3 direction=glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f));
    virtual ~Camera();

    void updateGlState(OpenglInterface *openglInterface);
    
    void setFov(float fov);
    void setClipping(float near, float far);
    void setView(GLint x, GLint y, GLsizei width, GLsizei height);

    glm::mat4x4 &projectionView() { updateMatrix(); return m_projectionViewMatrix; }

    virtual void setDirection(const glm::vec3 &direction);
    virtual void setDirection(const glm::vec3 &direction, const glm::vec3 &up);
    virtual void setOrientation(const glm::quat &orientation);

    void setViewportDirty() { m_viewPortDirty=true; }

private:
    bool updateMatrix();

    float m_fov;
    float m_near;
    float m_far;

    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_up;

    GLint m_x;
    GLint m_y;
    GLsizei m_width;
    GLsizei m_height;

    bool m_projectionDirty;
    glm::mat4 m_projectionMatrix;
    bool m_viewDirty;
    glm::mat4 m_viewMatrix;
    bool m_viewPortDirty;

    glm::mat4 m_projectionViewMatrix;
    
};

#endif //_Camera_h_