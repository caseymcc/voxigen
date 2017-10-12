#include "voxigen/chunkFunctions.h"

namespace voxigen
{

template<typename _Grid>
std::string SimpleRenderer<_Grid>::vertShader=
"#version 330 core\n"
"//layout (location = 0) in vec3 blockvertex;\n"
"//layout (location = 1) in vec3 blockNormal;\n"
"//layout (location = 2) in vec2 blockTexCoord;\n"
"//layout (location = 3) in vec4 blockOffset;\n"
"layout (location = 0) in uvec3 packedPosition;\n"
"layout (location = 1) in uint data;\n"
"\n"
"out vec3 position;\n"
"//out vec3 normal;\n"
"out vec3 texCoords;\n"
"\n"
"//layout (std140) uniform pos\n"
"//{\n"
"//   vec4 cameraPos;\n"
"//   vec4 lightPos;\n"
"//   vec4 lightColor;\n"
"//}\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"//   gl_Position=vec4(blockOffset.xyz+blockvertex, 1.0);\n"
"//   position=regionOffset+blockOffset.xyz+blockvertex;\n"
"   vec3 decodedPosition=packedPosition;\n"
"   decodedPosition=decodedPosition;\n"
"   position=regionOffset+decodedPosition;\n"
"//   normal=blockNormal;\n"
"//   texCoords=vec3(blockTexCoord, blockOffset.w);\n"
"   texCoords=vec3(0.0, 0.0, data);\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"

"}\n"
"";

template<typename _Grid>
std::string SimpleRenderer<_Grid>::fragmentShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"//in vec3 normal;\n"
"in vec3 texCoords;\n"
"out vec4 color;\n"
"\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"\n"
"void main()\n"
"{\n"
"   vec3 normal = cross(dFdy(position), dFdx(position));\n"
"   normal=normalize(normal);\n"
""
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
"//   color=vec4((ambient+diffuse)*vec3(value, value, value), 1.0f);\n"
"   color=vec4(abs(normal), 1.0f);\n"
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
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"//   position=inputOffset.xyz+inputVertex;\n"
"   position=regionOffset+inputOffset.xyz+inputVertex;\n"
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
m_outlineChunks(true),
m_queryComplete(true)
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
//    m_lightPositionId=m_program.getUniformId("lightPos");
//    m_lighColorId=m_program.getUniformId("lightColor");
    m_offsetId=m_program.getUniformId("regionOffset");

    m_program.use();
//    m_program.uniform(m_lighColorId)=glm::vec3(1.0f, 1.0f, 1.0f);

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

    m_uniformOutlintProjectionViewId=m_outlineProgram.getUniformId("projectionView");
    m_outlineLightPositionId=m_outlineProgram.getUniformId("lightPos");
    m_outlineOffsetId=m_outlineProgram.getUniformId("regionOffset");

    const std::vector<float> &outlineVertices=SimpleCube<ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value>::vertCoords;

