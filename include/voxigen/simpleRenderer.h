#ifndef _voxigen_simpleRenderer_h_
#define _voxigen_simpleRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/initGlew.h"
#include "voxigen/world.h"
#include "voxigen/SimpleCamera.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opengl_util/program.h>

namespace voxigen
{

template<typename _Block>
class SimpleRenderer;

struct VOXIGEN_EXPORT SimpleCube
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
    SimpleChunkRenderer():m_state(Invalid){}
    ~SimpleChunkRenderer() {}
    
    enum State
    {
        Invalid,
        Init,
        Dirty,
        Built,
        Empty
    };

    State getState() { return m_state; }

    void setParent(SimpleRenderer<_Block> *parent);
    void setChunk(Chunk<_Block> *chunk);
    void build(unsigned int instanceData);
    void update();
    void invalidate();

    void draw();

    const unsigned int getHash() { return m_chunk->getHash(); }
    const glm::ivec3 &getPosition() { return m_chunk->getPosition(); }
    
private:
    SimpleRenderer<_Block> *m_parent;

    State m_state;
    Chunk<_Block> *m_chunk;
    bool m_empty;

    unsigned int m_validBlocks;
    unsigned int m_vertexArray;
    unsigned int m_offsetVBO;
    std::vector<glm::vec4> m_blockOffset;
};

