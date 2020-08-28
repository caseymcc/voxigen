#include "voxigen/volume/chunkFunctions.h"
#include "voxigen/search.h"
#include "voxigen/rendering/openglDebug.h"

#include <algorithm>

//#define
namespace voxigen
{

template<typename _Grid>
thread_local ChunkTextureMesh SimpleRenderer<_Grid>::m_threadScratchMesh;

template<typename _Grid>
SimpleRenderer<_Grid>::SimpleRenderer(GridType *grid):
m_grid(grid),
m_viewRadius(128.0f, 128.0f, 128.0f),
m_viewLODDistance(90.0f),
m_lastUpdatePosition(0.0f, 0.0f, 0.0),
m_projectionViewMatUpdated(true),
m_camera(nullptr),
m_queryComplete(true),
//m_activeChunkVolume(grid, &grid->getDescriptors(), 
//    std::bind(&SimpleRenderer<_Grid>::getFreeChunkRenderer, this),
//    std::bind(&SimpleRenderer<_Grid>::releaseChunkRenderer, this, std::placeholders::_1)),
//m_activeRegionVolume(grid, &grid->getDescriptors(),
//    std::bind(&SimpleRenderer<_Grid>::getFreeRegionRenderer, this),
//    std::bind(&SimpleRenderer<_Grid>::releaseRegionRenderer, this, std::placeholders::_1)),
m_activeVolume(grid, &grid->getDescriptors()),
m_showRegions(true),
m_showChunks(true),
//m_chunksLoaded(0),
//m_chunksLoading(0),
//m_chunksMeshing(0),
m_maxMeshUploadCount(5),
m_meshUploading(0)
{
    m_freeChunkRenders.setGrowSize(64);
    m_freeRegionRenders.setGrowSize(64);

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

    getProcessThread().setMeshRequestCallback(std::bind(&SimpleRenderer<_Grid>::buildMesh, this, std::placeholders::_1));
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    const std::vector<float> &vertices=SimpleCube<1, 1, 1>::vertCoords;

    glGenBuffers(1, &m_instanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const std::vector<float> &outlineVertices=SimpleCube<ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value>::vertCoords;

    glGenBuffers(1, &m_outlineInstanceVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineInstanceVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //build texture for textureAtlas
    glGenTextures(1, &m_textureAtlasId);

    loadShaders();

    m_nativeGL.createSharedContext();
//    m_mesherThread.start();
//    m_mesherThread.start(&m_nativeGL, 
//        std::bind(&SimpleRenderer<_Grid>::initializePrepThread, this, std::placeholders::_1),
//        std::bind(&SimpleRenderer<_Grid>::terminatePrepThread, this, std::placeholders::_1));
}

template<typename _Grid>
void SimpleRenderer<_Grid>::loadShaders()
{
    ChunkRendererType::buildPrograms();
    RegionRendererType::buildPrograms();
    m_forceUpdate=true;
}

template<typename _Grid>
std::vector<std::string> SimpleRenderer<_Grid>::getShaderFileNames()
{
    std::vector<std::string> shaderFileNames;
    std::vector<std::string> chunkShaderFileNames=ChunkRendererType::getShaderFileNames();
    std::vector<std::string> regionShaderFileNames=ChunkRendererType::getShaderFileNames();

    shaderFileNames.insert(chunkShaderFileNames.begin(), chunkShaderFileNames.end());
    shaderFileNames.insert(regionShaderFileNames.begin(), regionShaderFileNames.end());
    return shaderFileNames;
}
//template<typename _Grid>
//void SimpleRenderer<_Grid>::initializePrepThread(NativeGL *nativeGL)
//{
//    nativeGL->makeCurrent();
//
//#ifndef NDEBUG
//    GLint majorVersion, minorVersion;
//
//    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
//    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
//
//    if((majorVersion>=4)&&(minorVersion>=3))
//    {
//        glDebugMessageCallback(openglDebugMessage, nullptr);
//        glEnable(GL_DEBUG_OUTPUT);
//    }
//#endif
//}
//
//template<typename _Grid>
//void SimpleRenderer<_Grid>::terminatePrepThread(NativeGL *nativeGL)
//{
//    nativeGL->releaseCurrent();
//}


template<typename _Grid>
void SimpleRenderer<_Grid>::destroy()
{
//    m_mesherThread.stop();
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

//    if(cameraDirty)
//    {
//        std::ostringstream cameraInfo;
//
//        glm::ivec3 regionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//        const glm::vec3 &cameraPos=m_camera->getPosition();
//
//        glm::ivec3 chunkIndex=m_grid->getChunkIndex(cameraPos);
//
//        cameraInfo<<"Pos: Region:"<<regionIndex.x<<" ,"<<regionIndex.y<<", "<<regionIndex.z<<" Chunk:"<<chunkIndex.x<<", "<<chunkIndex.y<<", "<<chunkIndex.z<<" ("<<cameraPos.x<<" ,"<<cameraPos.y<<" ,"<<cameraPos.z<<")\n";
//        cameraInfo<<"Press \"q\" toggle camera/player movement\n";
//        cameraInfo<<"Press \"r\" reset camera to player\n";
//        cameraInfo<<"Press \"o\" toggle chunk overlay";
//        if(m_displayOutline)
//            cameraInfo<<"\nPress \"i\" toggle chunk info";
//
//        gltSetText(m_cameraInfo, cameraInfo.str().c_str());
//    }
        
    
    //complete any outstanding mesh updates
    completeMeshUploads();

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
//        m_activeRegionVolume.draw();
//        drawActiveVolume<0>(m_activeRegionVolume);
    }

    if(m_showChunks)
    {
        ChunkRendererType::useProgram();
        if(cameraDirty)
            ChunkRendererType::updateProgramProjection(m_camera->getProjectionViewMat());
//        m_activeChunkVolume.draw();
//        drawActiveVolume<0>(m_activeChunkVolume);
        drawActiveVolume<Draw::ChunkVolume, Draw::Normal>();
    }


//#ifndef NDEBUG
    //draw Missing blocks in debug
    if(m_displayOutline)
    {
        ChunkRendererType::useOutlineProgram();

        if(cameraDirty)
            ChunkRendererType::updateOutlineProgramProjection(m_camera->getProjectionViewMat());

        glDisable(GL_CULL_FACE);
//        m_activeChunkVolume.drawOutline();
//        drawActiveVolume<2>(m_activeChunkVolume);
        drawActiveVolume<Draw::ChunkVolume, Draw::Outline>();
        
        if(m_displayInfo)
        {
            drawActiveVolume<Draw::ChunkVolume, Draw::Info>();
 //           drawActiveVolume<1>(m_activeChunkVolume);
 //           drawActiveVolume<1>(m_activeRegionVolume);
        }

        RegionRendererType::useOutlineProgram();
        if(cameraDirty)
            RegionRendererType::updateOutlineProgramProjection(m_camera->getProjectionViewMat());
//        m_activeRegionVolume.drawOutline();
//        drawActiveVolume<2>(m_activeRegionVolume);

        glEnable(GL_CULL_FACE);
    }

    gltDrawText2D(m_cameraInfo, 0.0, 0.0, 1.0f);
//#endif //NDEBUG

}

template<typename _Grid>
template<size_t volumeType, size_t _drawType>
void SimpleRenderer<_Grid>::drawActiveVolume()
{
    if(volumeType == 0)
        drawVolume<_drawType>(m_activeVolume.getRegionVolume());
    else
        drawVolume<_drawType>(m_activeVolume.getChunkVolume());
}

template<typename _Grid>
template<size_t _drawType, typename _VolumeType>
void SimpleRenderer<_Grid>::drawVolume(const _VolumeType &volume)
{
//    glm::ivec3 regionIndex=m_activeVolume.getRegionPosition();
    glm::ivec3 regionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());

    for(auto info:volume)
    {
        auto renderer=info.container;

        if(renderer)
        {
//            glm::ivec3 regionOffset=renderer->getRegionIndex()-regionIndex;
            glm::ivec3 regionOffset=details::difference<true, true>(m_grid->getRegionCount(), regionIndex, renderer->getRegionIndex());
            glm::ivec3 offset=regionOffset*renderer->getRegionCellSize();

            if(_drawType == 0)
                renderer->draw(offset);
            else if(_drawType==1)
                renderer->drawInfo(m_camera->getProjectionViewMat(), offset);
            else if(_drawType==2)
                renderer->drawOutline(offset);
        }
    }
}

template<typename _Grid>
void SimpleRenderer<_Grid>::update(bool &regionsUpdated, bool &chunksUpdated)
{
//    MEMORY_CHECK;
//    m_activeChunkVolume.update(m_playerIndex, m_loadChunk, m_releaseChunk);
//    MEMORY_CHECK;
//    m_activeRegionVolume.update(RegionIndexType(m_playerIndex.region), m_loadRegion, m_releaseRegion);

    m_activeVolume.update(m_loadedMeshes, m_releaseMeshes);

    updateMeshes();
    uploadMeshes();

//    LoadRequests failedLoads;


//    for(auto info:m_loadChunk)
//    {
//        auto renderer=info.container;
//        auto handle=renderer->getHandle();
//
//        if(!handle)
//        {
//#ifdef DEBUG_RENDERER_STATUS
//            Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) load from active chunk but no chunk handle", renderer, renderer->getKey().hash);
//#endif
//            continue;
//        }
//
//        if((handle->state() != voxigen::HandleState::Memory) ||
//            (handle->getLod() != info.lod))
//        {
//            //make sure we are not already requesting something from it
//            if(handle->action()==voxigen::HandleAction::Idle)
//            {
//                if(m_grid->loadChunk(handle.get(), info.lod))
//                {
//#ifdef DEBUG_RENDERER_STATUS
//                    Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) load requested", renderer, renderer->getKey().hash);
//#endif
//                    m_chunksLoading++;
//                }
//                else
//                {
//#ifdef DEBUG_RENDERER_STATUS
//                    Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) load request failed ********************", renderer, renderer->getKey().hash);
//#endif
////                    failedLoads.push_back(info);
//                }
//            }
//            else
//            {
//#ifdef DEBUG_RENDERER_STATUS
//                Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) load request but chunk busy **********************", renderer, renderer->getKey().hash);
//#endif
//            }
//        }
//        else
//        {
//#if defined(DEBUG_RENDERER_STATUS) || defined(DEBUG_RENDERER_MESH)
//            Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) load from active chunk, already in memory send to meshing *******************", renderer, renderer->getKey().hash);
//#endif
//            m_meshChunk.push_back(renderer);
//        }
//    }
//    m_loadChunk.clear();
//
//    MEMORY_CHECK
//
//    //push back any chunks that could not be loaded
////    if(!failedLoads.empty())
////        m_loadChunk.insert(m_loadChunk.end(), failedLoads.begin(), failedLoads.end());
//
//
//    for(auto renderer:m_releaseChunk)
//    {
//        if(renderer->getAction()==RenderAction::Meshing)
//        {
//#ifdef DEBUG_RENDERER_STATUS
//            Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) released cancel mesh", renderer, renderer->getKey().hash);
//#endif
//            getProcessThread().cancelChunkMesh(renderer);
////            m_mesherThread.requestCancelMesh(renderer);
//        }
//        
//        auto iter=std::find(m_meshChunk.begin(), m_meshChunk.end(), renderer);
//
//        if(iter!=m_meshChunk.end())
//        {
//#if defined(DEBUG_RENDERER_STATUS) || defined(DEBUG_RENDERER_MESH)
//            Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) released: remove from m_meshChunk", renderer, renderer->getKey().hash);
//#endif
//            m_meshChunk.erase(iter);
//        }
//
//#ifdef DEBUG_RENDERER_STATUS
//        Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) released: removing from active chunk", renderer, renderer->getKey().hash);
//#endif
//        m_activeChunkVolume.releaseInfo(renderer);
//    }
//    m_releaseChunk.clear();
//
//    MEMORY_CHECK
////    for(auto renderer:m_loadRegion)
////    {
////        auto handle=renderer->getHandle();
////
////        if(handle->state()!=voxigen::HandleState::Memory)
////        {
////            //make sure we are not already requesting something from it
////            if(handle->action()==voxigen::HandleAction::Idle)
////                m_grid->loadRegion(handle, 0, false);
////        }
////        else
////        {
////            if(handle->action()==voxigen::HandleAction::Idle)
////            {
////                renderer->setAction(RenderAction::Meshing);
////                m_mesherThread.requestMesh<_Grid>(renderer, m_textureAtlas.get());
////            }
////        }
////    }
////    m_loadRegion.clear();
//
//    for(auto renderer:m_releaseRegion)
//    {
//        m_activeRegionVolume.releaseInfo(renderer);
//    }
//    m_releaseRegion.clear();
//
//    MEMORY_CHECK
//    //force cached item into the queue
////    m_grid->updateProcessQueue();
//
//    //update chunks that have been loaded by the grid
//    updateChunkHandles(regionsUpdated, chunksUpdated);
//
//    MEMORY_CHECK
//
//    //update render prep chunk
//    updatePrepChunks();
//
//    //send chunks off the need to be meshed
//    for(auto iter=m_meshChunk.begin(); iter!=m_meshChunk.end(); ++iter)
//    {
//        auto renderer=*iter;
//        auto handle=renderer->getHandle();
//
//        if(!handle)
//        {
//            //chunk has been released so meshing no longer needed
////            iter=m_meshChunk.erase(iter);
////            (*iter)=nullptr;
//#if defined(DEBUG_RENDERER_STATUS) || defined(DEBUG_RENDERER_MESH)
//            Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) mesh request but handle invalid", renderer, renderer->getKey().hash);
//#endif
//            continue;
//        }
//
//        if((renderer->getAction()==RenderAction::Idle)&&(!handle->empty()))
//        {
////            ChunkTextureMesh *mesh=m_meshHeap.get();
////
////            if(!mesh)
////                break;//no mesh available 
//
////            if(m_mesherThread.requestMesh(renderer, m_textureAtlas.get()))
//#if defined(DEBUG_RENDERER_STATUS) || defined(DEBUG_RENDERER_MESH) || defined(DEBUG_MESH)
//            Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) request mesh", renderer, renderer->getKey().hash);
//#endif
//            getProcessThread().requestChunkMesh(renderer);
//            {
//                m_chunksMeshing++;
//                handle->addInUse();
//                renderer->setAction(RenderAction::Meshing);
////                iter=m_meshChunk.erase(iter);
//                (*iter)=nullptr;
//                continue;
//            }
////            else
////                break; //failed to add
//        }
//        else
//        {
//#if defined(DEBUG_RENDERER_STATUS) || defined(DEBUG_RENDERER_MESH) || defined(DEBUG_MESH)
//            Log::debug("SimpleRenderer::update - Chunk renderer(%llx, %llx) request mesh but chunk is busy", renderer, renderer->getKey().hash);
//#endif
////            assert(false);
//        }
////        else
////            ++iter;
//    }
//    
////    //move all non nullptrs to front, erase nullptrs on end
////    m_meshChunk.erase(std::remove_if(m_meshChunk.begin(), m_meshChunk.end(), [](ChunkRendererType *renderer){return renderer==nullptr; }), m_meshChunk.end());
//    m_meshChunk.clear();
//
//    MEMORY_CHECK
}

template<typename _Grid>
void SimpleRenderer<_Grid>::updateMeshes()
{
//    m_loadedMeshes, m_releaseMeshes;
    bool removeAll=true;

    for(size_t i=0; i<m_loadedMeshes.size(); ++i)
    {
        MeshUpdate &update=m_loadedMeshes[i];

        if(processChunkMesh(update))
            update.container=nullptr;
        else
            removeAll=false;
    }

    if(removeAll)
        m_loadedMeshes.clear();
    else
        m_loadedMeshes.erase(std::remove_if(m_loadedMeshes.begin(), m_loadedMeshes.end(), [](MeshUpdate &update){return (update.container==nullptr);}), m_loadedMeshes.end());
}

template<typename _Grid>
bool SimpleRenderer<_Grid>::processChunkMesh(MeshUpdate &update)
{
    ChunkRenderer*renderer=(ChunkRenderer *)update.container;

    //    renderer->decrementMesh();
        //updated chunks just need to swap out the mesh as that is all that should have been changed
#ifdef DEBUG_MESH
    glm::ivec3 regionIndex=renderer->getRegionIndex();
    glm::ivec3 chunkIndex=renderer->getChunkIndex();

    Log::debug("SimpleRenderer::processChunkMesh - ChunkRenderer %llx,%llx (%d, %d, %d) (%d, %d, %d) mesh complete", renderer,
        renderer->getHandle().get(),
        regionIndex.x, regionIndex.y, regionIndex.z,
        chunkIndex.x, chunkIndex.y, chunkIndex.z);
#endif//DEBUG_MESH

    SharedChunkHandle chunkHandle=renderer->getChunkHandle();

//    chunkHandle->removeInUse();
    if(!chunkHandle->inUse())
    {
#ifdef DEBUG_RENDERER_STATUS
        Log::debug("SimpleRenderer::processChunkMesh - Chunk renderer(%llx, %llx) mesh complete and being released", renderer, renderer->getKey().hash);
#endif
        m_grid->releaseChunk(renderer->getChunkHandle().get());
    }
    else
    {
#ifdef DEBUG_RENDERER_STATUS
        Log::debug("SimpleRenderer::processChunkMesh - Chunk renderer(%llx, %llx) mesh complete but still in use*********", renderer, renderer->getKey().hash);
#endif
    }
    
    m_meshUploadQueue.push_back(update);
//    uploadMesh(update);
    return true;
}

template<typename _Grid>
void SimpleRenderer<_Grid>::uploadMeshes()
{
//    size_t count=std::min(m_meshUploadQueue.size(), m_maxMeshUploadCount);
    size_t count=m_meshUploadQueue.size();

    //only send 5 at time
    for(size_t i=0; i<count; ++i)
        uploadMesh(m_meshUploadQueue[i]);

    if(count > 0)
        m_meshUploadQueue.erase(m_meshUploadQueue.begin(), m_meshUploadQueue.begin()+count);
}

template<typename _Grid>
void SimpleRenderer<_Grid>::uploadMesh(MeshUpdate &update)
{
    ChunkRenderer *renderer=(ChunkRenderer *)update.container;
    ChunkTextureMesh *mesh=update.mesh;

//    MeshRequestInfo info;
//    MeshUpload meshUpload;
//
//    meshUpload.renderer=renderer;
//    meshUpload.mesh=mesh;

    renderer->setMeshState(MeshState::Uploading);

    m_meshUploads.emplace_back(renderer, mesh);
    MeshUpload &meshUpload=m_meshUploads.back();
    MeshBuffer &meshBuffer=meshUpload.meshBuffer;

    gl::glGenBuffers(1, &meshBuffer.vertexBuffer);
    gl::glGenBuffers(1, &meshBuffer.indexBuffer);

    auto &verticies=mesh->getVertexes();
    std::vector<int> &indices=mesh->getIndexes();

    meshBuffer.frame=0;
    meshBuffer.ready=false;
    meshBuffer.indexType=(unsigned int)gl::GL_UNSIGNED_INT;

#ifdef DEBUG_MESH
    glm::ivec3 regionIndex=renderer->getRegionIndex();
    glm::ivec3 chunkIndex=renderer->getChunkIndex();

    Log::debug("MainThread - ChunkRenderer %llx,%llx (%d, %d, %d) (%d, %d, %d) mesh start upload", renderer,
        renderer->getHandle().get(),
        regionIndex.x, regionIndex.y, regionIndex.z,
        chunkIndex.x, chunkIndex.y, chunkIndex.z);
#endif//DEBUG_MESH

    if(!indices.empty())
    {
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, meshBuffer.vertexBuffer);
        gl::glBufferData(gl::GL_ARRAY_BUFFER, verticies.size()*sizeof(ChunkTextureMesh::Vertex), verticies.data(), gl::GL_STATIC_DRAW);

        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, meshBuffer.indexBuffer);
        gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), gl::GL_STATIC_DRAW);

        meshBuffer.sync=gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, gl::UnusedMask::GL_NONE_BIT);

        meshBuffer.valid=true;
        meshBuffer.indices=indices.size();

