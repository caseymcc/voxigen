#include "marker.h"

#include "voxigen/rendering/simpleShapes.h"
#include <glm/gtc/type_ptr.hpp>

bool Marker::m_programInit=false;
opengl_util::Program Marker::m_program;
size_t Marker::m_projectionViewId;
size_t Marker::m_offsetId;

std::string Marker::m_vertMarkerShader=
"#version 330 core\n"
"layout (location = 0) in vec3 inputVertex;\n"
"layout (location = 1) in vec3 inputNormal;\n"
"layout (location = 2) in vec2 inputTexCoord;\n"
"layout (location = 3) in vec4 color;\n"
"\n"
"out vec3 position;\n"
"out vec3 normal;\n"
"out vec4 vertexColor;\n"
"\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"   normal=inputNormal;\n"
"   vertexColor=color;\n"
"   vec3 position=regionOffset+inputVertex;\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"
"}\n"
"";

std::string Marker::m_fragMarkerShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec4 vertexColor;\n"
"\n"
"out vec4 color;\n"
"\n"
"//uniform vec3 lightPos;\n"
"//uniform vec3 lightColor;\n"
"\n"
"void main()\n"
"{\n"
"   //vec3 lightDir=normalize(lightPos-position); \n"
"   //float diff=max(dot(normal, lightDir), 0.0); \n"
"   //vec3 diffuse=diff * lightColor; \n"
"   //color=vec4((ambient+diffuse)*vertexColor.xyz, 1.0f); \n"
"   color=vertexColor;\n"
"}\n"
"";

Marker::Marker()
{
}

void Marker::init()
{
//create player marker
    const std::vector<float> &playerVertices=voxigen::SimpleCube<1, 1, 2>::vertCoords;

    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(1, &m_vertexBufferID);
    glGenBuffers(1, &m_color);

    glBindVertexArray(m_vertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*playerVertices.size(), playerVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, m_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, glm::value_ptr(color), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(!m_programInit)
    {
        std::string error;

        if(!m_program.attachLoadAndCompileShaders(m_vertMarkerShader, m_fragMarkerShader, error))
        {
            assert(false);
            return;
        }

        m_projectionViewId=m_program.getUniformId("projectionView");
        m_offsetId=m_program.getUniformId("regionOffset");

        m_programInit=true;
    }
}

void Marker::draw(glm::mat4x4 &projectionView, glm::vec3 &position)
{
    m_program.use();

    m_program.uniform(m_projectionViewId)=projectionView;
    m_program.uniform(m_offsetId)=position;

    glBindVertexArray(m_vertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1);
}