template<typename _Block>
void SimpleChunkRenderer<_Block>::setParent(SimpleRenderer<_Block> *parent)
{
    m_parent=parent;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::setChunk(Chunk<_Block> *chunk)
{
    m_chunk=chunk;
    m_state=Dirty;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::build(unsigned int instanceData)
{
    if(m_state != Init)
        return;

    glGenVertexArrays(1, &m_vertexArray);

    glGenBuffers(1, &m_offsetVBO);
    update();

    glBindVertexArray(m_vertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceData);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(1, 1);
    m_state=Dirty;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::update()
{
    if(m_state!=Dirty)
        return;

    auto &blocks=m_chunk->getBlocks();
    glm::ivec3 &chunkSize=m_parent->getWorld()->getDescriptors().chunkSize;
    std::vector<glm::vec4> translations(chunkSize.x*chunkSize.y*chunkSize.z);
    glm::ivec3 position=m_chunk->getPosition();
    glm::ivec3 pos=position;

    int index=0;
    
    for(int z=0; z<chunkSize.z; ++z)
    {
        pos.y=position.y;
        for(int y=0; y<chunkSize.y; ++y)
        {
            pos.x=position.x;
            for(int x=0; x<chunkSize.x; ++x)
            {
                unsigned int type=blocks[index].type;

                if(type>0)
                {
                    translations[index]=glm::vec4(pos, type);
                    index++;
                }
                pos.x++;
            }
            pos.y++;
        }
        pos.z++;
    }

    m_validBlocks=index;
    if(index==0)
    {
        m_state=Empty;
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*index, translations.data(), GL_STATIC_DRAW);
    m_state=Built;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::invalidate()
{
    m_state=Invalid;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::draw()
{
    if(m_state!=Built)
        return;

    glBindVertexArray(m_vertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 12, m_validBlocks);


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
    
    World<_Block> *getWorld(){ return m_world; }

    void build();
    void update();
    void updateProjection(size_t width, size_t height);
    void updateView();
    void draw();

    void setCamera(SimpleCamera &camera);
    void setViewRadius(float radius);
    void updateChunks();

private:
    static std::string vertShader;
    static std::string fragmentShader;
    
    float m_viewRadius;
    SimpleCamera m_camera;
    glm::vec3 m_lastUpdatePosition;

    World<_Block> *m_world;

    std::vector<glm::ivec3> m_chunkIndicies;
    std::vector<SimpleChunkRenderer<_Block>> m_chunkRenderers;

    opengl_util::Program m_program;
    size_t m_uniformProjectionViewId;

    glm::mat4 m_projectionMat;
    glm::mat4 m_viewMat;

    unsigned int m_instanceVertices;
    unsigned int m_instanceTexCoords;

//simple test
    unsigned int m_vertexArray;
    unsigned int m_offsetVBO;
};

template<typename _Block>
std::string SimpleRenderer<_Block>::vertShader=
"#version 330 core\n"
"layout (location = 0) in vec3 blockvertex;\n"
"layout (location = 1) in vec2 blockTexCoord;\n"
"layout (location = 2) in vec4 blockOffset;\n"
"\n"
"out vec3 texCoords;\n"
"\n"
"uniform mat4 projectionView;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position=vec4(blockOffset.xyz+blockvertex, 1.0);\n"
"//   gl_Position=projectionView*vec4(blockOffset.xyz+blockvertex, 1.0);\n"
"   texCoords=vec3(blockTexCoord, blockOffset.w);\n"
"}\n"
"";

template<typename _Block>
std::string SimpleRenderer<_Block>::fragmentShader=
"#version 330 core\n"
"\n"
"in vec3 texCoords;\n"
"out vec3 color;\n"
"\n"
"void main()\n"
"{\n"
"   float value=texCoords.z/10.0f;"
"//   color=vec3(value, value, value);\n"
"   color=vec3(1.0, 0.0, 0.0);\n"
"}\n"
"";

template<typename _Block>
SimpleRenderer<_Block>::SimpleRenderer(World<_Block> *world):
m_world(world),
m_viewRadius(60.0f),
m_lastUpdatePosition(0.0f, 0.0f, 0.0)
{
}

template<typename _Block>
SimpleRenderer<_Block>::~SimpleRenderer()
{

}

template<typename _Block>
void SimpleRenderer<_Block>::build()
{
//    initGlew();
    
    std::string error;

    if(!m_program.attachLoadAndCompileShaders(vertShader, fragmentShader, error))
    {
        assert(false);
        return;
    }

//    m_uniformProjectionViewId=m_program.getUniformId("projectionView");

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
            translation.z=(float)0.0f;
            translation.w=(x+10.0f)/2.0f;
            index++;
        }
    }

    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);

    glGenBuffers(1, &m_instanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVertices);
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

//    glGenBuffers(1, &m_instanceVertices);
//    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVertices);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*SimpleCube::vertCoords.size(), SimpleCube::vertCoords.data(), GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);

//    setViewRadius(m_viewRadius);
}

template<typename _Block>
void SimpleRenderer<_Block>::update()
{
}

template<typename _Block>
void SimpleRenderer<_Block>::updateProjection(size_t width, size_t height)
{
    m_projectionMat=glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

    glm::mat4 projectionViewMat=m_projectionMat*m_viewMat;
    m_program.uniform(m_uniformProjectionViewId)=projectionViewMat;
}

template<typename _Block>
void SimpleRenderer<_Block>::updateView()
{
}

template<typename _Block>
void SimpleRenderer<_Block>::draw()
{
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
//
//    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    m_program.use();
//    m_world->getChunkFromWorldPos(m_position);
//    m_world->getChunks()
//    for(int i=0; i<m_chunkRenderers.size(); ++i)
//    {
//        m_chunkRenderers[i].draw();
//    }

////////////////////////////////////////////////////////////////
//Simple test
    glBindVertexArray(m_vertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
////////////////////////////////////////////////////////////////
}

template<typename _Block>
void SimpleRenderer<_Block>::setCamera(SimpleCamera &camera)
{
    m_camera=camera;

    m_viewMat=glm::lookAt(m_camera.position, m_camera.forward, m_camera.up);
    glm::mat4 projectionViewMat=m_projectionMat*m_viewMat;
    m_program.uniform(m_uniformProjectionViewId)=projectionViewMat;

    if(glm::distance(m_camera.position, m_lastUpdatePosition)>8.0f)
    {
        updateChunks();
        m_lastUpdatePosition=m_camera.position;
    }
}

template<typename _Block>
void SimpleRenderer<_Block>::setViewRadius(float radius)
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
    updateChunks();
}

template<typename _Block>
void SimpleRenderer<_Block>::updateChunks()
{
    glm::ivec3 chunkIndex=m_world->getChunkIndex(m_camera.position);

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
        SimpleChunkRenderer<_Block> &chunkRenderer=m_chunkRenderers[i];

        if(chunkRenderer.getState()==SimpleChunkRenderer<_Block>::Invalid)
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
        SimpleChunkRenderer<_Block> &chunkRenderer=m_chunkRenderers[i];

        if(chunkRenderer.getState()==SimpleChunkRenderer<_Block>::Invalid)
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

            SimpleChunkRenderer<_Block> &chunkRenderer=m_chunkRenderers[j];

            Chunk<_Block> *chunk=&m_world->getChunk(chunks[i]);

            chunkRenderer.setChunk(chunk);
            chunkRenderer.update();

            invalidatedIndex=j+1;
            break;
        }
    }
}

}//namespace voxigen

#endif //_voxigen_simpleRenderer_h_