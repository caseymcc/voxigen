#ifndef _voxigen_simpleRenderer_h_
#define _voxigen_simpleRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/initGlew.h"
#include "voxigen/world.h"

#include <string>
#include <glm/glm.hpp>
#include <opengl_util/program.h>

namespace voxigen
{

struct SimpleCube
{
    static std::vector<float> vertCoords;
};

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleChunkRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Block>
class SimpleChunkRenderer
{
public:
    SimpleChunkRenderer(Chunk<_Block> *chunk) {}
    ~SimpleChunkRenderer() {}
    
    void build(unsigned int instanceData);
    void update();

private:
    Chunk<_Block> *chunk;

    unsigned int m_vertextArray;

    unsigned int m_offsetVBO;
    std::vector<glm::vec3> m_blockOffset;
};

template<typename _Block>
void SimpleChunkRenderer<_Block>::build(unsigned int instanceData)
{
    glGenVertexArrays(1, &m_vertextArray);

    glGenBuffers(1, &m_offsetVBO);
    update();

    glBindVertexArray(m_vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceData);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(1, 1);
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::update()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*100, &translations[0], GL_STATIC_DRAW);
}

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Block>
class SimpleRenderer
{
public:
    SimpleRenderer(World<_Block> *world);
    ~SimpleRenderer();

    void build();
    void update();
    void updateProjection();
    void updateView();
    void draw();

private:
    static std::string vertShader;
    static std::string fragmentShader;

    glm::vec3 m_position;
    float m_viewRadius;

    World<_Block> *m_world;

    std::vector<SimpleChunkRenderer<_Block>> m_chunkRenders;

    opengl_util::Program m_program;
    glm::mat4 m_projectionMat;
    glm::mat4 m_viewMat;

    unsigned int m_instanceVertices;
    unsigned int m_instanceTexCoords;
};

template<typename _Block>
std::string SimpleRenderer<_Block>::vertShader=
"#version 330 core\n"
"layout (location = 0) in vec3 blockvertex;\n"
"layout (location = 1) in vec2 blockTexCoord;\n"
"layout (location = 2) in vec3 blockOffset;\n"
"\n"
"out vec2 texCoords\n"
"\n"
"uniform mat4 projectionView;\n"
"uniform mat4 chunkTranslate;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position=projectionView*vec4(chunkTranslate+blockOffset+vertex, 1.0);\n"
"   texCoords=blockTexCoord;\n"
"}\n"
"";

template<typename _Block>
std::string SimpleRenderer<_Block>::fragmentShader=
"#version 330 core\n"
"\n"
"out vec3 color;\n"
"\n"
"void main()\n"
"{\n"
"	color=vec3(1,0,0);\n"
"}\n"
"";

template<typename _Block>
SimpleRenderer<_Block>::SimpleRenderer(World<_Block> *world):
m_world(world),
m_position(0.0f, 0.0f, 0.0),
m_viewRadius(200.0f)
{

}

template<typename _Block>
SimpleRenderer<_Block>::~SimpleRenderer()
{

}

template<typename _Block>
void SimpleRenderer<_Block>::build()
{
    initGlew();
    
    std::string error;

    if(!m_program.attachLoadAndCompileShaders(vertShader, fragmentShader, error))
    {
        return;
    }

    unsigned int instanceVBO;
    
    glGenBuffers(1, &m_instanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*SimpleCube::vertCoords.size(), SimpleCube::vertCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename _Block>
void SimpleRenderer<_Block>::update()
{

}

template<typename _Block>
void SimpleRenderer<_Block>::updateProjection()
{
    glm::mat4 projection=glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

    glm::mat4 projectionViewMat=m_projection*m_view;
}

template<typename _Block>
void SimpleRenderer<_Block>::updateView()
{
}

template<typename _Block>
void SimpleRenderer<_Block>::draw()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    m_program.use();
//    m_world->getChunkFromWorldPos(m_position);
//    m_world->getChunks()
}

}//namespace voxigen

#endif //_voxigen_simpleRenderer_h_