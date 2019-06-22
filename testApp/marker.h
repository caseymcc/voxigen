#ifndef _voxigen_marker_h_
#define _voxigen_marker_h_

#include <glbinding/gl/gl.h>
using namespace gl;
#include <opengl_util/program.h>

class Marker
{
public:
    Marker();

    void init();
    void draw(glm::mat4x4 &projectionView, glm::vec3 &position);

private:
    static std::string m_vertMarkerShader;
    static std::string m_fragMarkerShader;

    static bool m_programInit;
    static opengl_util::Program m_program;

    static size_t m_projectionViewId;
    static size_t m_offsetId;

    unsigned int m_vertexArray;
    unsigned int m_vertexBufferID;
    unsigned int m_color;
};

#endif //_voxigen_marker_h_