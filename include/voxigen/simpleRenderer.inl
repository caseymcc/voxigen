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
SimpleRenderer<_Grid>::SimpleRenderer(GridType *grid):
m_grid(grid),
m_viewRadius(128.0f, 128.0f, 128.0f),
m_viewLODDistance(90.0f),
m_lastUpdatePosition(0.0f, 0.0f, 0.0),
m_projectionViewMatUpdated(true),
m_camera(nullptr),
m_queryComplete(true),
m_renderCube(grid, &grid->getDescriptors(), &m_renderPrepThread),
m_regionRenderCube(grid, &grid->getDescriptors(), &m_renderPrepThread),
m_showRegions(true),
m_showChunks(true)
{
    m_forceUpdate=false;
#ifdef NDEBUG
    m_displayOutline=false;
#else
    m_displayOutline=true;
#endif
    m_displayInfo=false;

    m_projectionMat=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f);

    m_viewMat=glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);

    typename _Grid::DescriptorType &descriptors=m_grid->getDescriptors();
}

template<typename _Grid>
SimpleRenderer<_Grid>::~SimpleRenderer()
{

}

template<typename _Grid>
void SimpleRenderer<_Grid>::build()
{
    std::string error;

    gltInit();
    gltColor(1.0f, 1.0f, 1.0f, 1.0f);

    m_cameraInfo=gltCreateText();

    if(!m_program.attachLoadAndCompileShaders(vertShader, fragmentShader, error))
    {
        assert(false);
        return;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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

    RegionRendererType::buildPrograms();

    m_nativeGL.createSharedContext();
    m_renderPrepThread.start(&m_nativeGL);
}

template<typename _Grid>
void SimpleRenderer<_Grid>::destroy()
{
    m_renderPrepThread.stop();
//    m_regionRenderers.clear();
    m_chunkRenderers.clear();

    gltTerminate();
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
    bool cameraDirty=m_camera->isDirty()||m_forceUpdate;

    m_forceUpdate=false;
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

    if(cameraDirty)
    {
        std::ostringstream cameraInfo;

        glm::ivec3 regionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
        const glm::vec3 &cameraPos=m_camera->getPosition();

        glm::ivec3 chunkIndex=m_grid->getChunkIndex(cameraPos);

        cameraInfo<<"Pos: Region:"<<regionIndex.x<<" ,"<<regionIndex.y<<", "<<regionIndex.z<<" Chunk:"<<chunkIndex.x<<", "<<chunkIndex.y<<", "<<chunkIndex.z<<" ("<<cameraPos.x<<" ,"<<cameraPos.y<<" ,"<<cameraPos.z<<")\n";
        cameraInfo<<"Press \"q\" toggle camera/player movement\n";
        cameraInfo<<"Press \"r\" reset camera to player\n";
        cameraInfo<<"Press \"o\" toggle chunk overlay";
        if(m_displayOutline)
            cameraInfo<<"\nPress \"i\" toggle chunk info";

        gltSetText(m_cameraInfo, cameraInfo.str().c_str());
    }
        
    

#ifdef OCCLUSSION_QUERY
    updateOcclusionQueries();
#endif //OCCLUSSION_QUERY

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureAtlasId);

    if(m_showRegions)
    {
        RegionRendererType::useProgram();
        if(cameraDirty)
            RegionRendererType::updateProgramProjection(m_camera->getProjectionViewMat());
        m_regionRenderCube.draw(&m_program, m_offsetId);
    }

    if(m_showChunks)
    {
        if(cameraDirty)
            m_program.uniform(m_uniformProjectionViewId)=m_camera->getProjectionViewMat();
        m_renderCube.draw(&m_program, m_offsetId);
    }


//#ifndef NDEBUG
    //draw Missing blocks in debug
    if(m_displayOutline)
    {
        m_outlineProgram.use();

        if(cameraDirty)
            m_outlineProgram.uniform(m_uniformOutlintProjectionViewId)=m_camera->getProjectionViewMat();

        glDisable(GL_CULL_FACE);
        m_renderCube.drawOutline(&m_outlineProgram, m_outlineOffsetId, m_outlineStatusColor);
        
        if(m_displayInfo)
        {
            m_renderCube.drawInfo(m_camera->getProjectionViewMat());
            m_regionRenderCube.drawInfo(m_camera->getProjectionViewMat());
        }

        RegionRendererType::useOutlineProgram();
        if(cameraDirty)
            RegionRendererType::updateOutlineProgramProjection(m_camera->getProjectionViewMat());
        m_regionRenderCube.drawOutline(&m_outlineProgram, m_outlineOffsetId, m_outlineStatusColor);

        glEnable(GL_CULL_FACE);
    }

    gltDrawText2D(m_cameraInfo, 0.0, 0.0, 1.0f);
//#endif //NDEBUG

}