//        info.request=request;
//        m_meshUpdate.push_back(info);
//          m_meshUploads.emplace_back(renderer, mesh, meshbuffer)

        m_meshUploading++;
    }
    else
    {
#ifdef DEBUG_MESH
        Log::debug("MainThread - Mesh upload %llx - indices empty ", update.mesh);
#endif//DEBUG_MESH
        //nothing to upload likely canceled, dump it
//        m_releaseMeshes.emplace_back(update.container, update.mesh);
        m_meshUploads.pop_back();
    }
}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::updateChunkHandles(bool &regionsUpdated, bool &chunksUpdated)
//{
//    std::vector<RegionHash> updatedRegions;
//    std::vector<Key> updatedChunks;
//    RequestQueue completedRequests;
//    
//    m_grid->getUpdated(updatedRegions, updatedChunks, completedRequests);
//
//    if(!updatedRegions.empty())
//    {
//        regionsUpdated=true;
//        typename RegionActiveVolumeType::ContainerType *renderer;
//        RegionIndexType index;
//        typename _Grid::DescriptorType &descriptors=m_grid->getDescriptors();
//
//        for(size_t i=0; i<updatedRegions.size(); ++i)
//        {
//            RegionHash &key=updatedRegions[i];
//
//            index.index=descriptors.getRegionIndex(key);
//
//            renderer=m_activeRegionVolume.getRenderInfo(index);
//
//            if(renderer==nullptr)
//                continue;
//
//            //update finished back to idle
//            if(!renderer->getHandle()->empty())
//            {
//                //Chunk ready and renderer set so lets get a mesh
////                renderer->setAction(RenderAction::Meshing);
////                m_mesherThread.requestMesh(renderer, m_textureAtlas.get());
//            }
//        }
//    }
//
//    if(!updatedChunks.empty())
//    {
//        chunksUpdated=true;
//        typename ActiveVolumeType::ContainerType *renderer;
//        RegionChunkIndexType index;
//        typename _Grid::DescriptorType &descriptors=m_grid->getDescriptors();
//
//        for(size_t i=0; i<updatedChunks.size(); ++i)
//        {
//            m_chunksLoading--;
//
//            Key &key=updatedChunks[i];
//
//            index.region=descriptors.getRegionIndex(key.regionHash);
//            index.chunk=descriptors.getChunkIndex(key.chunkHash);
//
//            renderer=m_activeChunkVolume.getRenderInfo(index);
//
//            if(renderer==nullptr)
//            {
//#ifdef DEBUG_RENDERER_STATUS
//                Log::debug("SimpleRenderer::updateChunkHandles - Chunk renderer(%llx, %llx) load complete but renderer invalid", renderer, key.hash);
//#endif
//                continue;
//            }
//
//            SharedChunkHandle chunkHandle=renderer->getChunkHandle();
//
//            if(!chunkHandle->empty())
//            {
//#if defined(DEBUG_RENDERER_STATUS) || defined(DEBUG_RENDERER_MESH)
//                Log::debug("SimpleRenderer::updateChunkHandles - Chunk renderer(%llx, %llx) load complete adding to mesh request", renderer, key.hash);
//#endif
//
//                m_meshChunk.push_back(renderer);
//            }
//            else
//            {
//#ifdef DEBUG_RENDERER_STATUS
//                Log::debug("SimpleRenderer::updateChunkHandles - Chunk renderer(%llx, %llx) load complete but handle invalid", renderer, key.hash);
//#endif
//
//            }
//        }
//    }
//
//    for(size_t i=0; i<completedRequests.size(); ++i)
//    {
//        process::Request *request=completedRequests[i];
//
//        switch(request->type)
//        {
////        case prep::Mesh:
//        case process::Type::Mesh:
//            {
//#ifdef DEBUG_MESH
//            Log::debug("SimpleRenderer process chunk mesh %llx", request);
//#endif
//                processChunkMesh(request);
//            }
//            break;
//        default:
//#ifdef DEBUG_MESH
//            Log::debug("SimpleRenderer release request %llx", request);
//#endif
//#ifdef DEBUG_REQUESTS
//            Log::debug("updateChunkHandles release request %llx", request);
//#endif
//            getProcessThread().releaseRequest(request);
//            break;
//        }
//    }
//    completedRequests.clear();
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::updatePrepChunks()
//{
//    //TODO: need to add limits on how many to process in a single update
//
//    //m_mesherThread.updateQueues(m_completedRequest);
//    getProcessThread().updateQueues(m_completedRequest);
//
//    if(!m_completedRequest.empty())
//    {
////        for(RenderPrepThread::Request *request:m_completedRequest)
//        for(size_t i=0; i<m_completedRequest.size(); ++i)
//        {
//            process::Request *request=m_completedRequest[i];
//
//            switch(request->type)
//            {
//            case prep::Mesh:
//                {
//                    processChunkMesh(request);
//                }
//                break;
//            }
//        }
//        m_completedRequest.clear();
//    }
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::processChunkMesh(process::Request *request)//ChunkRequestMesh *request)
//{
//    ChunkRenderType *renderer=(ChunkRenderType *)request->data.buildMesh.renderer;
//
////    renderer->decrementMesh();
//    //updated chunks just need to swap out the mesh as that is all that should have been changed
//#ifdef DEBUG_MESH
//    glm::ivec3 regionIndex=renderer->getRegionIndex();
//    glm::ivec3 chunkIndex=renderer->getChunkIndex();
//
//    Log::debug("MainThread - ChunkRenderer %llx,%llx,%llx (%d, %d, %d) (%d, %d, %d) mesh complete", renderer,
//        renderer->getHandle().get(),
//        request,
//        regionIndex.x, regionIndex.y, regionIndex.z,
//        chunkIndex.x, chunkIndex.y, chunkIndex.z);
//#endif//DEBUG_MESH
//
//    RegionChunkIndexType index;
//
//    index.region=renderer->getRegionIndex();
//    index.chunk=renderer->getChunkIndex();
//    
//    auto cubeRenderer=m_activeChunkVolume.getRenderInfo(index);
//
//    if((cubeRenderer==nullptr) || (cubeRenderer!=renderer))
//    {
//        //We are in the main thread so we own this
//        ChunkRenderType *nonConstRenderer=const_cast<ChunkRenderType *>(renderer);
//
//#ifdef DEBUG_RENDERER_STATUS
//        Log::debug("SimpleRenderer::processChunkMesh - Chunk renderer(%llx, %llx) mesh complete but renderer no longer active*********", renderer, renderer->getKey().hash);
//#endif
//
//        //need to let the renderCube know that the renderer is idle again
//        nonConstRenderer->setAction(RenderAction::Idle); //renderer is idle again, make sure it can be cleaned up
//#ifdef DEBUG_MESH
//        Log::debug("MainThread - Mesh complete - renderer invalid %llx, %llx", cubeRenderer, request);
//#endif//DEBUG_MESH
////        m_mesherThread.returnMesh(request->getMesh());
////        m_mesherThread.returnRequest(request);
////        m_mesherThread.returnMeshRequest(request);
//        getProcessThread().returnMesh(renderer, request->data.buildMesh.mesh);
//#ifdef DEBUG_REQUESTS
//        Log::debug("processChunkMesh release request %llx", request);
//#endif
//        getProcessThread().releaseRequest(request);
//        m_chunksMeshing--;
//        return;
//    }
//
//    SharedChunkHandle chunkHandle=cubeRenderer->getChunkHandle();
//
//    chunkHandle->removeInUse();
//    if(!chunkHandle->inUse())
//    {
//#ifdef DEBUG_RENDERER_STATUS
//        Log::debug("SimpleRenderer::processChunkMesh - Chunk renderer(%llx, %llx) mesh complete and being released", renderer, renderer->getKey().hash);
//#endif
//        m_grid->releaseChunk(cubeRenderer->getChunkHandle().get());
//    }
//    else
//    {
//#ifdef DEBUG_RENDERER_STATUS
//        Log::debug("SimpleRenderer::processChunkMesh - Chunk renderer(%llx, %llx) mesh complete but still in use*********", renderer, renderer->getKey().hash);
//#endif
//    }
//    uploadMesh(request);
//
////    MeshBuffer mesh=cubeRenderer->setMesh(request->mesh);
////
////    if(mesh.valid)
////        m_mesherThread.returnMesh(mesh);
//
////    cubeRenderer->setAction(RenderAction::Idle);
//    
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::uploadMesh(process::Request *request)
//{
//    ChunkRendererType *renderer=(ChunkRendererType *)request->data.buildMesh.renderer;
//    ChunkTextureMesh *mesh=request->data.buildMesh.mesh;
//    MeshRequestInfo info;
//    MeshBuffer &meshBuffer=info.meshBuffer;
//
//    gl::glGenBuffers(1, &meshBuffer.vertexBuffer);
//    gl::glGenBuffers(1, &meshBuffer.indexBuffer);
//
//    auto &verticies=mesh->getVertexes();
//    std::vector<int> &indices=mesh->getIndexes();
//
//    meshBuffer.frame=0;
//    meshBuffer.ready=false;
//    meshBuffer.indexType=(unsigned int)gl::GL_UNSIGNED_INT;
//
//#ifdef DEBUG_MESH
//    glm::ivec3 regionIndex=renderer->getRegionIndex();
//    glm::ivec3 chunkIndex=renderer->getChunkIndex();
//
//    Log::debug("MainThread - ChunkRenderer %llx,%llx,%llx (%d, %d, %d) (%d, %d, %d) mesh start upload", renderer,
//        renderer->getHandle().get(), request,
//        regionIndex.x, regionIndex.y, regionIndex.z,
//        chunkIndex.x, chunkIndex.y, chunkIndex.z);
//#endif//DEBUG_MESH
//    
//
//    if(!indices.empty())
//    {
//        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, meshBuffer.vertexBuffer);
//        gl::glBufferData(gl::GL_ARRAY_BUFFER, verticies.size()*sizeof(ChunkTextureMesh::Vertex), verticies.data(), gl::GL_STATIC_DRAW);
//
//        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, meshBuffer.indexBuffer);
//        gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), gl::GL_STATIC_DRAW);
//
//        meshBuffer.sync=gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, gl::UnusedMask::GL_NONE_BIT);
//
//        meshBuffer.valid=true;
//        meshBuffer.indices=indices.size();
//
//        info.request=request;
//        m_meshUpdate.push_back(info);
//
//        m_meshUploading++;
//    }
//    else
//    {
//#ifdef DEBUG_MESH
//        Log::debug("MainThread - Mesh upload %llx, %llx - indices empty ", request->data.buildMesh.mesh, request);
//#endif//DEBUG_MESH
//        //nothing to upload likely canceled, dump it
////        m_mesherThread.returnMeshRequest(request);
////        m_meshHeap.release(request->data.buildMesh.mesh);
//        getProcessThread().returnMesh(renderer, request->data.buildMesh.mesh);
//#ifdef DEBUG_REQUESTS
//        Log::debug("uploadMesh release request %llx", request);
//#endif
//        getProcessThread().releaseRequest(request);
//        m_chunksMeshing--;
//    }
//}

