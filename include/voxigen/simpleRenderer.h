#ifndef _voxigen_simpleRenderer_h_
#define _voxigen_simpleRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/initGlew.h"
#include "voxigen/world.h"
#include "voxigen/SimpleCamera.h"
#include "voxigen/SimpleChunkRenderer.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opengl_util/program.h>

namespace voxigen
{

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleRenderer
/////////////////////////////////////////////////////////////////////////////////////////
//template<typename _Block, int _ChunkSizeX, int _ChunkSizeY, int _ChunkSizeZ>
template<typename _World>
class SimpleRenderer
{
public:
    typedef _World WorldType;
    typedef typename _World::ChunkType ChunkType;
    typedef SimpleChunkRenderer<SimpleRenderer, ChunkType> ChunkRenderType;
//    typedef World<_Block, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> WorldType;
    

    SimpleRenderer(WorldType *world);
    ~SimpleRenderer();
    
    WorldType *getWorld(){ return m_world; }

    void build();
    void update();
    void updateProjection(size_t width, size_t height);
    void updateView();
    void draw();

    void setCamera(SimpleFpsCamera *camera);
    void setViewRadius(float radius);
    void updateChunks();

private:
    static std::string vertShader;
    static std::string fragmentShader;

    static std::string vertOutlineShader;
    static std::string fragmentOutlineShader;
    
    float m_viewRadius;
    SimpleFpsCamera *m_camera;
    glm::vec3 m_lastUpdatePosition;

    bool m_outlineChunks;

    WorldType *m_world;

    std::vector<glm::ivec3> m_chunkIndicies;
    std::vector<ChunkRenderType> m_chunkRenderers;

    opengl_util::Program m_program;
    size_t m_uniformProjectionViewId;
    opengl_util::Program m_outlineProgram;
    size_t m_uniformOutlintProjectionViewId;

    size_t m_lightPositionId;
    size_t m_lighColorId;

    bool m_projectionViewMatUpdated;
    glm::mat4 m_projectionViewMat;
    glm::mat4 m_projectionMat;
    glm::mat4 m_viewMat;

    unsigned int m_instanceSquareVertices;
    unsigned int m_instanceVertices;
    unsigned int m_instanceTexCoords;

//simple test
    unsigned int m_vertexArray;
    unsigned int m_offsetVBO;
};

template<typename _World>
std::string SimpleRenderer<_World>::vertShader=
"#version 330 core\n"
"layout (location = 0) in vec3 blockvertex;\n"
"layout (location = 1) in vec3 blockNormal;\n"
"layout (location = 2) in vec2 blockTexCoord;\n"
"layout (location = 3) in vec4 blockOffset;\n"
"\n"
"out vec3 position;\n"
"out vec3 normal;\n"
"out vec3 texCoords;\n"
"\n"
"uniform mat4 projectionView;\n"
"\n"
"void main()\n"
"{\n"
"//   gl_Position=vec4(blockOffset.xyz+blockvertex, 1.0);\n"
"   position=blockOffset.xyz+blockvertex;\n"
"   normal=blockNormal;\n"
"   texCoords=vec3(blockTexCoord, blockOffset.w);\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"

"}\n"
"";

template<typename _World>
std::string SimpleRenderer<_World>::fragmentShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec3 texCoords;\n"
"out vec3 color;\n"
"\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"\n"
"void main()\n"
"{\n"
"   float value=texCoords.z/10.0f;"
"//   color=vec3(texCoords.x, 0.0, texCoords.y);"
"   // ambient\n"
"   float ambientStrength=0.5;\n"
"   vec3 ambient=ambientStrength * lightColor;\n"
"   \n"
"   // diffuse \n"
"   vec3 lightDir=normalize(lightPos-position); \n"
"   float diff=max(dot(normal, lightDir), 0.0); \n"
"   vec3 diffuse=diff * lightColor; \n"
"   color=(ambient+diffuse)*vec3(value, value, value);\n"
"   "
"//   color=vec3(1.0, 0.0, 0.0);\n"
"}\n"
"";



template<typename _World>
std::string SimpleRenderer<_World>::vertOutlineShader=
"#version 330 core\n"
"layout (location = 0) in vec3 vertex;\n"
"\n"
"uniform mat4 projectionView;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position=projectionView*vec4(vertex, 1.0);\n"
"}\n"
"";

template<typename _World>
std::string SimpleRenderer<_World>::fragmentOutlineShader=
"#version 330 core\n"
"\n"
"out vec3 color;\n"
"\n"
"void main()\n"
"{\n"
"   color=vec3(1.0, 1.0, 0.0);\n"
"}\n"
"";

template<typename _World>
SimpleRenderer<_World>::SimpleRenderer(_World *world):
m_world(world),
m_viewRadius(60.0f),
m_lastUpdatePosition(0.0f, 0.0f, 0.0),
m_projectionViewMatUpdated(true),
m_camera(nullptr),
m_outlineChunks(true)
{
    m_projectionMat=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f);

