#include "voxigen/volume/chunkFunctions.h"

namespace voxigen
{

template<typename _Container>
glm::ivec3 calcVolumeSize(const glm::ivec3 &radius)
{
    glm::ivec3 volumeSize;
    glm::ivec3 renderSize=_Container::getSize();

    volumeSize=(radius)/renderSize;

    if(volumeSize.x<=1)
        volumeSize.x++;
    if(volumeSize.y<=1)
        volumeSize.y++;
    if(volumeSize.z<=1)
        volumeSize.z++;

    //make everything odd
    if(volumeSize.x%2==0)
        volumeSize.x++;
    if(volumeSize.y%2==0)
        volumeSize.y++;
    if(volumeSize.z%2==0)
        volumeSize.z++;

    return volumeSize;
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::ActiveVolume(Grid *grid, Descriptor *descriptors):
m_grid(grid),
m_descriptors(descriptors),
m_regionVolume(grid, descriptors, 
    std::bind(&ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::initRegionVolumeInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 
    std::bind(&ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::getRegionContainer, this),
    std::bind(&ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::releaseRegionContainer, this, std::placeholders::_1)),
m_chunkVolume(grid, descriptors, 
    std::bind(&ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::initChunkVolumeInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
    std::bind(&ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::getChunkContainer, this), 
    std::bind(&ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::releaseChunkContainer, this, std::placeholders::_1)),
m_loadedChunks(0),
m_loadingChunks(0),
m_meshingChunks(0)
{
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::~ActiveVolume()
{

}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::setViewRadius(const glm::ivec3 &radius)
{
    m_regionVolume.setViewRadius(radius*10);// , m_regionLoadRequests, m_regionReleases);
    m_chunkVolume.setViewRadius(radius);// , m_chunkLoadRequests, m_chunkUpdates);
    
    m_chunkContainers.setMaxSize((m_chunkVolume.getContainerCount()*3)/2);
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::init(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    m_regionIndex.index=regionIndex;
    m_chunkIndex.region=regionIndex;
    m_chunkIndex.chunk=chunkIndex;

//    m_reigonVolume.init(m_regionIndex);
//    m_chunkVolume.init(m_chunkIndex);
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
_RegionContainer *ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::getRegionContainer()
{
    return m_regionContainers.get();
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::releaseRegionContainer(RegionContainer *container)
{
    m_regionContainers.release(container);
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
_ChunkContainer *ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::getChunkContainer()
{
    _ChunkContainer *container=m_chunkContainers.get();

    if(container)
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::getChunkContainer - Chunk container(%llx) get - %s", container, container->getActionString().c_str());
#endif
        container->build();
    }
    else
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::getChunkContainer - failed to get container");
#endif
    }
    return container;
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::releaseChunkContainer(ChunkContainer *container)
{
    RenderAction action=container->getAction();

//    if(action==RenderAction::Idle)
//    {
//#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
//        Log::debug("ActiveVolume::releaseChunkContainer - Chunk container(%llx, %llx) release - %s", container, container->getKey().hash, container->getActionString().c_str());
//#endif
//        container->release();
//        m_chunkContainers.release(container);
//    }
//    else
    if(action!=RenderAction::Idle)
    {
        //need to cancel whatever the container is doing
        if(action == RenderAction::HandleBusy)
        {
            SharedChunkHandle handle=container->getHandle();
            HandleAction handleAction=handle->getAction();

            if(handleAction == HandleAction::Generating)
            {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                Log::debug("ActiveVolume::releaseChunkContainer - Chunk container(%llx, %llx) canceling generate - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
                m_grid->cancelLoadChunk(handle.get());
            }
            else
            {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                Log::debug("ActiveVolume::releaseChunkContainer - Chunk container(%llx, %llx) busy - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
            }
        }
        else if(action == RenderAction::Meshing)
        {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
            Log::debug("ActiveVolume::releaseChunkContainer - Chunk container(%llx, %llx) canceling meshing - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
            getProcessThread().cancelChunkMesh(container);
        }
        else
        {
            assert(false);//should not be here
        }

//        m_releaseChunkContainers.push_back(container); //store to check later
    }
    else
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::releaseChunkContainer - Chunk container(%llx, %llx) adding to release - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
    }
    m_releaseChunkContainers.push_back(container); //store to check later
}

//template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
//void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::releaseMesh(Mesh *mesh)
//{
//    m_chunkMeshes.release(mesh);
//}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::initRegionVolumeInfo(std::vector<RegionContainerInfo> &volume, glm::ivec3 &volumeSize, glm::ivec3 &volumeCenter)
{

}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::initChunkVolumeInfo(std::vector<ChunkContainerInfo> &volume, glm::ivec3 &volumeSize, glm::ivec3 &volumeCenter)
{
//    glm::ivec3 center(m_volumeSize.x/2, m_volumeSize.y/2, m_volumeSize.z/2);
    glm::ivec3 chunkIndex(0, 0, 0);
    size_t index=0;

    glm::vec3 centerf=volumeCenter;

    bool borderZ;
    bool borderY;
    bool border;

    //build volume info
    chunkIndex.z=0;
    for(size_t z=0; z<volumeSize.z; ++z)
    {
        if((z==0)||(z==volumeSize.z-1))
            borderZ=true;
        else
            borderZ=false;

        chunkIndex.y=0;
        for(size_t y=0; y<volumeSize.y; ++y)
        {
            if((y==0)||(y==volumeSize.z-1))
                borderY=true;
            else
                borderY=borderZ;

            chunkIndex.x=0;
            for(size_t x=0; x<volumeSize.x; ++x)
            {
                if((x==0)||(x==volumeSize.x-1))
                    border=true;
                else
                    border=borderY;

                ChunkContainerInfo &info=volume[index];

                float distance=glm::distance(glm::vec3(chunkIndex), centerf);

                info.keepData=(distance<=3.0f);
                info.lod=(distance/10.0f);
                info.mesh=!border;

                ++chunkIndex.x;
                ++index;
            }
            ++chunkIndex.y;
        }
        ++chunkIndex.z;
    }

//    generateUpdateOrder();
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::updatePosition(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    if(m_regionIndex.index!=regionIndex)
    {
        m_regionIndex.index=regionIndex;

//        m_regionVolume.update(m_regionIndex);
    }

    if((m_chunkIndex.region!=regionIndex)||(m_chunkIndex.chunk!=chunkIndex))
    {
        m_chunkIndex.region=regionIndex;
        m_chunkIndex.chunk=chunkIndex;
    }
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::updateChunkVolume()
{
        m_chunkVolume.update(m_chunkIndex, m_chunkLoadRequests, m_chunkUpdates);

        size_t processedLoads=0;

        for(ChunkLoadContainer &loadRequest:m_chunkLoadRequests)
        {
            ChunkContainer *container=loadRequest.container;

            if(container->getAction() == RenderAction::Idle)
            {
                bool loadHandle=false;
                SharedChunkHandle chunkHandle=container->getHandle();

                if(!chunkHandle)
                {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                    Log::debug("ActiveVolume::updateChunkVolume - Chunk container(%llx, %llx) request load handle invalid - %s", container, 0, container->getActionString().c_str());
#endif
                    processedLoads++;//act like we processed it
                    continue;
                }

                HandleState chunkState=chunkHandle->getState();

                m_chunkCount++;

                if(chunkHandle->getState() != HandleState::Memory)
                    loadHandle=true;
                else
                {
                    if(!chunkHandle->empty())
                    {
                        if(chunkHandle->getLod() != loadRequest.lod)
                            loadHandle=true;
                    }
                }

                if(loadHandle)
                {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                    Log::debug("ActiveVolume::updateChunkVolume - Chunk container(%llx, %llx) request load - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
                    if(!m_grid->loadChunk(chunkHandle.get(), loadRequest.lod))
                    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                        Log::debug("ActiveVolume::updateChunkVolume - Chunk container(%llx, %llx) request load failed no requests - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
                        break;
                    }
                    processedLoads++;
                    m_loadingChunks++;
                }
                else
                {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                    Log::debug("ActiveVolume::updateChunkVolume - Chunk container(%llx, %llx) failed to request load, already loaded - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
                    processedLoads++;//act like we processed it
                }
            }
            else
            {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                Log::debug("ActiveVolume::updateChunkVolume - Chunk container(%llx, %llx) failed to request load, container busy - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
                processedLoads++;//act like we processed it
            }
        }

        if(processedLoads == m_chunkLoadRequests.size())
            m_chunkLoadRequests.clear();
        else
            m_chunkLoadRequests.erase(m_chunkLoadRequests.begin(), m_chunkLoadRequests.begin()+processedLoads);

        typename _Grid::DescriptorType &descriptors=m_grid->getDescriptors();

        for(ChunkUpdateContainer &update:m_chunkUpdates)
        {
            if(update.status == UpdateStatus::NeedMesh)
            {
                ChunkContainer *container=update.container;

                m_chunkMeshQueue.push_back(container);
            }
        }

        m_chunkUpdates.clear();
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::update(MeshUpdates &loadedMeshes, MeshUpdates &releaseMeshes)
{
    m_grid->getUpdated(m_updatedRegions, m_updatedChunks, m_completedRequests);

    releaseContainers();

    updateRegions();

    updateChunks();

    updateChunkVolume();

    updateMeshes(loadedMeshes, releaseMeshes);
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::releaseContainers()
{
    for(size_t i=0; i<m_releaseChunkContainers.size(); )
    {
        ChunkContainer *chunkContainer=m_releaseChunkContainers[i];

        if(chunkContainer->getAction()==RenderAction::Idle)
        {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
            Log::debug("ActiveVolume::releaseContainers - Chunk container(%llx, %llx) release - %s", chunkContainer, chunkContainer->getKey().hash, chunkContainer->getActionString().c_str());
#endif
            chunkContainer->release();
            m_chunkContainers.release(chunkContainer);

            //erase container by swapping with back and popping
            m_releaseChunkContainers[i]=m_releaseChunkContainers.back();
            m_releaseChunkContainers.pop_back();
        }
        else
            ++i;
    }
    
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::updateRegions()
{
    if(!m_updatedRegions.empty())
    {
        //not handling region updates at the moment
        m_updatedRegions.clear();
    }
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::updateChunks()
{
    if(!m_updatedChunks.empty())
    {
        //handle any chunk updates
        RegionChunkIndex index;
        typename _Grid::DescriptorType &descriptors=m_grid->getDescriptors();

        for(size_t i=0; i<m_updatedChunks.size(); ++i)
        {
            m_loadingChunks--;

            Key &key=m_updatedChunks[i];

            index.region=descriptors.getRegionIndex(key.regionHash);
            index.chunk=descriptors.getChunkIndex(key.chunkHash);

            ChunkContainerInfo *containerInfo=m_chunkVolume.getContainerInfo(index);

            if(containerInfo==nullptr)
            {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                Log::debug("ActiveVolume::updateChunks - Chunk container(%llx, %llx) load complete but containerInfo invalid", nullptr, key.hash);
#endif
                continue;
            }

            ChunkContainer *container=containerInfo->container;

            if(container == nullptr)
            {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                Log::debug("ActiveVolume::updateChunks - Chunk container(%llx, %llx) load complete but container invalid", nullptr, key.hash);
#endif
                continue;
            }

            if(container->isValid()&&containerInfo->mesh)
            {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                Log::debug("ActiveVolume::updateChunks - Chunk container(%llx, %llx) load complete adding to mesh request - %s", containerInfo->container, key.hash, 
                    containerInfo->container->getActionString().c_str());
#endif
                m_chunkMeshQueue.push_back(container);
            }
            else
            {
                if(!containerInfo->mesh)
                {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                    Log::debug("ActiveVolume::updateChunks - Chunk container(%llx, %llx) load complete no mesh currently needed - %s", containerInfo->container, key.hash, 
                        containerInfo->container->getActionString().c_str());
#endif
                }
                else
                {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
                    Log::debug("ActiveVolume::updateChunks - Chunk container(%llx, %llx) load complete but handle invalid - ", containerInfo->container, key.hash, 
                        containerInfo->container->getActionString().c_str());
#endif
                }
            }
        }
        m_updatedChunks.clear();
    }
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::updateMeshes(MeshUpdates &loadedMeshes, MeshUpdates &releaseMeshes)
{
    //return all released meshes
    for(size_t i=0; i<releaseMeshes.size(); ++i)
    {
        m_chunkMeshes.release(releaseMeshes[i].mesh);
    }
    releaseMeshes.clear();

    for(size_t i=0; i<m_completedRequests.size(); ++i)
    {
        process::Request *request=m_completedRequests[i];

        switch(request->type)
        {
        case process::Type::Mesh:
        {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
            Log::debug("ActiveVolume::updateMeshes - process chunk mesh %llx", request);
#endif
            completeMeshRequest(request, loadedMeshes);
        }
        break;
        default:
            break;
        }
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::updateMeshes - release request %llx", request);
#endif
        getProcessThread().releaseRequest(request);
    }
    m_completedRequests.clear();

    generateMeshRequest();
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::completeMeshRequest(process::Request *request, MeshUpdates &loadedMeshes)
{
    ChunkContainer *container=(ChunkContainer *)request->data.buildMesh.renderer;
    Mesh *mesh=(Mesh *)request->data.buildMesh.mesh;

    //updated chunks just need to swap out the mesh as that is all that should have been changed
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
    glm::ivec3 regionIndex=container->getRegionIndex();
    glm::ivec3 chunkIndex=container->getChunkIndex();

    Log::debug("ActiveVolume::completeMeshRequest - Chunk renderer (%llx,%llx) request:%llx - mesh complete", 
        container, container->getKey().hash, request);
#endif//DEBUG_MESH

    RegionChunkIndex index;

    index.region=container->getRegionIndex();
    index.chunk=container->getChunkIndex();

    ChunkContainerInfo *chunkContainer=m_chunkVolume.getContainerInfo(index);

    if((chunkContainer==nullptr) || (chunkContainer->container != container))
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::completeMeshRequest - Chunk renderer(%llx, %llx) request:%llx - mesh complete but renderer no longer active*********", 
            container, container->getKey().hash, request);
#endif

        //need to let the container know that it is idle again
        container->setAction(RenderAction::Idle); //renderer is idle again, make sure it can be cleaned up

//        getProcessThread().returnMesh(container, request->data.buildMesh.mesh);
        m_chunkMeshes.release(mesh);

//#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
//        Log::debug("ActiveVolume::completeMeshRequest - release request %llx", request);
//#endif
//        getProcessThread().releaseRequest(request);
        m_meshingChunks--;
        return;
    }

    SharedChunkHandle chunkHandle=container->getChunkHandle();

    chunkHandle->removeInUse();
    if(!chunkHandle->inUse())
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::completeMeshRequest - Chunk renderer(%llx, %llx) mesh complete and being released - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
        m_grid->releaseChunk(chunkHandle.get());
    }
    else
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::completeMeshRequest - Chunk renderer(%llx, %llx) mesh complete but still in use - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
    }
    
    loadedMeshes.emplace_back(container, mesh);
    m_meshingChunks--;
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
bool ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::requestChunkContainerMesh(_ChunkContainer *container)
{
    if(container->getAction() != RenderAction::Idle)
        return true;

    SharedChunkHandle handle=container->getHandle();

    if(!handle)
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::requestChunkContainerMesh - Chunk container(%llx, nullptr) mesh request - but handle invalid", container);
#endif
        return true;
    }

    if(handle->empty())//empty no need to mesh
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::requestChunkContainerMesh - Chunk container(%llx, %llx) mesh request - but chunk is empty", container, container->getKey().hash);
#endif
        return true;
    }
    
    if(handle->getState() != HandleState::Memory)
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::requestChunkContainerMesh - Chunk container(%llx, %llx) mesh request - but chunk is not loaded", container, container->getKey().hash);
#endif        
        return true;
    }

    Mesh *mesh=m_chunkMeshes.get();

    if(!mesh)
    {
#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
        Log::debug("ActiveVolume::requestChunkContainerMesh - Chunk container(%llx, %llx) request mesh failed no meshes*********************", container, container->getKey().hash);
#endif
        return false;
    }

#ifdef VOXIGEN_DEBUG_ACTIVEVOLUME
    Log::debug("ActiveVolume::requestChunkContainerMesh - Chunk container(%llx, %llx) request mesh - %s", container, container->getKey().hash, container->getActionString().c_str());
#endif
    container->setAction(RenderAction::Meshing);
    container->setMeshState(MeshState::Meshing);
    m_meshingChunks++;
    handle->addInUse();

    getProcessThread().requestChunkMesh(container, mesh);
    return true;
}

template<typename _Grid, typename _ChunkContainer, typename _RegionContainer>
void ActiveVolume<_Grid, _ChunkContainer, _RegionContainer>::generateMeshRequest()
{
    size_t count=0;
    //send chunks off the need to be meshed
    for(size_t i=0; i<m_chunkMeshQueue.size(); ++i)
    {
        ChunkContainer *container=m_chunkMeshQueue[i];
        
        if(!requestChunkContainerMesh(container))
            break;
        
        count++;
    }

    if(count == m_chunkMeshQueue.size())
        m_chunkMeshQueue.clear();
    else
        m_chunkMeshQueue.erase(m_chunkMeshQueue.begin(), m_chunkMeshQueue.begin()+count);
}

}//namespace voxigen