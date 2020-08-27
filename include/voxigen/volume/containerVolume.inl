#include "voxigen/volume/chunkFunctions.h"

namespace voxigen
{

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::ContainerVolume(Grid *grid, Descriptor *descriptors, InitVolumeInfo initVolumeInfo, GetContainer getContainer, ReleaseContainer releaseContainer):
m_grid(grid),
m_descriptors(descriptors),
m_init(false),
initVolumeInfo(initVolumeInfo),
getContainer(getContainer),
releaseContainer(releaseContainer)
{
//    setViewRadius(glm::ivec3(64, 64, 64));
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::~ContainerVolume()
{

}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::setViewRadius(const glm::ivec3 &radius)// , LoadRequests &load, std::vector<Container *> &release)
{
    m_radius=radius;
    glm::ivec3 volumeSize=calcVolumeSize(radius);

    m_containerCount=volumeSize.x*volumeSize.y*volumeSize.z;
//    init(m_index, load, release);
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
glm::ivec3 ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::calcVolumeSize(const glm::ivec3 &radius)
{
    glm::ivec3 volumeSize;
    glm::ivec3 renderSize=_Container::getSize();

    volumeSize=(radius)/renderSize;

    //make minimum size is 1
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

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::init(const Index &index, LoadRequests &load, UpdateContainers &updates)
{
    //release any currently existing container
    if(!m_volume.empty())
        releaseContainers(glm::ivec3(0, 0, 0), m_volumeSize, updates);

    m_volumeSize=calcVolumeSize(m_radius);
    m_volume.resize(m_volumeSize.x*m_volumeSize.y*m_volumeSize.z);

    for(auto &info:m_volume)
    {
        info.container=nullptr;
    }

    m_volumeCenterIndex=(m_volumeSize/2);

    initVolumeInfo(m_volume, m_volumeSize, m_volumeCenterIndex);

    rebuild(index, load, updates);
    m_init=true;
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::generateUpdateOrder()
{
    spiralIndex(m_volumeSize, m_updateOrder);
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::update(const Index &index, LoadRequests &load, UpdateContainers &updates)
{
    if(!m_init)
        init(index, load, updates);
    
    

    //no changes, skip update
    if(m_index == index)
    {
        if(m_missingContainers)
            getMissingContainers(load);
        return;
    }

    //    glm::ivec3 gridRegionSize=m_descriptors->getRegionSize();
    glm::ivec3 offset=_Index::difference(m_grid, m_index, index);

    bool needRebuild=false;
    glm::ivec3 direction=glm::abs(offset);

    if(direction.x>m_volumeSize.x/2)
        needRebuild=true;
    else if(direction.y>m_volumeSize.y/2)
        needRebuild=true;
    if(direction.z>m_volumeSize.z/2)
        needRebuild=true;

    if(needRebuild)
    {
        rebuild(index, load, updates);
        return;
    }

    glm::ivec3 releaseSize=direction;
    glm::ivec3 copySize=m_volumeSize-direction;

    if(direction.x!=0.0)
        direction.x=offset.x/direction.x;
    if(direction.y!=0.0)
        direction.y=offset.y/direction.y;
    if(direction.z!=0.0)
        direction.z=offset.z/direction.z;

    glm::ivec3 dir=direction;

    if(dir.x==0)
        dir.x=1;
    if(dir.y==0)
        dir.y=1;
    if(dir.z==0)
        dir.z=1;

    glm::ivec3 copyFrom;
    glm::ivec3 copyTo;

    if(direction.x<0)
        copyTo.x=m_volumeSize.x-1;
    else
        copyTo.x=0;
    copyFrom.x=copyTo.x+offset.x;

    if(direction.y<0)
        copyTo.y=m_volumeSize.y-1;
    else
        copyTo.y=0;
    copyFrom.y=copyTo.y+offset.y;

    if(direction.z<0)
        copyTo.z=m_volumeSize.z-1;
    else
        copyTo.z=0;
    copyFrom.z=copyTo.z+offset.z;

    //release
    glm::ivec3 regionStart;
    glm::ivec3 regionSize;

    regionStart.x=0;
    regionStart.y=0;
    if(copyTo.z<copyFrom.z)
        regionStart.z=0;
    else
        regionStart.z=m_volumeSize.z-releaseSize.z;

    //    regionStart.z=std::min(copyTo.z, copyFrom.z);

    regionSize.x=m_volumeSize.x;
    regionSize.y=m_volumeSize.y;
    regionSize.z=releaseSize.z;

    releaseContainers(regionStart, regionSize, updates);

    if(copyTo.z<copyFrom.z)
        regionStart.z=copyFrom.z;
    else
        regionStart.z=0;

    regionSize.z=copySize.z;
    //    regionStart.y=std::min(copyTo.y, copyFrom.y);
    regionSize.y=releaseSize.y;
    if(copyTo.y<copyFrom.y)
        regionStart.y=0;
    else
        regionStart.y=m_volumeSize.y-releaseSize.y;

    releaseContainers(regionStart, regionSize, updates);

    if(copyTo.y<copyFrom.y)
        regionStart.y=copyFrom.y;
    else
        regionStart.y=0;

    regionSize.y=copySize.y;
    //    regionStart.x=std::min(copyTo.x, copyFrom.x);
    regionSize.x=releaseSize.x;
    if(copyTo.x<copyFrom.x)
        regionStart.x=0;
    else
        regionStart.x=m_volumeSize.x-releaseSize.x;

    releaseContainers(regionStart, regionSize, updates);


    //copy
    size_t indexTo;
    size_t indexFrom;

    indexFrom=copyFrom.z*(m_volumeSize.y*m_volumeSize.x)+copyFrom.y*m_volumeSize.x+copyFrom.x;
    indexTo=copyTo.z*(m_volumeSize.y*m_volumeSize.x)+copyTo.y*m_volumeSize.x+copyTo.x;

    glm::ivec3 stride(dir.x, (dir.y-dir.x)*m_volumeSize.x, (dir.z-dir.y)*m_volumeSize.y*m_volumeSize.x);

    stride.y+=dir.x*(m_volumeSize.x-copySize.x);
    stride.z+=dir.y*(m_volumeSize.y-copySize.y)*m_volumeSize.x;

    //copy info
    for(size_t z=0; z<copySize.z; z++)
    {
        for(size_t y=0; y<copySize.y; y++)
        {
            for(size_t x=0; x<copySize.x; x++)
            {
                //                assert(m_volume[indexTo] == nullptr);
                //                if(m_volume[indexTo])
                //                    m_volume[indexTo]->refCount--;

                if(m_volume[indexTo].mesh && !m_volume[indexFrom].mesh && m_volume[indexFrom].container)
                    updates.emplace_back(UpdateStatus::NeedMesh, m_volume[indexFrom].container);

                m_volume[indexTo].container=m_volume[indexFrom].container;
                //               m_volume[indexTo]->refCount++;

                indexFrom+=stride.x;
                indexTo+=stride.x;
            }
            indexFrom+=stride.y;
            indexTo+=stride.y;
        }
        indexFrom+=stride.z;
        indexTo+=stride.z;
    }

    //add the new render info
    regionStart.x=0;
    regionStart.y=0;

    if(copyTo.z<copyFrom.z)
        regionStart.z=m_volumeSize.z-releaseSize.z;
    else
        regionStart.z=0;

    regionSize.x=m_volumeSize.x;
    regionSize.y=m_volumeSize.y;
    regionSize.z=releaseSize.z;

    //    glm::ivec3 updateRegionIndex;
    //    glm::ivec3 updateChunkIndex;
    Index updateIndex=Index::offset(m_grid, index, (regionStart-m_volumeCenterIndex));
    getContainers(regionStart, updateIndex, regionSize, load);

    if(copyTo.z<=copyFrom.z)
        regionStart.z=0;
    else
        regionStart.z=releaseSize.z;

    regionSize.z=copySize.z;

    if(copyTo.y<copyFrom.y)
        regionStart.y=m_volumeSize.y-releaseSize.y;
    else
        regionStart.y=0;
    regionSize.y=releaseSize.y;

    updateIndex=Index::offset(m_grid, index, (regionStart-m_volumeCenterIndex));
    getContainers(regionStart, updateIndex, regionSize, load);

    if(copyTo.y<=copyFrom.y)
        regionStart.y=0;
    else
        regionStart.y=releaseSize.y;

    regionSize.y=copySize.y;

    if(copyTo.x<=copyFrom.x)
        regionStart.x=m_volumeSize.x-releaseSize.x;
    else
        regionStart.x=0;
    regionSize.x=releaseSize.x;

    updateIndex=Index::offset(m_grid, index, (regionStart-m_volumeCenterIndex));
    getContainers(regionStart, updateIndex, regionSize, load);

    m_index=index;

    if(m_missingContainers)
        getMissingContainers(load);

    //    if(direction.x < 0)
    //        start.x=
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::rebuild(const Index &index, LoadRequests &load, UpdateContainers &updates)
{
    if(!m_volume.empty())
        releaseContainers(glm::ivec3(0, 0, 0), m_volumeSize, updates);

    m_missingContainers=false;

    Index startIndex;

    startIndex=Index::offset(m_grid, index, -m_volumeCenterIndex);
    getContainers(glm::ivec3(0, 0, 0), startIndex, m_volumeSize, load);
    m_index=index;
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::releaseContainers(const glm::ivec3 &start, const glm::ivec3 &size, UpdateContainers &updates)
{
    size_t index=start.z*(m_volumeSize.y*m_volumeSize.x)+start.y*m_volumeSize.x+start.x;

    size_t strideX=m_volumeSize.x-size.x;
    size_t strideY=(m_volumeSize.y-size.y)*m_volumeSize.x;
    size_t count=0;

    for(size_t z=0; z<size.z; z++)
    {
        for(size_t y=0; y<size.y; y++)
        {
            for(size_t x=0; x<size.x; x++)
            {
                Container *container=m_volume[index].container;

                if(container)
                {
                    m_volume[index].container=nullptr;
//                    updates.emplace_back(UpdateStatus::Release, container);
                    releaseContainer(container);
                    count++;
                }
                
                index++;
            }
            index+=strideX;
        }
        index+=strideY;
    }

#ifdef VOXIGEN_DEBUG_CONTAINERVOLUME
    Log::debug("ContainerVolume::releaseContainers - Start(%d, %d, %d) Size(%d, %d, %d) Count:%d", start.x, start.y, start.z, size.x, size.y, size.z, count);
#endif//VOXIGEN_DEBUG_CONTAINERVOLUME
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::getContainers(const glm::ivec3 &start, const Index &startIndex, const glm::ivec3 &size, LoadRequests &load)
{
    glm::ivec3 regionSize=m_descriptors->getRegionSize();
    size_t index=start.z*(m_volumeSize.y*m_volumeSize.x)+start.y*m_volumeSize.x+start.x;

    size_t strideX=m_volumeSize.x-size.x;
    size_t strideY=(m_volumeSize.y-size.y)*m_volumeSize.x;
    size_t count=0;

    Index renderIndex;

    renderIndex.setZ(startIndex);
    for(size_t z=0; z<size.z; z++)
    {
        renderIndex.setY(startIndex);

        for(size_t y=0; y<size.y; y++)
        {
            renderIndex.setX(startIndex);

            for(size_t x=0; x<size.x; x++)
            {
                typename Index::Handle handle=Index::getHandle(m_grid, renderIndex);

                if(handle)
                {
                    Container *container=getContainer();

                    //                assert(container);

                    if(container)
                    {
                        m_volume[index].container=container;

                        container->setAction(RenderAction::Idle);
                        container->setHandle(handle);
#ifdef VOXIGEN_DEBUG_CONTAINERVOLUME
                        Log::debug("ContainerVolume::getContainers - Chunk Container(%llx, %llx) %s setHandle %llx", container, container->getKey().hash, renderIndex.pos().c_str(), handle.get());
#endif//VOXIGEN_DEBUG_CONTAINERVOLUME
                        load.emplace_back(m_volume[index].lod, container);
                        count++;
                    }
                    else
                    {
#ifdef VOXIGEN_DEBUG_CONTAINERVOLUME
                        Log::debug("ContainerVolume::getContainers - Failed to get container %s *************************", renderIndex.pos().c_str());
#endif//VOXIGEN_DEBUG_CONTAINERVOLUME
                        m_volume[index].container=nullptr;
                        m_missingContainers=true;
                    }
                }
                else
                {
#ifdef VOXIGEN_DEBUG_CONTAINERVOLUME
                    Log::debug("ContainerVolume::getContainers - failed to get handle");
#endif//VOXIGEN_DEBUG_CONTAINERVOLUME
                    m_volume[index].container=nullptr;
                }

                index++;
                renderIndex.incX(m_grid);
            }

            index+=strideX;
            renderIndex.incY(m_grid);
        }

        index+=strideY;
        renderIndex.incZ(m_grid);
    }

#ifdef VOXIGEN_DEBUG_CONTAINERVOLUME
    Log::debug("ContainerVolume::getContainers - Start(%d, %d, %d) Size(%d, %d, %d) Count:%d", start.x, start.y, start.z, size.x, size.y, size.z, count);
#endif//VOXIGEN_DEBUG_CONTAINERVOLUME
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::getMissingContainers(LoadRequests &load)
{
    size_t index=0;
    Index startIndex=Index::offset(m_grid, m_index, -m_volumeCenterIndex);
    Index renderIndex;
    bool missingContainers=false;

    renderIndex.setZ(startIndex);
    for(size_t z=0; z<m_volumeSize.z; z++)
    {
        renderIndex.setY(startIndex);
        for(size_t y=0; y<m_volumeSize.y; y++)
        {
            renderIndex.setX(startIndex);
            for(size_t x=0; x<m_volumeSize.x; x++)
            {
                if(m_volume[index].container==nullptr)
                {
                    Container *container=getContainer();

                    if(!container)
                    {
                        m_missingContainers=true;
                        return;//no containers try again next frame
                    }

                    m_volume[index].container=container;
                    typename Index::Handle handle=Index::getHandle(m_grid, renderIndex);

                    if(!handle)
                    {
                        handle=nullptr;
                    }

                    container->setAction(RenderAction::Idle);
                    container->setHandle(handle);
#ifdef VOXIGEN_DEBUG_CONTAINERVOLUME
                    Log::debug("ContainerVolume::getMissingContainers - Chunk Container(%llx, %llx) get missing %s, %llx", container, container->getKey().hash, renderIndex.pos().c_str(), handle.get());
#endif//VOXIGEN_DEBUG_CONTAINERVOLUME
                    load.emplace_back(m_volume[index].lod, container);
                }
                renderIndex.incX(m_grid);
                ++index;
            }
            renderIndex.incY(m_grid);
        }
        renderIndex.incZ(m_grid);
    }

    m_missingContainers=missingContainers;
}

//template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
//void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::updateRegion(glm::ivec3 &startRegionIndex, glm::ivec3 &startChunkIndex, glm::ivec3 &size)
//{
//    glm::ivec3 regionIndex;
//    glm::ivec3 chunkIndex;
//    size_t renderPos=0;
//
//    glm::ivec3 regionSize=m_descriptors->getRegionSize();
//    glm::ivec3 offset=m_volumeSize-size;
//
//    regionIndex.z=startRegionIndex.z;
//    chunkIndex.z=startChunkIndex.z;
//
//    for(size_t z=0; z<size.z; ++z)
//    {
//        regionIndex.y=startRegionIndex.y;
//        chunkIndex.y=startChunkIndex.y;
//
//        for(size_t y=0; y<size.y; ++y)
//        {
//            regionIndex.x=startRegionIndex.x;
//            chunkIndex.x=startChunkIndex.x;
//
//            for(size_t x=0; x<size.x; ++x)
//            {
//                Container *container=m_volume[renderPos];
//                
//                //get rid of any existing info/chunks/containers
//                releaseChunkInfo(container);
//                m_volume[renderPos]=nullptr;
//                
//                Key key(m_descriptors->getRegionHash(regionIndex), m_descriptors->getChunkHash(chunkIndex));
//                SharedChunkHandle chunkHandle=m_grid->getChunk(key);
//
////                m_volumeMap[key.hash]=renderPos;
//                renderInfo.chunkHandle=chunkHandle;
//
//                //if not loaded, load it
//                if(chunkHandle->state()!=HandleState::Memory)
//                {
//                    //make sure we are not already requesting something from it
//                    if(chunkHandle->action()==HandleAction::Idle)
//                        m_grid->loadChunk(chunkHandle, 0);
//                }
//
//                //if we dont have a container go ahead and request it
//                if(renderInfo.container == nullptr)
//                {
////                    renderInfo.state=RenderState::RequestContainer;
////                    m_renderPrepThread->requestContainer(renderInfo.chunkHandle);
//                    renderInfo.container=m_containerQueue.get([&](Container *container){container->build(); container->buildOutline(m_outlineInstanceId);});
//
//                    if(renderInfo.container)
//                        renderInfo.container->setChunk(chunkHandle);
//                }
//
//                chunkIndex.x++;
//                
//                if(chunkIndex.x>=regionSize.x)
//                {
//                    chunkIndex.x=0;
//                    regionIndex.x++;
//                }
//                renderPos++;
//            }
//            renderPos+=offset.x;
//
//            chunkIndex.y++;
//            if(chunkIndex.y>=regionSize.y)
//            {
//                chunkIndex.y=0;
//                regionIndex.y++;
//            }
//        }
//
//        renderPos+=offset.y;
//
//        chunkIndex.z++;
//        if(chunkIndex.z>=regionSize.z)
//        {
//            chunkIndex.z=0;
//            regionIndex.z++;
//        }
//    }
//}

//template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
//void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::draw()
//{
//    glm::ivec3 regionIndex=m_index.regionIndex()-(m_index.regionIndex()-m_cameraIndex.regionIndex());
//
//    for(auto container:m_volume)
//    {
//        if(container)
//        {
//            glm::ivec3 regionOffset=container->getRegionIndex()-regionIndex;
//            glm::ivec3 offset=regionOffset*container->getRegionCellSize();
//
//            container->draw(offset);
//        }
//    }
//}
//
//template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
//void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::drawInfo(const glm::mat4x4 &projectionViewMat)
//{
//    glm::ivec3 regionIndex=m_index.regionIndex()-(m_index.regionIndex()-m_cameraIndex.regionIndex());
//
//    for(auto container:m_volume)
//    {
//        if(container)
//        {
//            glm::ivec3 regionOffset=container->getRegionIndex()-regionIndex;
//            glm::ivec3 offset=regionOffset*container->getRegionCellSize();
//
//            container->drawInfo(projectionViewMat, offset);
//        }
//    }
//}
//
//template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
//void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::drawOutline()
//{
//    glm::ivec3 regionIndex=m_index.regionIndex()-(m_index.regionIndex()-m_cameraIndex.regionIndex());
//
//    for(auto container:m_volume)
//    {
//        if(container)
//        {
//            glm::ivec3 regionOffset=container->getRegionIndex()-regionIndex;
//            glm::ivec3 offset=regionOffset*container->getRegionCellSize();
//
//            container->drawOutline(offset);
//        }
//    }
//}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
typename ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::ContainerInfo *ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::getContainerInfo(const Index &index)
{
    glm::ivec3 offset=_Index::difference(m_grid, m_index, index);
    glm::ivec3 containerIndex=m_volumeCenterIndex+offset;

    if(containerIndex.x<0)
        return nullptr;
    if(containerIndex.x>=m_volumeSize.x)
        return nullptr;
    if(containerIndex.y<0)
        return nullptr;
    if(containerIndex.y>=m_volumeSize.y)
        return nullptr;
    if(containerIndex.z<0)
        return nullptr;
    if(containerIndex.z>=m_volumeSize.z)
        return nullptr;

    size_t renderIndex=containerIndex.z*(m_volumeSize.y*m_volumeSize.x)+containerIndex.y*m_volumeSize.x+containerIndex.x;

    return &m_volume[renderIndex];
}

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
void ContainerVolume<_Grid, _Index, _ContainerInfo, _Container>::releaseInfo(Container *container)
{
    if(!container)
        return;

    //    assert(container->getAction()==RenderAction::Idle);
        //we are dumping this container so chunk no longer need.
    if(container->getHandle()->action()!=HandleAction::Idle)
        Index::cancelLoad(m_grid, container->getHandle());

    //can only release the container if it is not busy
    if((container->getAction()==RenderAction::Idle))//&& (container->getChunkHandle()->action()==HandleAction::Idle))
        releaseContainer(container);
    else
        m_containerReleaseQueue.push_back(container);
}

}//namespace voxigen