    glGenBuffers(1, &m_outlineInstanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineInstanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef _WINDOWS
    m_prepDC=wglGetCurrentDC();
    HGLRC currentContext=wglGetCurrentContext();

    m_prepGlContext=wglCreateContext(m_prepDC);
    
    BOOL success=wglShareLists(currentContext, m_prepGlContext);

    assert(success);
#else
    assert(false); //need os specific calls to create context for prepThread
#endif
    startPrepThread();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::destroy()
{
    stopPrepThread();
//    m_rendererMap.clear();
    m_regionRenderers.clear();
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
//        m_program.uniform(m_lightPositionId)=m_camera->getPosition();
    }

    std::vector<Key> updatedChunks=m_grid->getUpdatedChunks();

    if(!updatedChunks.empty())
    {
        for(size_t i=0; i<updatedChunks.size(); ++i)
        {
            Key &hash=updatedChunks[i];

            auto regionIter=m_regionRenderers.find(hash.regionHash);

            if(regionIter != m_regionRenderers.end())
            {
                ChunkRendererMap &chunkMap=regionIter->second.chunkRenderers;

                auto chunkIter=chunkMap.find(hash.chunkHash);

                if(chunkIter!=chunkMap.end())
                {
                    chunkIter->second->refCount=1; //make sure we hold onto it while it is in the prepThread
                    chunkIter->second->updateOutline();

                    if(chunkIter->second->getChunkHandle()->empty)
                        chunkIter->second->setEmpty();
                    else
                    {
                        m_outstandingChunkPreps++;
                        //give to prep thread to get it ready
                        addPrepQueue(chunkIter->second);
                    }
                }
            }

            m_outstandingChunkLoads--;
//            assert(m_outstandingChunkLoads>=0);
        }
    }
    
    if(!m_chunksUpdated.empty())
    {
        std::unique_lock<std::mutex> lock(m_prepMutex);

        for(size_t i=0; i<m_chunksUpdated.size(); ++i)
        {
//            m_chunksUpdated[i]->refCount=0; //done in the prepThread
            m_chunksUpdated[i]->updated();
            m_chunksUpdated[i]->getChunkHandle();//drop chunk as we have a mesh for it
        }

        m_chunksUpdated.clear();
    }

//    if(m_chunksUpdated.empty())
//        m_chunksUpdated=m_grid->getUpdatedChunks();
//
//    //update any chunks that may have changed
//    if(!m_chunksUpdated.empty())
//    {
//        //only update at max 10 per frame, better to be done in thread
//        size_t maxUpdates=std::min((size_t)10, m_chunksUpdated.size());
//
//        for(size_t i=0; i<maxUpdates; ++i)
//        {
//            Key &hash=m_chunksUpdated[i];
//
//            auto regionIter=m_regionRenderers.find(hash.regionHash);
//
//            if(regionIter != m_regionRenderers.end())
//            {
//                ChunkRendererMap &chunkMap=regionIter->second.chunkRenderers;
//
//                auto chunkIter=chunkMap.find(hash.chunkHash);
//
//                if(chunkIter!=chunkMap.end())
//                {
//                    //give to prep thread to get it ready
//                    addPrepQueue(chunkIter->second);
////                    chunkIter->second->update();
//                }
//            }
//        }
//
//        m_chunksUpdated.erase(m_chunksUpdated.begin(), m_chunksUpdated.begin()+maxUpdates);
//    }
    updateOcclusionQueries();

    //draw all chunks that are built, using regions
    for(auto &iter:m_regionRenderers)
    {
        RegionRenderer<ChunkRenderType> &renderer=iter.second;

//        m_program.uniform(m_offsetId)=renderer.offset;

        for(auto &chunkIter:renderer.chunkRenderers)
        {
            ChunkRenderType *chunkRenderer=chunkIter.second;
            
            if(chunkRenderer->getState()==ChunkRenderType::Built) //only set uniform if we are going to draw, could be copying
                m_program.uniform(m_offsetId)=(renderer.offset+chunkRenderer->getGridOffset());

            if(chunkRenderer->getState()==ChunkRenderType::Copy)
            {
                if(chunkRenderer->incrementCopy())
                    m_outstandingChunkPreps--;
            }
            chunkRenderer->draw();
        }
    }

    //turn off writing color buffer and depth buffer, just checking if it would render
    glColorMask(false, false, false, false);
    glDepthMask(GL_FALSE);
    //draw all occulution queries, using regions
    for(auto &iter:m_regionRenderers)
    {
        RegionRenderer<ChunkRenderType> &renderer=iter.second;

        //        m_program.uniform(m_offsetId)=renderer.offset;
        size_t ongoingQueries=0;

        for(auto &chunkIter:renderer.chunkRenderers)
        {
            ChunkRenderType *chunkRenderer=chunkIter.second;

            if(chunkRenderer->getState()==ChunkRenderType::Query)
            {
                m_program.uniform(m_offsetId)=(renderer.offset+chunkRenderer->getGridOffset());

                chunkRenderer->drawOcculsionQuery();
                ongoingQueries++;
            }
            else if(chunkRenderer->getState()==ChunkRenderType::QueryWait)
            {
                unsigned int samples;

                if(chunkRenderer->checkOcculsionQuery(samples))
                {
                    if(samples>0)
                    {
                        m_outstandingChunkLoads++;
                        m_grid->loadChunk(chunkRenderer->getChunkHandle(), 0);
                    }
                }
                else
                    ongoingQueries++;
                    
            }
        }

        if(!m_queryComplete&&(ongoingQueries<=0))
        {
            if((m_outstandingChunkLoads<=0) && (m_outstandingChunkPreps<=0)) //all loads/preps from previous query complete, can start next ring
                m_queryComplete=true;
        }
    }
    //done querying, turn color and depth buffer back on
    glColorMask(true, true, true, true);
    glDepthMask(GL_TRUE);


//#ifndef NDEBUG
    //draw Missing blocks in debug
//    if(m_outlineChunks)
    {
        m_outlineProgram.use();

        if(cameraDirty)
        {
            m_outlineProgram.uniform(m_uniformOutlintProjectionViewId)=m_camera->getProjectionViewMat();
            m_outlineProgram.uniform(m_outlineLightPositionId)=m_camera->getPosition();
        }
        
        for(auto &iter:m_regionRenderers)
        {
            RegionRenderer<ChunkRenderType> &renderer=iter.second;

            m_outlineProgram.uniform(m_outlineOffsetId)=renderer.offset;

            for(auto &chunkIter:renderer.chunkRenderers)
            {
                ChunkRenderType *chunkRenderer=chunkIter.second;
                
                chunkRenderer->drawOutline();
            }
        }
    }
//#endif //NDEBUG
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
    m_viewRadiusMax=radius*1.5f;
    m_maxChunkRing=std::ceil(radius/std::max(std::max(ChunkType::sizeX::value, ChunkType::sizeY::value), ChunkType::sizeZ::value));

    m_chunkIndices.resize(m_maxChunkRing);

    for(size_t i=0; i<m_maxChunkRing; ++i)
        ringCube<ChunkType>(m_chunkIndices[i], i);

//    if(m_chunkIndices.empty()) //always want at least the current chunk
//        m_chunkIndices.push_back(glm::ivec3(0, 0, 0));
}

struct ChunkQueryOffset
{
    ChunkQueryOffset(size_t queryRing, glm::vec3 &offset):queryRing(queryRing), offset(offset){}