template<typename _Grid>
void SimpleRenderer<_Grid>::update(bool &regionsUpdated, bool &chunksUpdated)
{
    m_renderCube.update(m_playerIndex);
    m_regionRenderCube.update(RegionIndexType(m_playerIndex.region));

    //force cached item into the queue
    m_grid->updateProcessQueue();

    //update chunks that have been loaded by the grid
    updateChunkHandles(regionsUpdated, chunksUpdated);

    //update render prep chunk
    updatePrepChunks();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updateChunkHandles(bool &regionsUpdated, bool &chunksUpdated)
{
    std::vector<RegionHash> updatedRegions;
    std::vector<Key> updatedChunks;
    
    m_grid->getUpdated(updatedRegions, updatedChunks);

    if(!updatedRegions.empty())
    {
        regionsUpdated=true;
        typename RegionRenderCubeType::RendererType *renderer;
        RegionIndexType index;
        typename _Grid::DescriptorType &descriptors=m_grid->getDescriptors();

        for(size_t i=0; i<updatedRegions.size(); ++i)
        {
            RegionHash &key=updatedRegions[i];

            index.index=descriptors.getRegionIndex(key);

            renderer=m_regionRenderCube.getRenderInfo(index);

            if(renderer==nullptr)
                continue;

            //update finished back to idle
            if(!renderer->getHandle()->empty())
            {
                //Chunk ready and renderer set so lets get a mesh
                renderer->setAction(RenderAction::Meshing);
                m_renderPrepThread.requestMesh<_Grid>(renderer, m_textureAtlas.get());
            }
        }
    }

    if(updatedChunks.empty())
        return;
    
    chunksUpdated=true;
    typename RenderCubeType::RendererType *renderer;
    RegionChunkIndexType index;
    typename _Grid::DescriptorType &descriptors=m_grid->getDescriptors();

    for(size_t i=0; i<updatedChunks.size(); ++i)
    {
        Key &key=updatedChunks[i];

        index.region=descriptors.getRegionIndex(key.regionHash);
        index.chunk=descriptors.getChunkIndex(key.chunkHash);

        renderer=m_renderCube.getRenderInfo(index);

        if(renderer==nullptr)
            continue;

        //update finished back to idle
        if(!renderer->getChunkHandle()->empty())
        {
            //Chunk ready and renderer set so lets get a mesh
            renderer->setAction(RenderAction::Meshing);
            m_renderPrepThread.requestMesh<_Grid>(renderer, m_textureAtlas.get());
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
//        for(RenderPrepThread::Request *request:m_completedRequest)
        for(size_t i=0; i<m_completedRequest.size(); ++i)
        {
            typename RenderPrepThread::Request *request=m_completedRequest[i];

            switch(request->type)
            {
            case prep::Mesh:
                processMesh(request);
                break;
            }
            delete request;
        }
        m_completedRequest.clear();
    }
}

template<typename _Grid>
void SimpleRenderer<_Grid>::processMesh(typename RenderPrepThread::Request *request)
{
    ChunkRequestMesh *chunkMeshRequest=dynamic_cast<ChunkRequestMesh *>(request);

    if(chunkMeshRequest)
    {
        processChunkMesh(chunkMeshRequest);
        return;
    }

    RegionRequestMesh *regionMeshRequest=dynamic_cast<RegionRequestMesh *>(request);

    if(regionMeshRequest)
    {
        processRegionMesh(regionMeshRequest);
        return;
    }
}

template<typename _Grid>
void SimpleRenderer<_Grid>::processChunkMesh(ChunkRequestMesh *request)
{
    ChunkRenderType const *renderer=request->renderer;

    //updated chunks just need to swap out the mesh as that is all that should have been changed
#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") updated";
#endif//LOG_PROCESS_QUEUE

    RegionChunkIndexType index;

    index.region=renderer->getRegionIndex();
    index.chunk=renderer->getChunkIndex();
    
    auto cubeRenderer=m_renderCube.getRenderInfo(index);

    if((cubeRenderer==nullptr) || (cubeRenderer!=renderer))
    {
        //We are in the main thread so we own this
        ChunkRenderType *nonConstRenderer=const_cast<ChunkRenderType *>(renderer);

        //need to let the renderCube know that the renderer is idle again
        nonConstRenderer->setAction(RenderAction::Idle); //renderer is idle again, make sure it can be cleaned up
        return;
    }

    MeshBuffer mesh=cubeRenderer->setMesh(request->mesh);

    if(mesh.valid)
        m_renderPrepThread.requestReleaseMesh(mesh);

    cubeRenderer->setAction(RenderAction::Idle);

    m_grid->releaseChunk(cubeRenderer->getChunkHandle());
}

template<typename _Grid>
void SimpleRenderer<_Grid>::processRegionMesh(RegionRequestMesh *request)
{
    RegionRendererType const *renderer=request->renderer;

    //updated chunks just need to swap out the mesh as that is all that should have been changed
#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - RegionRenderer "<<renderer<<" ("<<renderer->getHash()<<") updated";
#endif//LOG_PROCESS_QUEUE

    RegionIndexType index;

    index.index=renderer->getRegionIndex();

    auto cubeRenderer=m_regionRenderCube.getRenderInfo(index);

    if((cubeRenderer==nullptr)||(cubeRenderer!=renderer))
    {
        //We are in the main thread so we own this
        RegionRendererType *nonConstRenderer=const_cast<RegionRendererType *>(renderer);

        //need to let the renderCube know that the renderer is idle again
        nonConstRenderer->setAction(RenderAction::Idle); //renderer is idle again, make sure it can be cleaned up
        return;
    }

    MeshBuffer mesh=cubeRenderer->setMesh(request->mesh);

    if(mesh.valid)
        m_renderPrepThread.requestReleaseMesh(mesh);

    cubeRenderer->setAction(RenderAction::Idle);

//    m_grid->releaseChunk(cubeRenderer->getChunkHandle());
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setCamera(SimpleFpsCamera *camera)
{
    m_camera=camera;

    m_playerIndex.region=m_grid->getRegionIndex(camera->getRegionHash());
    m_playerIndex.chunk=m_grid->getChunkIndex(m_camera->getPosition());

    if(glm::distance(m_camera->getPosition(), m_lastUpdatePosition)>8.0f)
        m_lastUpdatePosition=m_camera->getPosition();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setCameraChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    RegionChunkIndexType index;

    index.region=regionIndex;
    index.chunk=chunkIndex;
    m_renderCube.updateCamera(index);
    m_regionRenderCube.updateCamera(RegionIndexType(regionIndex));
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setViewRadius(const glm::ivec3 &radius)
{
    m_viewRadius=radius;

    m_renderCube.setViewRadius(radius);
    m_regionRenderCube.setViewRadius(radius*10);
    m_chunkIndices.resize(1);

    m_maxChunkRing=m_chunkIndices.size();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setPlayerChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    m_playerIndex.region=regionIndex;
    m_playerIndex.chunk=chunkIndex;
}

template<typename _Grid>
std::vector<typename SimpleRenderer<_Grid>::ChunkRendererType *> SimpleRenderer<_Grid>::getChunkRenderers()
{
    return m_renderCube.getRenderers();
}

struct ChunkQueryOffset
{
    ChunkQueryOffset(size_t queryRing, glm::vec3 &offset):queryRing(queryRing), offset(offset){}

    size_t queryRing;
    glm::vec3 offset;
};

}//namespace voxigen