    m_viewMat=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);
}

template<typename _World>
SimpleRenderer<_World>::~SimpleRenderer()
{

}

template<typename _World>
void SimpleRenderer<_World>::build()
{
//    initGlew();
    
    std::string error;

    if(!m_program.attachLoadAndCompileShaders(vertShader, fragmentShader, error))
    {
        assert(false);
        return;
    }

    m_uniformProjectionViewId=m_program.getUniformId("projectionView");
    m_lightPositionId=m_program.getUniformId("lightPos");
    m_lighColorId=m_program.getUniformId("lightColor");

    m_program.use();
    m_program.uniform(m_lighColorId)=glm::vec3(1.0f, 1.0f, 1.0f);

    if(!m_outlineProgram.attachLoadAndCompileShaders(vertOutlineShader, fragmentOutlineShader, error))
    {
        assert(false);
        return;
    }

    m_uniformOutlintProjectionViewId=m_outlineProgram.getUniformId("projectionView");


////////////////////////////////////////////////////////////////
//Simple test
    float quadVertices[]=
    {
        // positions     // colors
        -0.05f,  0.05f,  0.0f, 0.0f, 0.0f,
        0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

        -0.05f,  0.05f,  0.0f, 0.0f, 0.0f,
        0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
        0.05f,  0.05f,  0.0f, 1.0f, 1.0f

//        -1.0f,  1.0f,  0.0f, 0.0f, 0.0f,
//        1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
//        -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
//
//        -1.0f,  1.0f,  0.0f, 0.0f, 0.0f,
//        1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
//        1.0f,  1.0f,  0.0f, 1.0f, 1.0f
    };

    glm::vec4 translations[100];
    int index=0;
    float offset=0.1f;
    
    for(int y=-10; y < 10; y+=2)
    {   
        for(int x=-10; x < 10; x+=2)   
        {
            glm::vec4 &translation=translations[index];

            translation.x=(float)x/10.0f+offset;
            translation.y=(float)y/10.0f+offset;
            translation.z=(float)-1.0f;
            translation.w=(x+10.0f)/2.0f;
            index++;
        }
    }

    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);

    glGenBuffers(1, &m_instanceSquareVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceSquareVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3));

    glGenBuffers(1, &m_offsetVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*100, &translations[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
    glVertexAttribDivisor(2, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
////////////////////////////////////////////////////////////////

    size_t verticies=SimpleCube::vertCoords.size();

    glGenBuffers(1, &m_instanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*SimpleCube::vertCoords.size(), SimpleCube::vertCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

//    setViewRadius(m_viewRadius);
}

template<typename _World>
void SimpleRenderer<_World>::update()
{
}

template<typename _World>
void SimpleRenderer<_World>::updateProjection(size_t width, size_t height)
{
    m_projectionMat=glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

    m_projectionViewMat=m_projectionMat*m_viewMat;
    m_projectionViewMatUpdated=true;
}

template<typename _World>
void SimpleRenderer<_World>::updateView()
{
}

template<typename _World>
void SimpleRenderer<_World>::draw()
{
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
//
//    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    m_program.use();

//    if(m_projectionViewMatUpdated)
//        m_program.uniform(m_uniformProjectionViewId)=m_projectionViewMat;
    if(m_camera->isDirty())
    {
        m_program.uniform(m_uniformProjectionViewId)=m_camera->getProjectionViewMat();
        m_program.uniform(m_lightPositionId)=m_camera->getPosition();
        m_outlineProgram.uniform(m_uniformOutlintProjectionViewId)=m_camera->getProjectionViewMat();
    }
//    m_world->getChunkFromWorldPos(m_position);
//    m_world->getChunks()

    for(int i=0; i<m_chunkRenderers.size(); ++i)
    {
        m_chunkRenderers[i].draw();
    }

    if(m_outlineChunks)
    {
        m_outlineProgram.use();
        for(int i=0; i<m_chunkRenderers.size(); ++i)
        {
            m_chunkRenderers[i].drawOutline();
        }
    }

////////////////////////////////////////////////////////////////
//Simple test
//    glBindVertexArray(m_vertexArray);
//    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
////////////////////////////////////////////////////////////////
}

template<typename _World>
void SimpleRenderer<_World>::setCamera(SimpleFpsCamera *camera)
{
    m_camera=camera;

//    m_viewMat=glm::lookAt(m_camera.position, m_camera.position+m_camera.direction, m_camera.up);
//    m_projectionViewMat=m_projectionMat*m_viewMat;
//    
//    m_projectionViewMatUpdated=true;

    if(glm::distance(m_camera->getPosition(), m_lastUpdatePosition)>8.0f)
    {
//        updateChunks();
        m_lastUpdatePosition=m_camera->getPosition();
    }
}

template<typename _World>
void SimpleRenderer<_World>::setViewRadius(float radius)
{
    m_viewRadius=radius;

    glm::ivec3 &chunkSize=m_world->getDescriptors().chunkSize;
    glm::ivec3 chunkRadius=glm::ceil(glm::vec3(m_viewRadius/chunkSize.x, m_viewRadius/chunkSize.y, m_viewRadius/chunkSize.z));

    m_chunkIndicies.clear();
    glm::vec3 startPos=(chunkRadius*(-chunkSize))+(chunkSize/2);
    glm::vec3 chunkPos;

    chunkPos.z=startPos.z;
    for(int z=-chunkRadius.z; z<chunkRadius.z; ++z)
    {
        chunkPos.y=startPos.y;
        for(int y=-chunkRadius.y; y<chunkRadius.y; ++y)
        {
            chunkPos.x=startPos.x;
            for(int x=-chunkRadius.x; x<chunkRadius.x; ++x)
            {
                float chunkDistance=glm::length(chunkPos);

                if(chunkDistance<=m_viewRadius)
                {
                    m_chunkIndicies.push_back(glm::ivec3(x, y, z));
                }
                chunkPos.x+=chunkSize.x;
            }
            chunkPos.y+=chunkSize.y;
        }
        chunkPos.z+=chunkSize.z;
    }

    if(m_chunkIndicies.empty()) //always want at least the current chunk
        m_chunkIndicies.push_back(glm::ivec3(0, 0, 0));

//    updateChunks();
}

template<typename _World>
void SimpleRenderer<_World>::updateChunks()
{
    glm::ivec3 chunkIndex=m_world->getChunkIndex(m_camera->getPosition());

    if(m_chunkRenderers.size()!=m_chunkIndicies.size())
    {
        //depending on how often this happens (should only be on viewRadius change), 
        //might want to move any renderers outside the new size to invalid render locations
        //lower in the vector, then resize (otherwise they will get rebuilt)

        bool buildRenderers=(m_chunkRenderers.size()<m_chunkIndicies.size());
        size_t buildIndex=m_chunkRenderers.size();

        m_chunkRenderers.resize(m_chunkIndicies.size());
        if(buildRenderers)
        {
            //need to setup buffers for new chunks
            for(size_t i=buildIndex; i<m_chunkRenderers.size(); ++i)
            {
                m_chunkRenderers[i].setParent(this);
                m_chunkRenderers[i].build(m_instanceVertices);
            }
        }
    }

    int size=m_chunkRenderers.size();

    std::vector<bool> invalidatedRenderers(size);
    std::vector<unsigned int> chunks(size);
    std::vector<bool> inUse(size);
    
    for(size_t i=0; i<size; ++i)
    {
        chunks[i]=m_world->chunkHash(chunkIndex+m_chunkIndicies[i]);
        invalidatedRenderers[i]=true;
        inUse[i]=false;
    }

    for(size_t i=0; i<size; ++i)
    {
        ChunkRenderType &chunkRenderer=m_chunkRenderers[i];

        if(chunkRenderer.getState()==ChunkRenderType::Invalid)
            continue;

        unsigned int chunkHash=chunkRenderer.getHash();

        for(size_t j=0; j<size; ++j)
        {
            if(chunks[j]==chunkHash)
            {
                invalidatedRenderers[i]=false;
                inUse[j]=true;
                break;
            }
        }
    }
    
    for(size_t i=0; i<size; ++i)
    {
        ChunkRenderType &chunkRenderer=m_chunkRenderers[i];

        if(chunkRenderer.getState()==ChunkRenderType::Invalid)
        {
            invalidatedRenderers[i]=true;
            continue;
        }
        
        if(invalidatedRenderers[i])
            chunkRenderer.invalidate();
    }

    size_t invalidatedIndex=0;

    for(size_t i=0; i<size; ++i)
    {
        if(inUse[i])
            continue;

        //find invalid renderer and update chuck
        for(size_t j=invalidatedIndex; j<size; ++j)
        {
            if(!invalidatedRenderers[j])
                continue;

            ChunkRenderType &chunkRenderer=m_chunkRenderers[j];

            ChunkType *chunk=&m_world->getChunk(chunks[i]);

            chunkRenderer.setChunk(chunk);
            chunkRenderer.update();

            invalidatedIndex=j+1;
            break;
        }
    }
}

}//namespace voxigen

#endif //_voxigen_simpleRenderer_h_