template<typename _Grid>
void SimpleRenderer<_Grid>::completeMeshUploads()
{
//    for(auto iter=m_meshUpdate.begin(); iter!=m_meshUpdate.end(); )
    for(size_t i=0; i<m_meshUploads.size(); )
    {
        bool remove=false;
        MeshUpload &update=m_meshUploads[i];
        ChunkRenderer *renderer=update.renderer;
        MeshBuffer &meshBuffer=update.meshBuffer;

        if(meshBuffer.frame == 0)
        {
            gl::GLenum result=gl::glClientWaitSync(meshBuffer.sync, gl::SyncObjectMask::GL_NONE_BIT, 0);

            if((result == gl::GL_ALREADY_SIGNALED) || (result == gl::GL_CONDITION_SATISFIED))
            {
                meshBuffer.frame=1;
                gl::glDeleteSync(meshBuffer.sync);
                meshBuffer.sync=nullptr;
            }
            else if(result==gl::GL_WAIT_FAILED)
                assert(false);
        }
        else
        {
            meshBuffer.frame++;
            if(meshBuffer.frame>3)
            {
//                MesherThread::Request *request=iter->request;
                meshBuffer.ready=true;
                
#ifdef DEBUG_MESH
                glm::ivec3 regionIndex=renderer->getRegionIndex();
                glm::ivec3 chunkIndex=renderer->getChunkIndex();

                Log::debug("MainThread - ChunkRenderer %llx, %llx - mesh upload complete", renderer,
                    renderer->getHandle().get());
#endif//DEBUG_MESH

                MeshBuffer prevMesh=renderer->setMesh(meshBuffer);
                renderer->setAction(RenderAction::Idle);
                renderer->setMeshState(MeshState::Ready);

                //TODO: re-use buffers
                if(prevMesh.valid)
                {
                    gl::glDeleteBuffers(1, &prevMesh.vertexBuffer);
                    gl::glDeleteBuffers(1, &prevMesh.indexBuffer); 
                }

//                m_mesherThread.returnMesh(request->getMesh());
//                m_mesherThread.returnRequest(request);
//                m_mesherThread.returnMeshRequest(request);
//                m_meshHeap.release(request->data.buildMesh.mesh);
//                getProcessThread().returnMesh(renderer, request->data.buildMesh.mesh);
               
                m_meshUploading--;
                remove=true;
            }
        }

        if(remove)
        {
            m_releaseMeshes.emplace_back(update.renderer, update.mesh);
            m_meshUploads[i]=m_meshUploads.back();
            m_meshUploads.pop_back();
        }
        else
            ++i;
    }
}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::processRegionMesh(RegionRequestMesh *request)
//{
//    RegionRendererType const *renderer=request->renderer;
//
//    //updated chunks just need to swap out the mesh as that is all that should have been changed
//#ifdef LOG_PROCESS_QUEUE
//    Log::debug("MainThread - RegionRenderer %llx (%d) updated", renderer, renderer->getHash());
//#endif//LOG_PROCESS_QUEUE
//
//    RegionIndexType index;
//
//    index.index=renderer->getRegionIndex();
//
//    auto cubeRenderer=m_activeRegionVolume.getRenderInfo(index);
//
//    if((cubeRenderer==nullptr)||(cubeRenderer!=renderer))
//    {
//        //We are in the main thread so we own this
//        RegionRendererType *nonConstRenderer=const_cast<RegionRendererType *>(renderer);
//
//        //need to let the renderCube know that the renderer is idle again
//        nonConstRenderer->setAction(RenderAction::Idle); //renderer is idle again, make sure it can be cleaned up
//        return;
//    }
//
//    MeshBuffer mesh=cubeRenderer->setMesh(request->mesh);
//
//    if(mesh.valid)
//        m_mesherThread.requestReleaseMesh(mesh);
//
//    cubeRenderer->setAction(RenderAction::Idle);
//
////    m_grid->releaseChunk(cubeRenderer->getChunkHandle());
//}