    size_t queryRing;
    glm::vec3 offset;
};

template<typename _Grid>
void SimpleRenderer<_Grid>::updateChunks()
{
    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());

    int chunkIndicesSize=m_chunkIndices.size();

//    std::unordered_map<RegionHash, glm::vec3> regions;
    std::unordered_map<Key::Type, ChunkQueryOffset> chunks;
    glm::ivec3 index;
    glm::ivec3 currentRegionIndex;

    for(size_t i=0; i<m_maxChunkRing; ++i)
    {
        std::vector<glm::ivec3> &chunkIndices=m_chunkIndices[i];

        for(size_t j=0; j<chunkIndices.size(); ++j)
        {
            index=playerChunkIndex+chunkIndices[j];
            currentRegionIndex=playerRegionIndex;

            glm::vec3 regionOffset=m_grid->getDescriptors().adjustRegion(currentRegionIndex, index);
            Key key=m_grid->getHashes(currentRegionIndex, index);

            chunks.insert(std::pair<Key::Type, ChunkQueryOffset>(key.hash, ChunkQueryOffset(i, regionOffset)));
        }
    }

    m_chunkQueryOrder.resize(m_maxChunkRing);

    //invalidate renderers that pass outside max range
    for(auto regionIter=m_regionRenderers.begin(); regionIter!=m_regionRenderers.end(); )
    {
        RegionRendererType &regionRenderer=regionIter->second;
        glm::ivec3 regionOffset=regionRenderer.index-playerRegionIndex;

        regionRenderer.offset=regionOffset*m_grid->getDescriptors().m_regionCellSize;

        auto &chunkRendererMap=regionRenderer.chunkRenderers;

        for(auto chunkIter=chunkRendererMap.begin(); chunkIter!=chunkRendererMap.end(); )
        {
            auto *chunkRenderer=chunkIter->second;
            Key key(chunkRenderer->getRegionHash(), chunkRenderer->getChunkHash());

            glm::ivec3 regionIndex=m_grid->getDescriptors().regionIndex(key.regionHash);
            glm::ivec3 chunkIndex=m_grid->getDescriptors().chunkIndex(key.chunkHash);
            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, regionIndex, chunkIndex);

            //chunk outside of range so invalidate
            if(chunkDistance > m_viewRadiusMax)
            {
                if(chunkRenderer->refCount == 0) //need to keep if in prepThread
                {
                    chunkRenderer->invalidate();
                    chunkIter=chunkRendererMap.erase(chunkIter);
                    m_freeChunkRenderers.push_back(chunkRenderer);
                    continue;
                }
            }
            else
            {
                auto iter=chunks.find(key.hash);

                if(iter!=chunks.end())
                {
                    m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);
                    chunks.erase(iter);
                }
            }
            ++chunkIter;
        }

        if(chunkRendererMap.empty())
            regionIter=m_regionRenderers.erase(regionIter);
        else
            ++regionIter;
    }

    //add missing chunks
    for(auto iter=chunks.begin(); iter!=chunks.end(); ++iter)
    {
        ChunkRenderType *chunkRenderer=getFreeRenderer();

        if(chunkRenderer==nullptr)
            continue;

        Key key(iter->first);
        SharedChunkHandle chunkHandle=m_grid->getChunk(key.regionHash, key.chunkHash);

//        m_grid->loadChunk(chunkHandle, 0);

//        chunkRenderer->setRegionHash(key.regionHash);
        chunkRenderer->setChunk(chunkHandle);
//        chunkRenderer->setChunkOffset(iter->second.offset);
//        chunkRenderer->update();

        m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);

        auto regionIter=m_regionRenderers.find(key.regionHash);

        if(regionIter == m_regionRenderers.end())
        {
            glm::ivec3 index=m_grid->getRegionIndex(key.regionHash);

            auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(key.regionHash, RegionRendererType(key.regionHash, index, iter->second.offset)));

            assert(interResult.second);
            regionIter=interResult.first;
        }
        regionIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(key.chunkHash, chunkRenderer));
    }

    //we likely altered the chunks, start the occlussion query over
    m_currentQueryRing=0;
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updateOcclusionQueries()
{
    if(!m_queryComplete)
        return;

    m_outstandingChunkLoads=0;
    m_outstandingChunkPreps=0;

    if(m_currentQueryRing>=m_maxChunkRing) //we have completed all query rings, start over
        m_currentQueryRing=0;
    
    size_t queriesStarted=0;

    std::vector<ChunkRenderType *> &chunkRenderers=m_chunkQueryOrder[m_currentQueryRing];

    for(size_t i=0; i<chunkRenderers.size(); ++i)
    {
        ChunkRenderType *chunkRenderer=chunkRenderers[i];

        if(chunkRenderer->getState()==ChunkRenderType::Occluded)
        {
            chunkRenderer->startOcculsionQuery();
            queriesStarted++;
        }
    }

    m_currentQueryRing++;

    if(queriesStarted>0)
        m_queryComplete=false;
     
}

