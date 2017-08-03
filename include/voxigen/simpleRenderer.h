#ifndef _voxigen_simpleRenderer_h_
#define _voxigen_simpleRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/initGlew.h"
#include "voxigen/regularGrid.h"
#include "voxigen/SimpleCamera.h"
#include "voxigen/SimpleChunkRenderer.h"
#include "voxigen/simpleShapes.h"

#include <string>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opengl_util/program.h>

namespace voxigen
{

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Grid>
class SimpleRenderer
{
public:
    typedef _Grid GridType;
    typedef typename GridType::ChunkType ChunkType;
    typedef typename _Grid::SharedChunkHandle SharedChunkHandle;
    typedef SimpleChunkRenderer<SimpleRenderer, typename _Grid::ChunkType> ChunkRenderType;
    typedef std::unordered_map<ChunkHash, size_t> ChunkRendererMap;
    typedef std::unordered_map<SegmentHash, ChunkRendererMap> SegmentRendererMap;
    
    SimpleRenderer(GridType *grid);
    ~SimpleRenderer();
    
    GridType *getGrid(){ return m_grid; }

    void build();
    void destroy();
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

    GridType *m_grid;

    std::vector<glm::ivec3> m_chunkIndices;
    std::vector<ChunkRenderType> m_chunkRenderers;
    SegmentRendererMap m_rendererMap;
    std::vector<SegmentChunkHash> m_chunksUpdated;

    opengl_util::Program m_program;
    size_t m_uniformProjectionViewId;
    opengl_util::Program m_outlineProgram;
    size_t m_uniformOutlintProjectionViewId;
    size_t m_outlineLightPositionId;

    size_t m_lightPositionId;
    size_t m_lighColorId;

    bool m_projectionViewMatUpdated;
    glm::mat4 m_projectionViewMat;
    glm::mat4 m_projectionMat;
    glm::mat4 m_viewMat;

