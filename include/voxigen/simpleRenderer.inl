#include "voxigen/chunkFunctions.h"
#include "voxigen/search.h"

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
"layout (location = 1) in ivec2 vTexCoords;\n"
"layout (location = 2) in uint data;\n"
"\n"
"out vec3 position;\n"
"//out vec3 normal;\n"
"out vec2 texCoords;\n"
"flat out uint type;\n"
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
"   texCoords=vec2(vTexCoords.x, vTexCoords.y);\n"
"//   texCoords=vec3(0.0, 0.0, data);\n"
"   type=data;\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"

"}\n"
"";

template<typename _Grid>
std::string SimpleRenderer<_Grid>::fragmentShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"//in vec3 normal;\n"
"in vec2 texCoords;\n"
"flat in uint type;\n"
"out vec4 color;\n"
"\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"\n"
"uniform sampler2D textureSampler;\n"
"\n"
"void main()\n"
"{\n"
"   vec3 normal = cross(dFdy(position), dFdx(position));\n"
"   normal=normalize(normal);\n"
"\n"
"//   float value=texCoords.z/10.0f;\n"
"//   color=vec3(texCoords.x, 0.0, texCoords.y);\n"
"   // ambient\n"
"   float ambientStrength=0.5;\n"
"   vec3 ambient=ambientStrength * lightColor;\n"
"   \n"
"   // diffuse \n"
"   vec3 lightDir=normalize(lightPos-position); \n"
"   float diff=max(dot(normal, lightDir), 0.0); \n"
"   vec3 diffuse=diff * lightColor; \n"
"//   color=vec4(texCoords.x/1024.0f, texCoords.y/1024.0f, 0.0f, 1.0f);\n"
"   color=texelFetch(textureSampler, ivec2(texCoords), 0);\n"
"//   color=vec4((ambient+diffuse)*color.rgb, color.a);\n"
"//   color=vec4((ambient+diffuse)*vec3(value, value, value), 1.0f);\n"
"//   color=vec4(abs(normal), 1.0f);\n"
"//   color=vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
"//   if(type==1u)\n"
"//       color=vec4(0.2f, 0.2f, 1.0f, 0.8f);\n"
"//   else if(type>=2u && type<=3u)\n"
"//       color=vec4(0.08f, 0.53f, 0.08f, 1.0f);\n"
"//   else if(type>3u)\n"
"//   {\n"
"//       float level=1.0f-(float(type-3u)/10.f);"
"//       color=vec4(level, level, level, 1.0f);\n"
"//   }\n"
"//   color=vec4(color.rgb*(ambient+diffuse), color.a);\n"
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
"out vec3 cubePos;\n"
"\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"//   position=inputOffset.xyz+inputVertex;\n"
"   cubePos=inputVertex;\n"
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
"in vec3 vertexColor;\n"
"in vec3 cubePos;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform vec3 lightPos;\n"
"uniform vec3 statusColor;\n"
"uniform float lineWidth=0.1;\n"
"\n"
"void main()\n"
"{\n"
"//   float value=1.0f;"
"//   vec3 lightColor=vec3(1.0f, 1.0f, 1.0f);\n"
"   vec3 distance=min(cubePos, vec3(64.0, 64.0, 16.0)-cubePos);\n"
"   float ambientStrength=0.5; \n"
"//   vec3 ambient=ambientStrength * lightColor;\n"
"   \n"
"   int count=0;\n"
"   if(distance.x < lineWidth)\n"
"       count++;\n"
"   if(distance.y < lineWidth)\n"
"       count++;\n"
"   if(distance.z < lineWidth)\n"
"       count++;\n"
"   if(count<2)\n"
"       discard;\n"
"   // diffuse \n"
"//   vec3 lightDir=normalize(lightPos-position); \n"
"//   float diff=max(dot(normal, lightDir), 0.0); \n"
"//   vec3 diffuse=diff*lightColor; \n"
"//   color=vec4(statusColor*(ambientStrength+diff), 1.0f);\n"
"   color=vec4(statusColor, 1.0f);\n"
"}\n"
"";

template<typename _Grid>
SimpleRenderer<_Grid>::SimpleRenderer(_Grid *grid):
m_grid(grid),
m_viewRadius(128.0f, 128.0f, 128.0f),
m_viewLODDistance(90.0f),
m_lastUpdatePosition(0.0f, 0.0f, 0.0),
m_projectionViewMatUpdated(true),
m_camera(nullptr),
m_queryComplete(true),
m_renderCube(grid, &grid->getDescriptors(), &m_renderPrepThread)
//m_updateChunks(false)
{
#ifdef NDEBUG
    m_displayOutline=false;
#else
    m_displayOutline=true;
#endif

    m_projectionMat=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f);

    m_viewMat=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);

    _Grid::DescriptorType &descriptors=m_grid->getDescriptors();

//    m_grid->setChunkUpdateCallback(std::bind(&SimpleRenderer<_Grid>::updateCallback, this, std::placeholders::_1));
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
//    m_outlineLightPositionId=m_outlineProgram.getUniformId("lightPos");
    m_outlineOffsetId=m_outlineProgram.getUniformId("regionOffset");
    m_outlineStatusColor=m_outlineProgram.getUniformId("statusColor");

    const std::vector<float> &outlineVertices=SimpleCube<ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value>::vertCoords;

    glGenBuffers(1, &m_outlineInstanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineInstanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //build texture for textureAtlas
    glGenTextures(1, &m_textureAtlasId);

    m_renderCube.setOutlineInstance(m_outlineInstanceVertices);

#ifdef _WINDOWS
    m_prepDC=wglGetCurrentDC();
    HGLRC currentContext=wglGetCurrentContext();

    m_prepGlContext=wglCreateContext(m_prepDC);
    
    BOOL success=wglShareLists(currentContext, m_prepGlContext);

    assert(success);
#else
    assert(false); //need os specific calls to create context for prepThread
#endif
//    startPrepThread();
    m_renderPrepThread.start(m_prepDC, m_prepGlContext);
}

