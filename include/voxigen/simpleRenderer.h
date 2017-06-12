#ifndef _voxigen_simpleRenderer_h_
#define _voxigen_simpleRenderer_h_

#include "voxigen/voxigen_export.h"
#include <string>

namespace voxigen
{

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleChunkRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Block>
class SimpleChunkRenderer
{
public:
    SimpleChunkRenderer(Chunk<_Block> *chunk) {}
    ~SimpleChunkRenderer() {}
    
    void build(unsigned int instanceVerticies, unsigned int instanceTexCoords);
    void update();
private:
    Chunk<_Block> *chunk;

    unsigned int m_vertextArray;

    unsigned int m_offsetVBO;
    std::vector<glm::vec3> m_blockOffset;
};

template<typename _Block>
void SimpleChunkRenderer<_Block>build(unsigned int instanceVerticies, unsigned int instanceTexCoords)
{
    glGenVertexArrays(1, &m_vertextArray);

    glGenBuffers(1, &m_offsetVBO);
    update();


    glBindVertexArray(m_vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVerticies);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceTexCoords);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<typename _Block>
void SimpleChunkRenderer<_Block>update()
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
    void draw();

private:
    static std::string vertShader;
    glm::vec3 m_position;
    float m_viewRadius;

    World<_Block> *m_world;

    std::vector<SimpleChunkRenderer> m_chunkRenders;

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
m_viewRadius(200.0f);
{

}

template<typename _Block>
SimpleRenderer<_Block>::~SimpleRenderer()
{

}

template<typename _Block>
void SimpleRenderer<_Block>::build()
{

    unsigned int instanceVBO;
    
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*100, &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename _Block>
void SimpleRenderer<_Block>::updateProjection()
{
    glm::mat4 projection=glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

    glm::mat4 projectionViewMat=m_projection*m_view;
}

template<typename _Block>
void SimpleRenderer<_Block>::updateProjection()
{
}

template<typename _Block>
void SimpleRenderer<_Block>::draw()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    m_world->getChunkFromWorldPos(m_position);


    m_world->getChunks()
}

}//namespace voxigen

#endif //_voxigen_simpleRenderer_h_