template<typename _Grid>
typename SimpleRenderer<_Grid>::ChunkRenderType *SimpleRenderer<_Grid>::getFreeRenderer()
{
    if(m_freeChunkRenderers.empty())
    {
//        size_t minRendererCount=(m_chunkIndices.size()*3)/2;
        size_t minRendererCount=0;
        
        for(size_t i=0; i<m_chunkIndices.size(); ++i)
            minRendererCount+=m_chunkIndices[i].size();
        minRendererCount=(minRendererCount*3)/2;

        if(minRendererCount<m_chunkRenderers.size())
            minRendererCount+m_chunkRenderers.size()+1;

        if(m_chunkRenderers.size()<minRendererCount)
        {
            size_t buildIndex=m_chunkRenderers.size();
            m_chunkRenderers.resize(minRendererCount);

            //need to setup buffers for new chunks
            for(size_t i=buildIndex; i<m_chunkRenderers.size(); ++i)
            {
                ChunkRenderType *chunkRenderer=new ChunkRenderType();

                m_chunkRenderers[i].reset(chunkRenderer);

                chunkRenderer->setParent(this);
                chunkRenderer->build(m_instanceVertices);
                chunkRenderer->buildOutline(m_outlineInstanceVertices);

                m_freeChunkRenderers.push_back(chunkRenderer);
            }
        }
    }

    if(m_freeChunkRenderers.empty())
        return nullptr;
    
    ChunkRenderType *renderer=m_freeChunkRenderers.back();

    m_freeChunkRenderers.pop_back();
    return renderer;
}


template<typename _Grid>
void SimpleRenderer<_Grid>::addPrepQueue(ChunkRenderType *chunkRenderer)
{
    {
        std::unique_lock<std::mutex> lock(m_prepMutex);

        m_prepQueue.push(chunkRenderer);
    }
    m_prepEvent.notify_all();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::startPrepThread()
{
    m_prepThreadRun=true;
    m_prepThread=std::thread(std::bind(&SimpleRenderer<_Grid>::prepThread, this));
}

template<typename _Grid>
void SimpleRenderer<_Grid>::stopPrepThread()
{
    {
        std::unique_lock<std::mutex> lock(m_prepMutex);
        m_prepThreadRun=false;
    }

    m_prepEvent.notify_all();
    m_prepThread.join();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::prepThread()
{
    std::unique_lock<std::mutex> lock(m_prepMutex);

#ifdef _WINDOWS
    wglMakeCurrent(m_prepDC, m_prepGlContext);
#else
    assert(false);
#endif

    while(m_prepThreadRun)
    {
        if(m_prepQueue.empty())
        {
            m_prepEvent.wait(lock);
            continue;
        }

        ChunkRenderType *chunkRenderer=m_prepQueue.front();

        if(chunkRenderer == nullptr)
            return;

        m_prepQueue.pop();

        lock.unlock();//drop lock while working

        chunkRenderer->update();
        
        lock.lock();

        m_chunksUpdated.push_back(chunkRenderer);
    }
}

}//namespace voxigen