template<typename _Grid>
void SimpleRenderer<_Grid>::destroy()
{
//    stopPrepThread();
    m_renderPrepThread.stop();
//    m_rendererMap.clear();
    m_regionRenderers.clear();
    m_chunkRenderers.clear();
//    m_chunkRendererMap.clear();
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

    if(m_textureAtlasDirty)
    {
        if(m_textureAtlas)
        {
            glBindTexture(GL_TEXTURE_2D, m_textureAtlasId);

            const imglib::SimpleImage &image=m_textureAtlas->getImage();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        m_textureAtlasDirty=false;
    }

//    if(m_updateRenderCube)

    if(cameraDirty)
    {
        m_program.uniform(m_uniformProjectionViewId)=m_camera->getProjectionViewMat();

        //camera moved restart queries
//        m_currentQueryRing=0;

//        m_program.uniform(m_lightPositionId)=m_camera->getPosition();
    }

    
    
//    if(!m_chunksUpdated.empty())
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//
//        for(size_t i=0; i<m_chunksUpdated.size(); ++i)
//        {
////            m_chunksUpdated[i]->refCount=0; //done in the prepThread
//            m_chunksUpdated[i]->updated();
////            m_chunksUpdated[i]->getChunkHandle()->release();//drop chunk as we have a mesh for it
//            m_chunksUpdated[i]->releaseChunkMemory();
////            SharedChunkHandle chunkHandle=m_chunksUpdated[i]->getChunkHandle();
////
////            chunkHandle->release();
//        }
//
//        m_chunksUpdated.clear();
//    }

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
#ifdef OCCLUSSION_QUERY
    updateOcclusionQueries();
#endif //OCCLUSSION_QUERY
//    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());

    //draw all chunks that are built, using regions
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureAtlasId);

//    for(auto &iter:m_regionRenderers)
//    {
//        RegionRenderer<ChunkRenderType> &renderer=iter.second;
//
////        m_program.uniform(m_offsetId)=renderer.offset;
//        glm::vec3 regionOffset=renderer.index-playerRegionIndex;
//        
//        renderer.offset=regionOffset*glm::vec3(m_grid->getDescriptors().m_regionCellSize);
//
//        for(auto &chunkIter:renderer.chunkRenderers)
//        {
//            ChunkRenderType *chunkRenderer=chunkIter.second;
//            
//            if(chunkRenderer->getState()==ChunkRenderType::Copy)
//            {
//                if(chunkRenderer->incrementCopy())
//                    m_outstandingChunkPreps--;
//            }
//
//            if(chunkRenderer->getState()==ChunkRenderType::Built) //only set uniform if we are going to draw, could be copying
//                m_program.uniform(m_offsetId)=(renderer.offset+chunkRenderer->getGridOffset());
//
//            chunkRenderer->draw();
//        }
//    }


    m_renderCube.draw(&m_program, m_offsetId);

    //turn off writing color buffer and depth buffer, just checking if it would render
    glColorMask(false, false, false, false);
    glDepthMask(GL_FALSE);

#ifdef OCCLUSSION_QUERY
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
#endif //OCCLUSSION_QUERY
    //done querying, turn color and depth buffer back on
    glColorMask(true, true, true, true);
    glDepthMask(GL_TRUE);


//#ifndef NDEBUG
    //draw Missing blocks in debug
    if(m_displayOutline)
    {
        m_outlineProgram.use();

        if(cameraDirty)
        {
            m_outlineProgram.uniform(m_uniformOutlintProjectionViewId)=m_camera->getProjectionViewMat();
//            m_outlineProgram.uniform(m_outlineLightPositionId)=m_camera->getPosition();
        }
        
        m_renderCube.drawOutline(&m_outlineProgram, m_outlineOffsetId, m_outlineStatusColor);
//        m_regionRenderCube.drawOutline(&m_outlineProgram, m_outlineOffsetId, m_outlineStatusColor);

//        for(auto &iter:m_regionRenderers)
//        {
//            RegionRenderer<ChunkRenderType> &renderer=iter.second;
//
//            m_outlineProgram.uniform(m_outlineOffsetId)=renderer.offset;
//
//            for(auto &chunkIter:renderer.chunkRenderers)
//            {
//                ChunkRenderType *chunkRenderer=chunkIter.second;
//                
//                chunkRenderer->drawOutline(&m_outlineProgram, m_outlineStatusColor);
//            }
//        }
    }
//#endif //NDEBUG

//    rendererUpdateChunks();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::update()
{
    //update render information

    //update player position
//    glm::ivec3 regionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//    glm::ivec3 chunkIndex=m_grid->getChunkIndex(m_camera->getPosition());

    m_renderCube.update(m_playerRegionIndex, m_playerChunkIndex);
//    m_regionRenderCube.update(regionIndex, chunkIndex);

    //force cached item into the queue
    m_grid->updateProcessQueue();

    //update chunks that have been loaded by the grid
    updateChunkHandles();

    //update render prep chunk
    updatePrepChunks();
//        if(!requestUpdate.empty())
//            m_renderPrepThread.requestUpdate(requestUpdate);
//    }
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updateChunkHandles()
{
    std::vector<Key> updatedChunks=m_grid->getUpdatedChunks();

    if(updatedChunks.empty())
        return;
    
    RenderCubeType::ChunkRendererType *renderer;

    for(size_t i=0; i<updatedChunks.size(); ++i)
    {
        Key &key=updatedChunks[i];

        renderer=m_renderCube.getChunkRenderInfo(key);

        if(renderer==nullptr)
            continue;

        //update finished back to idle
        if(!renderer->getChunkHandle()->empty())
        {
            //Chunk ready and renderer set so lets get a mesh
            renderer->setAction(RenderAction::Meshing);
            m_renderPrepThread.requestMesh(renderer, m_textureAtlas.get());
        }
    }
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updatePrepChunks()
{
    //TODO: need to add limits on how many to process in a single update

    m_renderPrepThread.updateQueues(m_completedRequest);

    if(!m_completedRequest.empty())
    {
//        for(RenderPrepThreadType::Request *request:m_completedRequest)
        for(size_t i=0; i<m_completedRequest.size(); ++i)
        {
            RenderPrepThreadType::Request *request=m_completedRequest[i];

            switch(request->type)
            {
//            case prep::Add:
//                processAdd((RenderPrepThreadType::RequestAdd *)request);
//                break;
//            case prep::Remove:
//                processRemove((RenderPrepThreadType::RequestRemove *)request);
//                break;
            case prep::Mesh:
                processMesh((RenderPrepThreadType::RequestMesh *)request);
                break;
            }
            delete request;
        }
        m_completedRequest.clear();
    }

//    m_renderPrepThread.updateQueues(m_addedChunkRenderers, m_updatedChunkRenderers, m_removedChunkRenderers);
//
//    if(!m_addedChunkRenderers.empty())
//    {
//        for(auto renderer:m_addedChunkRenderers)
//        {
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") added";
//#endif//LOG_PROCESS_QUEUE
//
//            auto info=m_renderCube.getChunkRenderInfo(renderer->getKey());
//
//            if(info==nullptr)
//            {
//                //need to remove renderer as chunk no longer used
//                m_renderPrepThread.requestRemove(renderer);
//                continue;
//            }
//
//            info->renderer=renderer;
//
//            if(info->chunkHandle->state()==ChunkState::Memory)
//            {
//                //we have data on hand so lets request it to be meshed
//                //make sure that it is idle before request chunking
//                if(info->chunkHandle->action()==ChunkAction::Idle)
//                {
//                    info->state=RenderState::Meshing;
//                    m_renderPrepThread.requestUpdate(renderer);
//                }
//                else
//                    assert(false);//this stuff should not be out of sync
//            }
//        }
//        m_addedChunkRenderers.clear();
//    }
//
//    if(!m_updatedChunkRenderers.empty())
//    {
//        //updated chunks just need to swap out the mesh as that is all that should have been changed
//        for(auto renderer:m_updatedChunkRenderers)
//        {
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") updated";
//#endif//LOG_PROCESS_QUEUE
//
//            auto info=m_renderCube.getChunkRenderInfo(renderer->getKey());
//
//            if(info==nullptr)
//            {
//                //need to remove renderer as chunk no longer used
//                m_renderPrepThread.requestRemove(renderer);
//                continue;
//            }
//            
//            info->state=RenderState::Idle;
//        }
//        m_updatedChunkRenderers.clear();
//    }
//
//    if(!m_removedChunkRenderers.empty())
//    {
//        for(auto renderer:m_removedChunkRenderers)
//        {
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") removed";
//#endif//LOG_PROCESS_QUEUE
//        }
//        m_removedChunkRenderers.clear();
//    }
}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::processAdd(typename RenderPrepThreadType::RequestAdd *request)
//{
//    ChunkRenderType *renderer=request->renderer;
//
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") added";
//#endif//LOG_PROCESS_QUEUE
//
//    auto info=m_renderCube.getChunkRenderInfo(renderer->getKey());
//
//    if(info==nullptr)
//    {
//        //need to remove renderer as chunk no longer used
//        m_renderPrepThread.requestRemove(renderer);
//        return;
//    }
//
//    info->renderer=renderer;
//
//    //has to be done here as VAO gen not supported across contexts
//    renderer->build();
//    renderer->buildOutline(m_outlineInstanceVertices);
//
//    if(info->chunkHandle->state()==ChunkState::Memory)
//    {
//        //we have data on hand so lets request it to be meshed
//        //make sure that it is idle before request chunking
//        if(info->chunkHandle->action()==ChunkAction::Idle) 
//        {
//            if(!info->chunkHandle->empty())
//            { //no mesh needed for empty chunk
//                info->state=RenderState::Meshing;
//                m_renderPrepThread.requestMesh(renderer, m_textureAtlas.get());
//            }
//        }
//        else
//            assert(false);//this stuff should not be out of sync
//    }
//    else
//        info->state=RenderState::Idle;
//}
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::processRemove(typename RenderPrepThreadType::RequestRemove *request)
//{
//    ChunkRenderType *renderer=request->renderer;
//
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" removed";
//#endif//LOG_PROCESS_QUEUE
//}

template<typename _Grid>
void SimpleRenderer<_Grid>::processMesh(typename RenderPrepThreadType::RequestMesh *request)
{
    ChunkRenderType *renderer=request->renderer;

    //updated chunks just need to swap out the mesh as that is all that should have been changed
#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") updated";
#endif//LOG_PROCESS_QUEUE

    auto cubeRenderer=m_renderCube.getChunkRenderInfo(renderer->getKey());

    if((cubeRenderer==nullptr) || (cubeRenderer!=renderer))
    {
        //need to remove renderer as chunk no longer used
//        m_renderPrepThread.requestRemove(renderer);
        m_renderPrepThread.requestReleaseMesh(request->mesh);
        renderer->setAction(RenderAction::Idle); //renderer is idle again, make sure it can be cleaned up
        return;
    }

    MeshBuffer mesh=renderer->setMesh(request->mesh);

    if(mesh.valid)
        m_renderPrepThread.requestReleaseMesh(mesh);

    cubeRenderer->setAction(RenderAction::Idle);

    m_grid->releaseChunk(cubeRenderer->getChunkHandle());
}


template<typename _Grid>
void SimpleRenderer<_Grid>::setCamera(SimpleFpsCamera *camera)
{
    m_camera=camera;

    m_playerRegionIndex=m_grid->getRegionIndex(camera->getRegionHash());
    m_playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());

    if(glm::distance(m_camera->getPosition(), m_lastUpdatePosition)>8.0f)
        m_lastUpdatePosition=m_camera->getPosition();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setCameraChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    m_renderCube.updateCamera(regionIndex, chunkIndex);
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setViewRadius(const glm::ivec3 &radius)
{
    m_viewRadius=radius;
//    m_viewRadiusMax=radius*1.5f;

    m_renderCube.setViewRadius(radius);
//    m_maxChunkRing=std::ceil(radius/std::max(std::max(ChunkType::sizeX::value, ChunkType::sizeY::value), ChunkType::sizeZ::value));
//
//    m_chunkIndices.resize(m_maxChunkRing);
//
//    for(size_t i=0; i<m_maxChunkRing; ++i)
//        ringCube<ChunkType>(m_chunkIndices[i], i);
//    m_chunkIndices=buildRadiusRingMap<ChunkType>(m_viewRadiusMax);
    m_chunkIndices.resize(1);
//    spiralCube<ChunkType>(m_chunkIndices[0], m_viewRadiusMax);

    m_maxChunkRing=m_chunkIndices.size();

//    m_updateRenderCube=true;
//    if(m_chunkIndices.empty()) //always want at least the current chunk
//        m_chunkIndices.push_back(glm::ivec3(0, 0, 0));
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setPlayerChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    m_playerRegionIndex=regionIndex;
    m_playerChunkIndex=chunkIndex;
}

struct ChunkQueryOffset
{
    ChunkQueryOffset(size_t queryRing, glm::vec3 &offset):queryRing(queryRing), offset(offset){}

    size_t queryRing;
    glm::vec3 offset;
};

//template<typename _Grid>
//typename SimpleRenderer<_Grid>::ChunkRenderType *SimpleRenderer<_Grid>::createRenderNode(Key key)
//{
//    ChunkRenderMap::iterator iter=m_chunkRendererMap.find(key.hash);
//
//    if(iter!=m_chunkRendererMap.end())
//        return iter->second;
//
//    ChunkRenderType *renderer=getFreeRenderer();
//
//    if(renderer==nullptr)
//        return nullptr;
//
//    SharedChunkHandle chunkHandle=m_grid->getChunk(key.regionHash, key.chunkHash);
//
//    //add region if not found
//    auto regionIter=m_regionRenderers.find(key.regionHash);
//
//    if(regionIter==m_regionRenderers.end())
//    {
//        glm::ivec3 index=m_grid->getRegionIndex(key.regionHash);
//
//        auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(key.regionHash, RegionRendererType(key.regionHash, index, renderer->getChunkOffset())));
//        regionIter=interResult.first;
//    }
//    regionIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(key.chunkHash, renderer));
//
//    renderer->setChunk(chunkHandle);
//    m_chunkRendererMap.insert(ChunkRenderMap::value_type(key.hash, renderer));
//    return renderer;
//}

#ifndef OLD_SEARCH

//template<typename _Grid>
//void SimpleRenderer<_Grid>::updateChunks()
//{
//    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());
//
//    m_searchMap=buildRingSearchMap<_Grid, ChunkRenderType>(m_grid, playerRegionIndex, playerChunkIndex, m_chunkIndices, std::bind(&SimpleRenderer<_Grid>::createRenderNode, this, std::placeholders::_1));
//
//    m_currentRegion=playerRegionIndex;
//    m_currentChunk=playerChunkIndex;
//
//    m_currentQueryRing=0;
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::rendererUpdateChunks()
//{
//    if(m_addedChunkRenderers.empty()&&m_updatedChunkRenderers.empty()&&m_removedChunkRenderers.empty())
//        return;
//
//    _Grid::DescriptorType &descriptors=m_grid->getDescriptors();
//
//    std::vector<ChunkRenderType *> addedChunkRenderers;
//    std::vector<ChunkRenderType *> updatedChunkRenderers;
//    std::vector<ChunkRenderType *> removedChunkRenderers;
//    
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//
//        addedChunkRenderers.insert(addedChunkRenderers.end(), m_addedChunkRenderers.begin(), m_addedChunkRenderers.end());
//        m_addedChunkRenderers.clear();
//        updatedChunkRenderers.insert(updatedChunkRenderers.end(), m_updatedChunkRenderers.begin(), m_updatedChunkRenderers.end());
//        m_updatedChunkRenderers.clear();
//        removedChunkRenderers.insert(removedChunkRenderers.end(), m_removedChunkRenderers.begin(), m_removedChunkRenderers.end());
//        m_removedChunkRenderers.clear();
//    }
//
//    if(!updatedChunkRenderers.empty())
//    {
//        for(auto renderer:updatedChunkRenderers)
//        {
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") request update (forced load)";
//#endif//LOG_PROCESS_QUEUE
//
//            m_grid->loadChunk(renderer->getChunkHandle(), renderer->getLod(), true);
//        }
////        updatedChunkRenderers.clear();
//    }
//
//    if(!addedChunkRenderers.empty())
//    {
//        for(auto renderer:addedChunkRenderers)
//        {
//            auto regionIter=m_regionRenderers.find(renderer->getRegionHash());
//            
//            if(regionIter==m_regionRenderers.end())
//            {
//                glm::ivec3 index=m_grid->getRegionIndex(renderer->getRegionHash());
//                glm::ivec3 offset=index*descriptors.getRegionCellSize();
//            
//                auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(renderer->getRegionHash(), RegionRendererType(renderer->getRegionHash(), index, offset)));
//            
//                regionIter=interResult.first;
//            }
//            regionIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(renderer->getChunkHash(), renderer));
//
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") init";
//#endif//LOG_PROCESS_QUEUE
//
//            renderer->build(m_instanceVertices);
//            renderer->buildOutline(m_outlineInstanceVertices);
//#ifndef OCCLUSSION_QUERY
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") request load";
//#endif//LOG_PROCESS_QUEUE
//            m_grid->loadChunk(renderer->getChunkHandle(), renderer->getLod());
//#endif//!OCCLUSSION_QUERY
//        }
////        m_addedChunkRenderers.clear();
//    }
//
//    if(!removedChunkRenderers.empty())
//    {
//        std::vector<ChunkRenderType *> renderersToRemove;
//
//        for(auto renderer:removedChunkRenderers)
//        {
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") request remove";
//#endif//LOG_PROCESS_QUEUE
////            m_freeChunkRenderers.push_back(renderer);
//            renderersToRemove.push_back(renderer);
//
//            auto regionIter=m_regionRenderers.find(renderer->getRegionHash());
//
//            if(regionIter==m_regionRenderers.end())
//                continue;
//
//            RegionRenderer<ChunkRendererType>::ChunkRendererMap &chunkRenderers=regionIter->second.chunkRenderers;
//
//            auto chunkIter=chunkRenderers.find(renderer->getChunkHash());
//
//            if(chunkIter==chunkRenderers.end())
//                continue;
//
//            chunkRenderers.erase(chunkIter);
//        }
//        //m_removedChunkRenderers.clear();
//
//        if(!renderersToRemove.empty())
//            removePrepQueue(renderersToRemove);
//    }
//    
//    //notify thread that things should be in the queue
//    m_grid->updateProcessQueue();;
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::prepUpdateChunks(std::vector<ChunkRendererType *> &addRenderers, std::vector<ChunkRendererType *> &updateRenderers, std::vector<ChunkRendererType *> &removeRenderers)
//{
//    SearchSettings settings;
//
//    settings.radius=m_viewRadiusMax;
//    settings.lodDistance=m_viewLODDistance;
//    settings.regionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//    settings.chunkIndex=m_grid->getChunkIndex(m_camera->getPosition());
//
//    m_updateChunks=searchRenderers(m_grid, settings, std::bind(&SimpleRenderer<_Grid>::getFreeRenderer, this), m_chunkRendererMap, addRenderers, updateRenderers, removeRenderers);
////    _Grid::DescriptorType &descriptors=m_grid->getDescriptors();
////
////    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
////    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());
////
////    typedef std::unordered_map<Key::Type, bool> AddMap;
////    AddMap addChunk;
////
////    RegionHash playerRegionHash=descriptors.regionHash(playerRegionIndex);
////    ChunkHash playerChunkHash=descriptors.chunkHash(playerChunkIndex);
////    Key playerChunkKey(playerRegionHash, playerChunkHash);
////
////    //make sure player chunk added
////    addChunk.insert(AddMap::value_type(playerChunkKey.hash, true));
////
////    for(auto chunkIter=m_chunkRendererMap.begin(); chunkIter!=m_chunkRendererMap.end(); )
////    {
////        auto *chunkRenderer=chunkIter->second;
////        Key key(chunkRenderer->getRegionHash(), chunkRenderer->getChunkHash());
////
////        float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, chunkRenderer->getRegionIndex(), chunkRenderer->getChunkIndex());
////        
////        //chunk outside of range so invalidate
////        if(chunkDistance>m_viewRadiusMax)
////        {
////            addChunk[key.hash]=false;
////            removeRenderers.push_back(chunkRenderer);
////            chunkIter=m_chunkRendererMap.erase(chunkIter);
////        }
////        else
////        {
////            size_t lod=floor(chunkDistance/m_viewLODDistance);
////
////            if(chunkRenderer->getLod()!=lod)
////            {
////                chunkRenderer->setLod(lod);
////                updateRenderers.push_back(chunkRenderer);
////            }
////            addChunk[key.hash]=false;
////
////            //add neighbors chunks
////            for(int z=-1; z<=1; ++z)
////            {
////                for(int y=-1; y<=1; ++y)
////                {
////                    for(int x=-1; x<=1; ++x)
////                    {
////                        glm::ivec3 neighborRegion=chunkRenderer->getRegionIndex();
////                        glm::ivec3 neighborChunk=chunkRenderer->getChunkIndex()+glm::ivec3(x, y, z);
////
////                        descriptors.adjustRegion(neighborRegion, neighborChunk);
////                        float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, neighborRegion, neighborChunk);
////
////                        if(chunkDistance<m_viewRadiusMax)
////                        {
////                            RegionHash regionHash=descriptors.regionHash(neighborRegion);
////                            ChunkHash chunkHash=descriptors.chunkHash(neighborChunk);
////                            Key key(regionHash, chunkHash);
////
////                            if(addChunk.find(key.hash)==addChunk.end())
////                                addChunk.insert(AddMap::value_type(key.hash, true));
////                        }
////                    }
////                }
////            }
////            ++chunkIter;
////        }
////    }
////
////    size_t addedRenderers=0;
////    for(auto &iter:addChunk)
////    {
////        if(iter.second)
////        {
////            ChunkRenderType *chunkRenderer=getFreeRenderer();
////
////            if(chunkRenderer==nullptr)
////                continue;
////
////            Key key(iter.first);
////            SharedChunkHandle chunkHandle=m_grid->getChunk(key.regionHash, key.chunkHash);
////
////            chunkRenderer->setChunk(chunkHandle);
////
////            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, chunkRenderer->getRegionIndex(), chunkRenderer->getChunkIndex());
////            float lod=floor(chunkDistance/m_viewLODDistance);
////
////            chunkRenderer->setLod(lod);
////
////            m_chunkRendererMap.insert(ChunkRenderMap::value_type(key.hash, chunkRenderer));
////            addRenderers.push_back(chunkRenderer);
////            addedRenderers++;
////        }
////    }
////
////    if(addedRenderers > 0)
////        m_updateChunks=true;
////    else
////        m_updateChunks=false;
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::updateChunks()
//{
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//
//        m_updateChunks=true;
//    }
//    m_prepEvent.notify_all();
//}

//template<typename _Grid>
//bool SimpleRenderer<_Grid>::updateChunks()
//{
//    _Grid::DescriptorType &descriptors=m_grid->getDescriptors();
//
//    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());
//
//    typedef std::unordered_map<Key::Type, bool> AddMap;
//    AddMap addChunk;
//
//    RegionHash playerRegionHash=descriptors.regionHash(playerRegionIndex);
//    ChunkHash playerChunkHash=descriptors.chunkHash(playerRegionIndex);
//    Key playerChunkKey(playerRegionHash, playerChunkHash);
//
//    //make sure player chunk added
//    addChunk.insert(AddMap::value_type(playerChunkKey.hash, true));
//
//    for(auto regionIter=m_regionRenderers.begin(); regionIter!=m_regionRenderers.end(); )
//    {
//        RegionRendererType &regionRenderer=regionIter->second;
//        glm::ivec3 regionOffset=regionRenderer.index-playerRegionIndex;
//
//        regionRenderer.offset=regionOffset*m_grid->getDescriptors().m_regionCellSize;
//
//        auto &chunkRendererMap=regionRenderer.chunkRenderers;
//
//        for(auto chunkIter=chunkRendererMap.begin(); chunkIter!=chunkRendererMap.end(); )
//        {
//            auto *chunkRenderer=chunkIter->second;
//            Key key(chunkRenderer->getRegionHash(), chunkRenderer->getChunkHash());
//
//            glm::ivec3 regionIndex=m_grid->getDescriptors().regionIndex(key.regionHash);
//            glm::ivec3 chunkIndex=m_grid->getDescriptors().chunkIndex(key.chunkHash);
//            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, regionIndex, chunkIndex);
//
//            //chunk outside of range so invalidate
//            if(chunkDistance>m_viewRadiusMax)
//            {
//                addChunk[key.hash]=false;
//                if(chunkRenderer->refCount==0) //need to keep if in prepThread
//                {
//                    chunkRenderer->invalidate();
//                    chunkIter=chunkRendererMap.erase(chunkIter);
//                    m_freeChunkRenderers.push_back(chunkRenderer);
//                    continue;
//                }
//            }
//            else
//            {
//                addChunk[key.hash]=false;
//
//                //add neighbors chunks
//                for(int z=-1; z<=1; ++z)
//                {
//                    for(int y=-1; y<=1; ++y)
//                    {
//                        for(int x=-1; x<=1; ++x)
//                        {
//                            glm::ivec3 neighborRegion=regionIndex;
//                            glm::ivec3 neighborChunk=chunkIndex+glm::ivec3(x, y, z);
//
//                            descriptors.adjustRegion(neighborRegion, neighborChunk);
//                            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, neighborRegion, neighborChunk);
//
//                            if(chunkDistance<m_viewRadiusMax)
//                            {
//                                RegionHash regionHash=descriptors.regionHash(neighborRegion);
//                                ChunkHash chunkHash=descriptors.chunkHash(neighborChunk);
//                                Key key(regionHash, chunkHash);
//
//                                if(addChunk.find(key.hash)==addChunk.end())
//                                    addChunk.insert(AddMap::value_type(key.hash, true));
//                            }
//                        }
//                    }
//                }
//                ++chunkIter;
//            }
//        }
//
//        if(chunkRendererMap.empty())
//            regionIter=m_regionRenderers.erase(regionIter);
//        else
//            ++regionIter;
//    }
//
//    size_t addedRenderers=0;
//    for(auto &iter:addChunk)
//    {
//        if(iter.second)
//        {
//            ChunkRenderType *chunkRenderer=getFreeRenderer();
//
//            if(chunkRenderer==nullptr)
//                continue;
//
//            Key key(iter.first);
//            SharedChunkHandle chunkHandle=m_grid->getChunk(key.regionHash, key.chunkHash);
//
//            chunkRenderer->setChunk(chunkHandle);
//
//            auto regionIter=m_regionRenderers.find(key.regionHash);
//
//            if(regionIter==m_regionRenderers.end())
//            {
//                glm::ivec3 index=m_grid->getRegionIndex(key.regionHash);
//                glm::ivec3 offset=index*descriptors.getRegionCellSize();
//
//                auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(key.regionHash, RegionRendererType(key.regionHash, index, offset)));
//
//                assert(interResult.second);
//                regionIter=interResult.first;
//            }
//            
//            m_addedChunks.push_back(chunkRenderer);
//            regionIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(key.chunkHash, chunkRenderer));
//#ifndef OCCLUSSION_QUERY
//            m_grid->loadChunk(chunkRenderer->getChunkHandle(), 0);
//#endif//!OCCLUSSION_QUERY
//
//            addedRenderers++;
//        }
//    }
//
//    if(addedRenderers > 0)
//        return true;
//    return false;
//}

#else//!OLD_SEARCH

//template<typename _Grid>
//bool SimpleRenderer<_Grid>::updateChunks()
//{
//    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());
//
//    int chunkIndicesSize=m_chunkIndices.size();
//
////    std::unordered_map<RegionHash, glm::vec3> regions;
//    std::unordered_map<Key::Type, ChunkQueryOffset> chunks;
//    glm::ivec3 index;
//    glm::ivec3 currentRegionIndex;
//
//    for(size_t i=0; i<m_maxChunkRing; ++i)
//    {
//        std::vector<glm::ivec3> &chunkIndices=m_chunkIndices[i];
//
//        for(size_t j=0; j<chunkIndices.size(); ++j)
//        {
//            index=playerChunkIndex+chunkIndices[j];
//            currentRegionIndex=playerRegionIndex;
//
//            glm::vec3 regionOffset=m_grid->getDescriptors().adjustRegion(currentRegionIndex, index);
//            Key key=m_grid->getHashes(currentRegionIndex, index);
//
//            chunks.insert(std::pair<Key::Type, ChunkQueryOffset>(key.hash, ChunkQueryOffset(i, regionOffset)));
//        }
//    }
//
//    m_chunkQueryOrder.resize(m_maxChunkRing);
//
//    //invalidate renderers that pass outside max range
//    for(auto regionIter=m_regionRenderers.begin(); regionIter!=m_regionRenderers.end(); )
//    {
//        RegionRendererType &regionRenderer=regionIter->second;
//        glm::ivec3 regionOffset=regionRenderer.index-playerRegionIndex;
//
//        regionRenderer.offset=regionOffset*m_grid->getDescriptors().m_regionCellSize;
//
//        auto &chunkRendererMap=regionRenderer.chunkRenderers;
//
//        for(auto chunkIter=chunkRendererMap.begin(); chunkIter!=chunkRendererMap.end(); )
//        {
//            auto *chunkRenderer=chunkIter->second;
//            Key key(chunkRenderer->getRegionHash(), chunkRenderer->getChunkHash());
//
//            glm::ivec3 regionIndex=m_grid->getDescriptors().regionIndex(key.regionHash);
//            glm::ivec3 chunkIndex=m_grid->getDescriptors().chunkIndex(key.chunkHash);
//            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, regionIndex, chunkIndex);
//
//            //chunk outside of range so invalidate
//            if(chunkDistance > m_viewRadiusMax)
//            {
//                if(chunkRenderer->refCount == 0) //need to keep if in prepThread
//                {
//                    chunkRenderer->invalidate();
//                    chunkIter=chunkRendererMap.erase(chunkIter);
//                    m_freeChunkRenderers.push_back(chunkRenderer);
//                    continue;
//                }
//            }
//            else
//            {
//                auto iter=chunks.find(key.hash);
//
//                if(iter!=chunks.end())
//                {
//                    m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);
//                    chunks.erase(iter);
//                }
//            }
//            ++chunkIter;
//        }
//
//        if(chunkRendererMap.empty())
//            regionIter=m_regionRenderers.erase(regionIter);
//        else
//            ++regionIter;
//    }
//
//    //add missing chunks
//    for(auto iter=chunks.begin(); iter!=chunks.end(); ++iter)
//    {
//        ChunkRenderType *chunkRenderer=getFreeRenderer();
//
//        if(chunkRenderer==nullptr)
//            continue;
//
//        Key key(iter->first);
//        SharedChunkHandle chunkHandle=m_grid->getChunk(key.regionHash, key.chunkHash);
//
////        m_grid->loadChunk(chunkHandle, 0);
//
////        chunkRenderer->setRegionHash(key.regionHash);
//        chunkRenderer->setChunk(chunkHandle);
////        chunkRenderer->setChunkOffset(iter->second.offset);
////        chunkRenderer->update();
//
//        m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);
//
//        auto regionIter=m_regionRenderers.find(key.regionHash);
//
//        if(regionIter == m_regionRenderers.end())
//        {
//            glm::ivec3 index=m_grid->getRegionIndex(key.regionHash);
//
//            auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(key.regionHash, RegionRendererType(key.regionHash, index, iter->second.offset)));
//
//            assert(interResult.second);
//            regionIter=interResult.first;
//        }
//
//        regionIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(key.chunkHash, chunkRenderer));
//    }
//
//    //we likely altered the chunks, start the occlussion query over
//    m_currentQueryRing=0;
//
//    return false;
//}
#endif//OLD_SEARCH

//template<typename _Grid>
//void SimpleRenderer<_Grid>::updateOcclusionQueries()
//{
//    if(!m_queryComplete)
//        return;
//
//    m_outstandingChunkLoads=0;
//    m_outstandingChunkPreps=0;
//
//    if(m_currentQueryRing>=m_maxChunkRing) //we have completed all query rings, start over
//        m_currentQueryRing=0;
//    
//    size_t queriesStarted=0;
//
//#ifdef OLD_SEARCH
//    std::vector<ChunkRenderType *> &chunkRenderers=m_chunkQueryOrder[m_currentQueryRing];
//
//    for(size_t i=0; i<chunkRenderers.size(); ++i)
//    {
//        ChunkRenderType *chunkRenderer=chunkRenderers[i];
//
//        if(chunkRenderer->getState()==ChunkRenderType::Occluded)
//        {
//            chunkRenderer->startOcculsionQuery();
//            queriesStarted++; 
//        }
//    }
//
//    m_currentQueryRing++;
//#else//OLD_SEARCH
////    std::vector<ChunkRenderType *> &chunkRenderers=m_searchMap[m_currentQueryRing];
//
//    for(ChunkRenderType *chunkRenderer:m_addedChunks)
//    {
//        if(chunkRenderer->getState()==ChunkRenderType::Occluded)
//        {
//            chunkRenderer->startOcculsionQuery();
//            queriesStarted++;
//        }
//    }
//    m_addedChunks.clear();
//#endif//OLD_SEARCH
//
//    if(queriesStarted>0)
//        m_queryComplete=false;
//     
//}
//
//template<typename _Grid>
//typename SimpleRenderer<_Grid>::ChunkRenderType *SimpleRenderer<_Grid>::getFreeRenderer()
//{
//    if(m_freeChunkRenderers.empty())
//    {
////        size_t minRendererCount=(m_chunkIndices.size()*3)/2;
//        size_t minRendererCount=0;
//        
//        for(size_t i=0; i<m_chunkIndices.size(); ++i)
//            minRendererCount+=m_chunkIndices[i].size();
//        minRendererCount=(minRendererCount*3)/2;
//
//        if(minRendererCount<m_chunkRenderers.size())
//            minRendererCount+m_chunkRenderers.size()+1;
//
//        if(m_chunkRenderers.size()<minRendererCount)
//        {
//            size_t buildIndex=m_chunkRenderers.size();
//            m_chunkRenderers.resize(minRendererCount);
//
//            //need to setup buffers for new chunks
//            for(size_t i=buildIndex; i<m_chunkRenderers.size(); ++i)
//            {
//                ChunkRenderType *chunkRenderer=new ChunkRenderType();
//
//                chunkRenderer->setTextureAtlas(m_textureAtlas);
//                m_chunkRenderers[i].reset(chunkRenderer);
//
//                chunkRenderer->setParent(this);
////                chunkRenderer->build(m_instanceVertices);
////                chunkRenderer->buildOutline(m_outlineInstanceVertices);
//
//                m_freeChunkRenderers.push_back(chunkRenderer);
//            }
//        }
//
//        if(m_freeChunkRenderers.empty())
//            return nullptr;
//    }
//
//    ChunkRenderType *renderer=m_freeChunkRenderers.back();
//
//    m_freeChunkRenderers.pop_back();
//    return renderer;
//}
//
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::addPrepQueue(ChunkRenderType *chunkRenderer)
//{
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//
////        if(std::find(m_prepQueue.begin(), m_prepQueue.end(), chunkRenderer)!=m_prepQueue.end())
////            LOG(INFO)<<"duplicate being added";
//
//        m_prepQueue.push_back(chunkRenderer);
//    }
//    m_prepEvent.notify_all();
//}
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::addPrepQueue(const std::vector<ChunkRenderType *> &chunkRenderers)
//{
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//
//        m_prepQueue.insert(m_prepQueue.end(), chunkRenderers.begin(), chunkRenderers.end());
//    }
//    m_prepEvent.notify_all();
//}
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::removePrepQueue(const std::vector<ChunkRenderType *> &chunkRenderers)
//{
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//
//        m_removeQueue.insert(m_removeQueue.end(), chunkRenderers.begin(), chunkRenderers.end());
//    }
//    m_prepEvent.notify_all();
//}
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::startPrepThread()
//{
//    m_prepThreadRun=true;
//    m_prepThread=std::thread(std::bind(&SimpleRenderer<_Grid>::prepThread, this));
//}
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::stopPrepThread()
//{
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//        m_prepThreadRun=false;
//    }
//
//    m_prepEvent.notify_all();
//    m_prepThread.join();
//}
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::prepThread()
//{
//    std::unique_lock<std::mutex> lock(m_prepMutex);
//
//#ifdef _WINDOWS
//    wglMakeCurrent(m_prepDC, m_prepGlContext);
//#else
//    assert(false);
//#endif
//
//    std::vector<ChunkRendererType *> addRenderers;
//    std::vector<ChunkRendererType *> updateRenderers;
//    std::vector<ChunkRendererType *> removeRenderers;
//
//    while(m_prepThreadRun)
//    {
//        if(m_prepQueue.empty() && m_removeQueue.empty() && !m_updateChunks)
//        {
//            m_prepEvent.wait(lock);
//            continue;
//        }
//
//        if(!m_removeQueue.empty())
//        {
//            for(ChunkRendererType *renderer:m_removeQueue)
//            {
//                auto chunkIter=m_chunkRendererMap.find(renderer->getKey().hash);
//
//#ifdef LOG_PROCESS_QUEUE
//                LOG(INFO)<<"PrepThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") remove";
//#endif//LOG_PROCESS_QUEUE
//
//                if(chunkIter!=m_chunkRendererMap.end())
//                    m_chunkRendererMap.erase(chunkIter);
//                m_freeChunkRenderers.push_back(renderer);
//            }
//
//            m_removeQueue.clear();
//        }
//
//        if(!m_prepQueue.empty())
//        {
//            ChunkRenderType *chunkRenderer=m_prepQueue.front();
//
//            if(chunkRenderer==nullptr)
//                continue;
//
//            m_prepQueue.pop_front();
//
//            lock.unlock();//drop lock while working
//
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"PrepThread - ChunkRenderer "<<chunkRenderer<<"("<<chunkRenderer->getRegionHash()<<", "<<chunkRenderer->getChunkHash()<<") building mesh";
//#endif//LOG_PROCESS_QUEUE
//
//            bool copyStarted=chunkRenderer->update();
//
//            lock.lock();
//
////cannot release in this thread
////#ifdef LOG_PROCESS_QUEUE
////            LOG(INFO)<<"ChunkHandle ("<<chunkRenderer->getRegionHash()<<", "<<chunkRenderer->getChunkHash()<<") releaseChunkMemory";
////#endif//LOG_PROCESS_QUEUE
////            chunkRenderer->releaseChunkMemory();
//
//            if(copyStarted)
//                m_chunksUpdated.push_back(chunkRenderer);
//
//            m_prepUpdateEvent.notify_all();
//        }
//        else if(m_updateChunks)
//        {
//            lock.unlock();//drop lock while working
//            
//            addRenderers.clear();
//            removeRenderers.clear();
//
//#ifdef LOG_PROCESS_QUEUE
//            LOG(INFO)<<"PrepThread - Updating chunks";
//#endif//LOG_PROCESS_QUEUE
//
//            prepUpdateChunks(addRenderers, updateRenderers, removeRenderers);
//            
//            lock.lock();
//
//            m_addedChunkRenderers.insert(m_addedChunkRenderers.end(), addRenderers.begin(), addRenderers.end());
//            m_updatedChunkRenderers.insert(m_updatedChunkRenderers.end(), updateRenderers.begin(), updateRenderers.end());
//            m_removedChunkRenderers.insert(m_removedChunkRenderers.end(), removeRenderers.begin(), removeRenderers.end());
//        }
//    }
//}

//template<typename _Grid>
//bool SimpleRenderer<_Grid>::updateCallback(SharedChunkHandle chunkHandle)
//{
////    auto regionIter=m_regionRenderers.find(chunkHandle->regionHash());
////
////    if(regionIter!=m_regionRenderers.end())
////    {
////        ChunkRendererMap &chunkMap=regionIter->second.chunkRenderers;
////
////        auto chunkIter=chunkMap.find(chunkHandle->hash());
////
////        if(chunkIter!=chunkMap.end())
////        {
////            ChunkRenderType *chunkRenderer=chunkIter->second;
////            
////            chunkRenderer->updateOutline();
////            if(chunkRenderer->getChunkHandle()->empty())
////                chunkRenderer->setEmpty();
////            else
////            {
//////                chunkRenderer->update();
//////                chunkRenderer->updated();
////                chunkRenderer->buildMesh();
////                chunkRenderer->releaseChunkMemory();
////            }
////        }
////    }
//
//    //make sure we dont have too many builds in flight
//    if(m_prepQueue.size()>10)
//    {
//        std::unique_lock<std::mutex> lock(m_prepMutex);
//
//        while(m_prepQueue.size()>10)
//            m_prepUpdateEvent.wait(lock);
//    }
//
//    return false;
//}

}//namespace voxigen