template<typename _Grid>
void SimpleRenderer<_Grid>::setCamera(SimpleFpsCamera *camera)
{
    m_camera=camera;

    m_playerRegion=m_grid->getRegionIndex(camera->getRegionHash());
    m_playerChunk=m_grid->getChunkIndex(m_camera->getPosition());

    if(glm::distance(m_camera->getPosition(), m_lastUpdatePosition)>8.0f)
        m_lastUpdatePosition=m_camera->getPosition();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setCameraChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
//    m_playerRegion=regionIndex;
//    m_playerChunk=chunkIndex;
//
//    RegionChunkIndexType index;
//
//    index.region=regionIndex;
//    index.chunk=chunkIndex;
//    m_activeChunkVolume.updateCamera(index);
//    m_activeRegionVolume.updateCamera(RegionIndexType(regionIndex));

//    m_mesherThread.requestPositionUpdate(regionIndex, chunkIndex);
//    getProcessThread().updatePosition(regionIndex, chunkIndex);
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setViewRadius(const glm::ivec3 &radius)
{
    m_viewRadius=radius;

    m_activeVolume.setViewRadius(radius);
//    m_activeChunkVolume.setViewRadius(radius);
//    m_activeRegionVolume.setViewRadius(radius*10);
    
//    //set maximum request size to number of containers
//    m_grid->setChunkRequestSize(m_activeChunkVolume.getContainerCount());

//    size_t chunkContainerCount=m_activeChunkVolume.getContainerCount();
//    m_freeChunkRenders.setMaxSize((chunkContainerCount/2)*3);
//
//    size_t regionContainerCount=m_activeRegionVolume.getContainerCount();
//    m_freeRegionRenders.setMaxSize((regionContainerCount/2)*3);

    m_chunkIndices.resize(1);
    m_maxChunkRing=m_chunkIndices.size();
}

template<typename _Grid>
size_t SimpleRenderer<_Grid>::getRendererCount() 
{
//    return m_activeChunkVolume.getContainerCount();
    return m_activeVolume.getChunkContainerCount();
}

template<typename _Grid>
void SimpleRenderer<_Grid>::setPlayerChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    m_playerRegion=regionIndex;
    m_playerChunk=chunkIndex;

//    m_mesherThread.requestPositionUpdate(regionIndex, chunkIndex);
    getProcessThread().updatePosition(regionIndex, chunkIndex);
    m_activeVolume.updatePosition(regionIndex, chunkIndex);
}

