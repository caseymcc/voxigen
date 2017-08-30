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
"uniform vec3 segmentOffset;\n"
"\n"
"void main()\n"
"{\n"
"//   gl_Position=vec4(blockOffset.xyz+blockvertex, 1.0);\n"
"//   position=segmentOffset+blockOffset.xyz+blockvertex;\n"
"   vec3 decodedPosition=packedPosition;\n"
"   decodedPosition=decodedPosition;\n"
"   position=segmentOffset+decodedPosition;\n"
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
"uniform vec3 segmentOffset;\n"
"\n"
"void main()\n"
"{\n"
"//   position=inputOffset.xyz+inputVertex;\n"
"   position=segmentOffset+inputOffset.xyz+inputVertex;\n"
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
//    m_lightPositionId=m_program.getUniformId("lightPos");
//    m_lighColorId=m_program.getUniformId("lightColor");
    m_offsetId=m_program.getUniformId("segmentOffset");

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

#ifndef NDEBUG
    m_uniformOutlintProjectionViewId=m_outlineProgram.getUniformId("projectionView");
    m_outlineLightPositionId=m_outlineProgram.getUniformId("lightPos");
    m_outlineOffsetId=m_outlineProgram.getUniformId("segmentOffset");

    const std::vector<float> &outlineVertices=SimpleCube<ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value>::vertCoords;

    glGenBuffers(1, &m_outlineInstanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineInstanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif //NDEBUG

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
    m_segmentRenderers.clear();
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

            auto segmentIter=m_segmentRenderers.find(hash.segmentHash);

            if(segmentIter != m_segmentRenderers.end())
            {
                ChunkRendererMap &chunkMap=segmentIter->second.chunkRenderers;

                auto chunkIter=chunkMap.find(hash.chunkHash);

                if(chunkIter!=chunkMap.end())
                {
                    chunkIter->second->refCount=1; //make sure we hold onto it while it is in the prepThread
                    chunkIter->second->updateOutline();
                    //give to prep thread to get it ready
                    addPrepQueue(chunkIter->second);
                }
            }
        }
    }
    
    if(!m_chunksUpdated.empty())
    {
        std::unique_lock<std::mutex> lock(m_prepMutex);

        for(size_t i=0; i<m_chunksUpdated.size(); ++i)
            m_chunksUpdated[i]->refCount=0; //done in the prepThread

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
//            auto segmentIter=m_segmentRenderers.find(hash.segmentHash);
//
//            if(segmentIter != m_segmentRenderers.end())
//            {
//                ChunkRendererMap &chunkMap=segmentIter->second.chunkRenderers;
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

    //draw all chunks, using segments
    for(auto &iter:m_segmentRenderers)
    {
        SegmentRenderer<ChunkRenderType> &renderer=iter.second;

//        m_program.uniform(m_offsetId)=renderer.offset;

        for(auto &chunkIter:renderer.chunkRenderers)
        {
            ChunkRenderType *chunkRenderer=chunkIter.second;
            
            if(chunkRenderer->getState()==ChunkRenderType::Built) //only set uniform if we are going to draw
                m_program.uniform(m_offsetId)=(renderer.offset+chunkRenderer->getGridOffset());

            chunkRenderer->draw();
        }
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
        
        for(auto &iter:m_segmentRenderers)
        {
            SegmentRenderer<ChunkRenderType> &renderer=iter.second;

            m_outlineProgram.uniform(m_outlineOffsetId)=renderer.offset;

            for(auto &chunkIter:renderer.chunkRenderers)
            {
                ChunkRenderType *chunkRenderer=chunkIter.second;
                
                chunkRenderer->drawOutline();
            }
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
void SimpleRenderer<_Grid>::spiralCube(std::vector<glm::ivec3> &positions, float radius)
{
    glm::ivec3 &chunkSize=m_grid->getDescriptors().m_chunkSize;
    glm::ivec3 chunkRadius=glm::ceil(glm::vec3(radius/chunkSize.x, radius/chunkSize.y, radius/chunkSize.z));

    int maxSteps=std::max(std::max(chunkRadius.x, chunkRadius.y), chunkRadius.z);
    int index;
    glm::ivec3 position(0, 0, 0);
    int z=0;

    for(int j=0; j<maxSteps; j++)
    {
        z=0;
        while(z>=-j && z<=j)
        {
            position.z=z;

            if((z==j)||(z==-j))
            {
                position.x=0;
                position.y=0;
                float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                if(chunkDistance<=m_viewRadius)
                    positions.push_back(position);

                index=1;
            }
            else
                index=j;

            while(index<=j)
            {
                //right side
                position.x=index;
                for(int i=-index; i<index; ++i)
                {
                    position.y=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=m_viewRadius)
                        positions.push_back(position);
                }

                //bottom side
                position.y=index;
                for(int i=index; i>-index; --i)
                {
                    position.x=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=m_viewRadius)
                        positions.push_back(position);
                }

                //left side
                position.x=-index;
                for(int i=index; i>-index; --i)
                {
                    position.y=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=m_viewRadius)
                        positions.push_back(position);
                }

                //top side
                position.y=-index;
                for(int i=-index; i<=index; ++i)
                {
                    position.x=i;
                    float chunkDistance=glm::length(glm::vec3(position*chunkSize));

                    if(chunkDistance<=m_viewRadius)
                        positions.push_back(position);
                }
                index++;
            }

            if(z>0)
                z=-z;
            else
                z=-z+1;
        }
    }
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setViewRadius(float radius)
{
    m_viewRadius=radius;
    m_viewRadiusMax=radius*1.5f;

    m_chunkIndices.clear();
    spiralCube(m_chunkIndices, m_viewRadius);
 //   glm::ivec3 &chunkSize=m_grid->getDescriptors().m_chunkSize;
 //   glm::ivec3 chunkRadius=glm::ceil(glm::vec3(m_viewRadius/chunkSize.x, m_viewRadius/chunkSize.y, m_viewRadius/chunkSize.z));
 //
 //   m_chunkIndices.clear();
 //   glm::vec3 startPos=(chunkRadius*(-chunkSize))+(chunkSize/2);
 //   glm::vec3 chunkPos;
 //
 //   chunkPos.z=startPos.z;
 //   for(int z=-chunkRadius.z; z<chunkRadius.z; ++z)
 //   {
 //       chunkPos.y=startPos.y;
 //       for(int y=-chunkRadius.y; y<chunkRadius.y; ++y)
 //       {
 //           chunkPos.x=startPos.x;
 //           for(int x=-chunkRadius.x; x<chunkRadius.x; ++x)
 //           {
 //               float chunkDistance=glm::length(chunkPos);
 //
 //               if(chunkDistance<=m_viewRadius)
 //               {
 //                   m_chunkIndices.push_back(glm::ivec3(x, y, z));
 //               }
 //               chunkPos.x+=chunkSize.x;
 //           }
 //           chunkPos.y+=chunkSize.y;
 //       }
 //       chunkPos.z+=chunkSize.z;
 //   }

    if(m_chunkIndices.empty()) //always want at least the current chunk
        m_chunkIndices.push_back(glm::ivec3(0, 0, 0));
}

struct SegmentInfo
{
    SegmentInfo(glm::vec3 &offset):offset(offset){}

    glm::vec3 offset;
};

template<typename _Grid>
void SimpleRenderer<_Grid>::updateChunks()
{
    glm::ivec3 playerSegmentIndex=m_grid->getSegmentIndex(m_camera->getSegmentHash());
    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());

    int chunkIndicesSize=m_chunkIndices.size();

//    std::unordered_map<SegmentHash, glm::vec3> segments;
    std::unordered_map<Key::Type, glm::vec3> chunks;
    glm::ivec3 index;
    glm::ivec3 currentSegmentIndex;

    for(size_t i=0; i<chunkIndicesSize; ++i)
    {
//        chunks[i]=m_grid->chunkHash(chunkIndex+m_chunkIndices[i]);
        index=playerChunkIndex+m_chunkIndices[i];
        currentSegmentIndex=playerSegmentIndex;

        glm::vec3 segmentOffset=m_grid->getDescriptors().adjustSegment(currentSegmentIndex, index);
        Key key=m_grid->getHashes(currentSegmentIndex, index);

//        segments.insert(std::pair<SegmentHash, glm::vec3>(key.segmentHash, segmentOffset));
        chunks.insert(std::pair<Key::Type, glm::vec3>(key.hash, segmentOffset));
    }

    //invalidate renderers that pass outside max range
    for(auto segmentIter=m_segmentRenderers.begin(); segmentIter!=m_segmentRenderers.end(); )
    {
        SegmentRendererType &segmentRenderer=segmentIter->second;
        glm::ivec3 segmentOffset=segmentRenderer.index-playerSegmentIndex;

        segmentRenderer.offset=segmentOffset*m_grid->getDescriptors().m_segmentCellSize;

        auto &chunkRendererMap=segmentRenderer.chunkRenderers;

        for(auto chunkIter=chunkRendererMap.begin(); chunkIter!=chunkRendererMap.end(); )
        {
            auto *chunkRenderer=chunkIter->second;
            Key key(chunkRenderer->getSegmentHash(), chunkRenderer->getChunkHash());

            glm::ivec3 segmentIndex=m_grid->getDescriptors().segmentIndex(key.segmentHash);
            glm::ivec3 chunkIndex=m_grid->getDescriptors().chunkIndex(key.chunkHash);
            float chunkDistance=m_grid->getDescriptors().distance(playerSegmentIndex, playerChunkIndex, segmentIndex, chunkIndex);

            //chunk outside of range and invalidate
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

                if(iter != chunks.end())
                    chunks.erase(iter);
            }
            ++chunkIter;
        }

        if(chunkRendererMap.empty())
            segmentIter=m_segmentRenderers.erase(segmentIter);
        else
            ++segmentIter;
    }

    //add missing chunks
    for(auto iter=chunks.begin(); iter!=chunks.end(); ++iter)
    {
        ChunkRenderType *chunkRenderer=getFreeRenderer();

        if(chunkRenderer==nullptr)
            continue;

        Key key(iter->first);
        SharedChunkHandle chunkHandle=m_grid->getChunk(key.segmentHash, key.chunkHash);

        chunkRenderer->setSegmentHash(key.segmentHash);
        chunkRenderer->setChunk(chunkHandle);
        chunkRenderer->setChunkOffset(iter->second);
//        chunkRenderer->update();

        auto segmentIter=m_segmentRenderers.find(key.segmentHash);

        if(segmentIter == m_segmentRenderers.end())
        {
            glm::ivec3 index=m_grid->getSegmentIndex(key.segmentHash);

            auto interResult=m_segmentRenderers.insert(SegmentRendererMap::value_type(key.segmentHash, SegmentRendererType(key.segmentHash, index, iter->second)));

            assert(interResult.second);
            segmentIter=interResult.first;
        }
        segmentIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(key.chunkHash, chunkRenderer));
    }
}

template<typename _Grid>
typename SimpleRenderer<_Grid>::ChunkRenderType *SimpleRenderer<_Grid>::getFreeRenderer()
{
    if(m_freeChunkRenderers.empty())
    {
        size_t minRendererCount=(m_chunkIndices.size()*3)/2;

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
#ifndef NDEBUG
                chunkRenderer->buildOutline(m_outlineInstanceVertices);
#endif //NDEBUG

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