    unsigned int m_instanceSquareVertices;
    unsigned int m_instanceVertices;
    unsigned int m_instanceTexCoords;

#ifndef NDEBUG
    unsigned int m_outlineInstanceVertices;
#endif //NDEBUG
};

template<typename _Grid>
std::string SimpleRenderer<_Grid>::vertShader=
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

template<typename _Grid>
std::string SimpleRenderer<_Grid>::fragmentShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec3 texCoords;\n"
"out vec4 color;\n"
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
"   color=vec4((ambient+diffuse)*vec3(value, value, value), 1.0f);\n"
"   "
"//   color=vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n"
"";



template<typename _Grid>
std::string SimpleRenderer<_Grid>::vertOutlineShader=
"#version 330 core\n"
"layout (location = 0) in vec3 inputVertex;\n"
"layout (location = 1) in vec3 inputNormal;\n"
"layout (location = 2) in vec2 inputTexCoord;\n"
"layout (location = 3) in vec4 inputOffset;\n"
"\n"
"out vec3 position;\n"
"out vec3 normal;\n"
"out vec3 texCoords;\n"
"\n"
"uniform mat4 projectionView;\n"
"\n"
"void main()\n"
"{\n"
"   position=inputOffset.xyz+inputVertex;\n"
"   normal=inputNormal;\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"
"}\n"
"";

template<typename _Grid>
std::string SimpleRenderer<_Grid>::fragmentOutlineShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec3 texCoords;\n"
"out vec4 color;\n"
"\n"
"uniform vec3 lightPos;\n"
"\n"
"void main()\n"
"{\n"
"   float value=1.0f;"
"//   vec3 lightColor=vec3(1.0f, 1.0f, 1.0f);\n"
"   float ambientStrength=0.5; \n"
"//   vec3 ambient=ambientStrength * lightColor;\n"
"   \n"
"   // diffuse \n"
"   vec3 lightDir=normalize(lightPos-position); \n"
"   float diff=max(dot(normal, lightDir), 0.0); \n"
"//   vec3 diffuse=diff*lightColor; \n"
"   color=vec4((ambientStrength+diff)*value, 0.0f, 0.0f, 0.1f);\n"
"}\n"
"";

template<typename _Grid>
SimpleRenderer<_Grid>::SimpleRenderer(_Grid *grid):
m_grid(grid),
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

template<typename _Grid>
SimpleRenderer<_Grid>::~SimpleRenderer()
{

}

template<typename _Grid>
void SimpleRenderer<_Grid>::build()
{
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

    const std::vector<float> &vertices=SimpleCube<1, 1, 1>::vertCoords;

    glGenBuffers(1, &m_instanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifndef NDEBUG
    m_uniformOutlintProjectionViewId=m_outlineProgram.getUniformId("projectionView");
    m_outlineLightPositionId=m_outlineProgram.getUniformId("lightPos");

    const std::vector<float> &outlineVertices=SimpleCube<ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value>::vertCoords;

    glGenBuffers(1, &m_outlineInstanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineInstanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif //NDEBUG
}

template<typename _Grid>
void SimpleRenderer<_Grid>::destroy()
{
    m_rendererMap.clear();
    m_chunkRenderers.clear();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::update()
{
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updateProjection(size_t width, size_t height)
{
    m_projectionMat=glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

    m_projectionViewMat=m_projectionMat*m_viewMat;
    m_projectionViewMatUpdated=true;
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updateView()
{
}

template<typename _Grid>
void SimpleRenderer<_Grid>::draw()
{
    m_program.use();
    bool cameraDirty=m_camera->isDirty();

    if(cameraDirty)
    {
        m_program.uniform(m_uniformProjectionViewId)=m_camera->getProjectionViewMat();
        m_program.uniform(m_lightPositionId)=m_camera->getPosition();
    }
    if(m_chunksUpdated.empty())
        m_chunksUpdated=m_grid->getUpdatedChunks();

    //update any chunks that may have changed
    if(!m_chunksUpdated.empty())
    {
        //only update at max 10 per frame, better to be done in thread
        size_t maxUpdates=std::min((size_t)10, m_chunksUpdated.size());

        for(size_t i=0; i<maxUpdates; ++i)
        {
            SegmentChunkHash &hash=m_chunksUpdated[i];

            auto iter=std::find_if(m_chunkRenderers.begin(), m_chunkRenderers.end(), [&hash](auto &renderer){return ((renderer.getSegmentHash()==hash.segmentHash)&&(renderer.getChunkHash()==hash.chunkHash)); });

            if(iter!=m_chunkRenderers.end())
                iter->update();
        }

        m_chunksUpdated.erase(m_chunksUpdated.begin(), m_chunksUpdated.begin()+maxUpdates);
    }

    //draw all chunks
    for(int i=0; i<m_chunkRenderers.size(); ++i)
    {
        m_chunkRenderers[i].draw();
    }

#ifndef NDEBUG
    //draw Missing blocks in debug
//    if(m_outlineChunks)
    {
        m_outlineProgram.use();

        if(cameraDirty)
        {
            m_outlineProgram.uniform(m_uniformOutlintProjectionViewId)=m_camera->getProjectionViewMat();
            m_outlineProgram.uniform(m_outlineLightPositionId)=m_camera->getPosition();
        }
        
        for(int i=0; i<m_chunkRenderers.size(); ++i)
        {
            m_chunkRenderers[i].drawOutline();
        }
    }
#endif //NDEBUG
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setCamera(SimpleFpsCamera *camera)
{
    m_camera=camera;
    if(glm::distance(m_camera->getPosition(), m_lastUpdatePosition)>8.0f)
        m_lastUpdatePosition=m_camera->getPosition();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setViewRadius(float radius)
{
    m_viewRadius=radius;

    glm::ivec3 &chunkSize=m_grid->getDescriptors().m_chunkSize;
    glm::ivec3 chunkRadius=glm::ceil(glm::vec3(m_viewRadius/chunkSize.x, m_viewRadius/chunkSize.y, m_viewRadius/chunkSize.z));

    m_chunkIndices.clear();
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
                    m_chunkIndices.push_back(glm::ivec3(x, y, z));
                }
                chunkPos.x+=chunkSize.x;
            }
            chunkPos.y+=chunkSize.y;
        }
        chunkPos.z+=chunkSize.z;
    }

    if(m_chunkIndices.empty()) //always want at least the current chunk
        m_chunkIndices.push_back(glm::ivec3(0, 0, 0));
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updateChunks()
{
    glm::ivec3 segmentIndex=m_grid->getSegmentIndex(m_camera->getSegmentHash());
    glm::ivec3 chunkIndex=m_grid->getChunkIndex(m_camera->getPosition());

    if(m_chunkRenderers.size()!=m_chunkIndices.size())
    {
        //depending on how often this happens (should only be on viewRadius change), 
        //might want to move any renderers outside the new size to invalid render locations
        //lower in the vector, then resize (otherwise they will get rebuilt)

        bool buildRenderers=(m_chunkRenderers.size()<m_chunkIndices.size());
        size_t buildIndex=m_chunkRenderers.size();

        m_chunkRenderers.resize(m_chunkIndices.size());
        if(buildRenderers)
        {
            //need to setup buffers for new chunks
            for(size_t i=buildIndex; i<m_chunkRenderers.size(); ++i)
            {
                m_chunkRenderers[i].setParent(this);
                m_chunkRenderers[i].build(m_instanceVertices);
#ifndef NDEBUG
                m_chunkRenderers[i].buildOutline(m_outlineInstanceVertices);
#endif //NDEBUG
            }
        }
    }

    int size=m_chunkRenderers.size();
    size_t mapSize=m_rendererMap.size();

    std::vector<bool> invalidatedRenderers(size);
//    std::vector<unsigned int> chunks(size);
    std::vector<SegmentChunkHash> chunks(size);
    std::vector<glm::vec3> chunkOffset(size);

    std::vector<bool> inUse(size);
    glm::ivec3 index;
    glm::ivec3 currentSegmentIndex;

    for(size_t i=0; i<size; ++i)
    {
//        chunks[i]=m_grid->chunkHash(chunkIndex+m_chunkIndices[i]);
        index=chunkIndex+m_chunkIndices[i];
        currentSegmentIndex=segmentIndex;

        chunkOffset[i]=m_grid->getDescriptors().adjustSegment(currentSegmentIndex, index);
        chunks[i]=m_grid->getHashes(currentSegmentIndex, index);

        invalidatedRenderers[i]=true;
        inUse[i]=false;
    }

    for(size_t i=0; i<size; ++i)
    {
        auto &iter=m_rendererMap.find(chunks[i].segmentHash);

        if(iter!=m_rendererMap.end())
        {
            auto &chunkIter=iter->second.find(chunks[i].chunkHash);

            if(chunkIter!=iter->second.end())
            {
                invalidatedRenderers[chunkIter->second]=false;
                inUse[i]=true;

                //update if offset is different
                if(m_chunkRenderers[chunkIter->second].getChunkOffset()!=chunkOffset[i])
                    m_chunkRenderers[chunkIter->second].setChunkOffset(chunkOffset[i]);
            }
        }
    }

    //invalidate renderers not in use
    for(size_t i=0; i<size; ++i)
    {
        ChunkRenderType &chunkRenderer=m_chunkRenderers[i];

        if(chunkRenderer.getState()==ChunkRenderType::Invalid)
        {
            invalidatedRenderers[i]=true;
            continue;
        }

        if(invalidatedRenderers[i])
        {
            auto &iter=m_rendererMap.find(chunkRenderer.getSegmentHash());

            if(iter!=m_rendererMap.end())
            {
                auto &chunkIter=iter->second.find(chunkRenderer.getChunkHash());

                if(chunkIter!=iter->second.end())
                    iter->second.erase(chunkIter);

                if(iter->second.empty())
                    m_rendererMap.erase(iter);

            }

            chunkRenderer.invalidate();
        }
    }

    size_t invalidatedIndex=0;

    //add missing chunks
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

            auto &iter=m_rendererMap.find(chunks[i].segmentHash);

            ChunkRendererMap *chunkRenderMap;

            if(iter==m_rendererMap.end())
            {
                m_rendererMap.insert(SegmentRendererMap::value_type(chunks[i].segmentHash, ChunkRendererMap()));
                chunkRenderMap=&m_rendererMap[chunks[i].segmentHash];
            }
            else
                chunkRenderMap=&iter->second;

            chunkRenderMap->insert(ChunkRendererMap::value_type(chunks[i].chunkHash, j));

            SharedChunkHandle chunkHandle=m_grid->getChunk(chunks[i].segmentHash, chunks[i].chunkHash);

            chunkRenderer.setSegmentHash(chunks[i].segmentHash);
            chunkRenderer.setChunk(chunkHandle);
            chunkRenderer.setChunkOffset(chunkOffset[i]);
            chunkRenderer.update();

            invalidatedIndex=j+1;
            break;
        }
    }
}

}//namespace voxigen

#endif //_voxigen_simpleRenderer_h_