//template<typename _Grid>
//typename SimpleRenderer<_Grid>::ChunkRendererType *SimpleRenderer<_Grid>::getFreeChunkRenderer()
//{
//    ChunkRendererType *renderer=m_freeChunkRenders.get();
//
//    if(renderer)
//    {
//        m_chunksLoaded++;
//        renderer->build();
//    }
//    return renderer;
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::releaseChunkRenderer(ChunkRendererType *renderer)
//{
//    MeshBuffer prevMesh=renderer->clearMesh();
//
//    if(prevMesh.valid)
//    {
//        gl::glDeleteBuffers(1, &prevMesh.vertexBuffer);
//        gl::glDeleteBuffers(1, &prevMesh.indexBuffer);
//    }
//
//    m_chunksLoaded--;
//    m_freeChunkRenders.release(renderer);
//}

//template<typename _Grid>
//typename SimpleRenderer<_Grid>::RegionRendererType *SimpleRenderer<_Grid>::getFreeRegionRenderer()
//{
//    RegionRendererType *renderer=m_freeRegionRenders.get();
//
//    if(renderer)
//        renderer->build();
//
//    return renderer;
//}

//template<typename _Grid>
//void SimpleRenderer<_Grid>::releaseRegionRenderer(RegionRendererType *renderer)
//{
//    m_freeRegionRenders.release(renderer);
//}

//template<typename _Grid>
//typename SimpleRenderer<_Grid>::ActiveVolumeType::VolumeInfo &SimpleRenderer<_Grid>::getVolumeInfo()
//{
//    return m_activeChunkVolume.getVolume();
//}

struct ChunkQueryOffset
{
    ChunkQueryOffset(size_t queryRing, glm::vec3 &offset):queryRing(queryRing), offset(offset){}

    size_t queryRing;
    glm::vec3 offset;
};

template<typename _Grid>
bool SimpleRenderer<_Grid>::buildMesh(process::Request *request)
{
    MEMORY_CHECK

    ChunkRendererType *renderer=(ChunkRendererType *)request->data.buildMesh.renderer;
    ChunkTextureMesh *mesh=(ChunkTextureMesh *)request->data.buildMesh.mesh;

    assert(mesh);


    //    Request::ObjectMesh &objectMesh=request->getObjectMesh();
    //    _Object *object=objectMesh.object;

#ifdef DEBUG_MESH
    glm::ivec3 regionIndex=renderer->getRegionIndex();
    glm::ivec3 chunkIndex=renderer->getChunkIndex();

    Log::debug("RenderPrepThread - RenderPrepThread %llx (%d, %d, %d) (%d, %d, %d) meshing (%llx)", renderer,
        regionIndex.x, regionIndex.y, regionIndex.z,
        chunkIndex.x, chunkIndex.y, chunkIndex.z,
        request);
#endif//DEBUG_MESH

    mesh->clear();
    mesh->setTextureAtlas(m_textureAtlas.get());

    m_threadScratchMesh.clear();
    m_threadScratchMesh.setTextureAtlas(m_textureAtlas.get());

    auto chunk=renderer->getHandle()->chunk();

#ifdef DEBUG_ALLOCATION
    Log::debug("SimpleRenderer::buildMesh renderer:%llx hash:(%d, %d) accessing chunk data:%llx", this, renderer->getRegionHash(), renderer->getChunkHash(), chunk);
#endif

    //    if(chunk->hasNeighbors())
    //        voxigen::buildCubicMesh_Neighbor(*scratchMesh, chunk, chunk->getNeighbors());
    //    else
    MEMORY_CHECK

    voxigen::buildCubicMesh(m_threadScratchMesh, chunk);

    MEMORY_CHECK

#ifdef DEBUG_ALLOCATION
    Log::debug("SimpleRenderer::buildMesh renderer:%llx hash:(%d, %d) access complete:%llx", this, renderer->getRegionHash(), renderer->getChunkHash(), chunk);
#endif

    std::vector<ChunkTextureMesh::Vertex> &vertexes=m_threadScratchMesh.getVertexes();
    std::vector<int> &indexes=m_threadScratchMesh.getIndexes();

    //copy from scratch to allocated mesh
    std::vector<ChunkTextureMesh::Vertex> &meshVertexes=mesh->getVertexes();
    std::vector<int> &meshIndexes=mesh->getIndexes();

    meshVertexes.resize(vertexes.size());
    memcpy(meshVertexes.data(), vertexes.data(), vertexes.size()*sizeof(ChunkTextureMesh::Vertex));
    meshIndexes.resize(indexes.size());
    memcpy(meshIndexes.data(), indexes.data(), indexes.size()*sizeof(int));

    MEMORY_CHECK

//    request->data.objectMesh.mesh=mesh;
    return true;
}


}//namespace voxigen