#include "voxigen/volume/chunkFunctions.h"

namespace voxigen
{

template<typename _Grid, typename _Container, typename _Index>
ActiveVolume<_Grid, _Container, _Index>::ActiveVolume(GridType *grid, DescriptorType *descriptors, GetContainer getContainer, ReleaseContainer releaseContainer):
m_grid(grid),
m_descriptors(descriptors),
//m_viewRadius(64),
getContainer(getContainer),
releaseContainer(releaseContainer)
{
    //make it allocate 512 upfront
 //   m_containerQueue.setMaxSize(512);
 //   m_containerQueue.setGrowSize(512);
    setViewRadius(glm::ivec3(64, 64, 64));
}

template<typename _Grid, typename _Container, typename _Index>
ActiveVolume<_Grid, _Container, _Index>::~ActiveVolume()
{

}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::setViewRadius(const glm::ivec3 &radius)
{
    m_viewRadius=radius;

    glm::ivec3 cubeSize=calcVolumeSize(m_viewRadius);
    m_containerCount=cubeSize.x*cubeSize.y*cubeSize.z;
//    containerCount=(containerCount/2)*3;
//    m_containerQueue.setMaxSize(containerCount);
}

template<typename _Grid, typename _Container, typename _Index>
glm::ivec3 ActiveVolume<_Grid, _Container, _Index>::calcVolumeSize(const glm::ivec3 &radius)
{
    glm::ivec3 cubeSize;
//  glm::ivec3 chunkSize=m_descriptors->getChunkSize();
    glm::ivec3 renderSize=_Container::getSize();

    cubeSize=(radius)/renderSize;

    if(cubeSize.x<=1)
        cubeSize.x++;
    if(cubeSize.y<=1)
        cubeSize.y++;
    if(cubeSize.z<=1)
        cubeSize.z++;

    //make everything odd
    if(cubeSize.x%2==0)
        cubeSize.x++;
    if(cubeSize.y%2==0)
        cubeSize.y++;
    if(cubeSize.z%2==0)
        cubeSize.z++;

    return cubeSize;
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::init(const Index &index, LoadRequests &load, std::vector<ContainerType *> &release)
{
    glm::ivec3 regionSize=m_descriptors->getRegionSize();
//    glm::ivec3 chunkSize=m_descriptors->getChunkSize();
//    m_volumeSize=m_viewRadius/chunkSize;
//
//    //make everything odd
//    if(m_volumeSize.x%2==0)
//        m_volumeSize.x++;
//    if(m_volumeSize.y%2==0)
//        m_volumeSize.y++;
//    if(m_volumeSize.z%2==0)
//        m_volumeSize.z++;
    
    //release any currently existing container
    if(!m_volume.empty())
        releaseRegion(glm::ivec3(0, 0, 0), m_volumeSize, release);
    
    m_volumeSize=calcVolumeSize(m_viewRadius);
    m_volume.resize(m_volumeSize.x*m_volumeSize.y*m_volumeSize.z);

    for(auto &info:m_volume)
    {
        info.container=nullptr;
    }
//    std::fill(m_volume.begin(), m_volume.end(), nullptr);

    m_volumeCenterIndex=(m_volumeSize/2);
//    glm::ivec3 centerPos=chunkIndex;

//    glm::ivec3 startRegion;// =regionIndex;
//    glm::ivec3 startChunk;// =m_chunkIndex-m_volumeCenter+glm::ivec3(1, 1, 1);
    Index startIndex;
    size_t renderPos=0;

//    Index::offset(regionIndex, chunkIndex, -m_volumeCenterIndex, startRegion, startChunk);
    startIndex=Index::offset(index, -m_volumeCenterIndex);

    initVolumeInfo();

//    while(startChunk.x<0)
//    {
//        startRegion.x--;
//        startChunk.x+=regionSize.x;
//    }
//    while(startChunk.y<0)
//    {
//        startRegion.y--;
//        startChunk.y+=regionSize.y;
//    }
//    while(startChunk.z<0)
//    {
//        startRegion.z--;
//        startChunk.z+=regionSize.z;
//    }
//    getRegion(glm::ivec3(0, 0, 0), startRegion, startChunk, m_volumeSize);
    getRegion(glm::ivec3(0, 0, 0), startIndex, m_volumeSize, load);
//    updateRegion(startRegion, startChunk, m_volumeSize);

    m_index=index;
//    m_chunkIndex=chunkIndex;
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::updateCamera(const Index &index)
{
//    m_cameraRegionIndex=regionIndex;
//    m_cameraChunkIndex=chunkIndex;
    m_cameraIndex=index;
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::initVolumeInfo()
{
    glm::ivec3 center(m_volumeSize.x/2, m_volumeSize.y/2, m_volumeSize.z/2);
    glm::ivec3 chunkIndex(0, 0, 0);
    size_t index=0;

    glm::vec3 centerf=center;

    bool borderZ;
    bool borderY;
    bool border;

    //build volume info
    chunkIndex.z=0;
    for(size_t z=0; z<m_volumeSize.z; ++z)
    {
        if((z==0)||(z==m_volumeSize.z-1))
            borderZ=true;
        else
            borderZ=false;

        chunkIndex.y=0;
        for(size_t y=0; y<m_volumeSize.y; ++y)
        {
            if((y==0)||(y==m_volumeSize.z-1))
                borderY=true;
            else
                borderY=borderZ;

            chunkIndex.x=0;
            for(size_t x=0; x<m_volumeSize.x; ++x)
            {
                if((x==0)||(x==m_volumeSize.x-1))
                    border=true;
                else
                    border=borderY;

                ContainerInfo &info=m_volume[index];

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
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::update(const Index &index, LoadRequests &load, std::vector<ContainerType *> &release)
{
    //no changes, skip update
//    if((regionIndex==m_regionIndex)&&(chunkIndex==m_chunkIndex))
//        return;
    if(m_index==index)
    {
        getMissingContainers(load);
        return;
    }

//    glm::ivec3 gridRegionSize=m_descriptors->getRegionSize();
    glm::ivec3 offset=_Index::difference(m_index, index);

    bool rebuild=false;
    glm::ivec3 direction=glm::abs(offset);

    if(direction.x>m_volumeSize.x/2)
        rebuild=true;
    else if(direction.y>m_volumeSize.y/2)
        rebuild=true;
    if(direction.z>m_volumeSize.z/2)
        rebuild=true;

    if(rebuild)
    {
        init(index, load, release);
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
    
    releaseRegion(regionStart, regionSize, release);

    if(copyTo.z < copyFrom.z)
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

    releaseRegion(regionStart, regionSize, release);

    if(copyTo.y < copyFrom.y)
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

    releaseRegion(regionStart, regionSize, release);

//    size_t index=copyTo.z*(m_volumeSize.y*m_volumeSize.x)+copyTo.y*m_volumeSize.x+copyTo.x;
//    size_t strideY=(dir.y-dir.x)*m_volumeSize.x;
//
//    //release info that is going to be removed
//    for(size_t z=0; z<releaseSize.z; z++)
//    {
//        //take out entire plane
//        for(size_t y=0; y<m_volumeSize.y; y++)
//        {
//            for(size_t x=0; x<m_volumeSize.x; x++)
//            {
//                releaseChunkInfo(_volume[indexTo]);
//                index+=dir.x;
//            }
//            index+=strideY;
//        }
//    }
//
//    size_t alignX=(dir.x)*copySize.x;
//    strideZ=(dir.z-dir.y)*m_volumeSize.y*m_volumeSize.x;
//    
//    for(size_t z=0; z<copySize.z; z++)
//    {
//        for(size_t y=0; y<releaseSize.y; y++)
//        {
//            for(size_t x=0; x<m_volumeSize.x; x++)
//            {
//                releaseChunkInfo(_volume[indexTo]);
//                index+=dir.x;
//            }
//            index+=strideY;
//        }
//
//        for(size_t y=0; y<copySize.y; y++)
//        {
//            for(size_t x=0; x<releaseSize.x; x++)
//            {
//                releaseChunkInfo(_volume[indexTo]);
//                index+=dir.x;
//            }
//            index+=alignX;
//            index+=strideY;
//        }
//        index+=strideZ;
//    }

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
    Index updateIndex=Index::offset(index, (regionStart-m_volumeCenterIndex));
    getRegion(regionStart, updateIndex, regionSize, load);

    if(copyTo.z <= copyFrom.z)
        regionStart.z=0;
    else
        regionStart.z=releaseSize.z;

    regionSize.z=copySize.z;
    
    if(copyTo.y<copyFrom.y)
        regionStart.y=m_volumeSize.y-releaseSize.y;
    else
        regionStart.y=0;
    regionSize.y=releaseSize.y;

    updateIndex=Index::offset(index, (regionStart-m_volumeCenterIndex));
    getRegion(regionStart, updateIndex, regionSize, load);

    if(copyTo.y <= copyFrom.y)
        regionStart.y=0;
    else
        regionStart.y=releaseSize.y;

    regionSize.y=copySize.y;
    
    if(copyTo.x<=copyFrom.x)
        regionStart.x=m_volumeSize.x-releaseSize.x;
    else
        regionStart.x=0;
    regionSize.x=releaseSize.x;

    updateIndex=Index::offset(index, (regionStart-m_volumeCenterIndex));
    getRegion(regionStart, updateIndex, regionSize, load);

    m_index=index;

//    if(direction.x < 0)
//        start.x=
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::releaseRegion(const glm::ivec3 &start, const glm::ivec3 &size, std::vector<ContainerType *> &release)
{
    size_t index=start.z*(m_volumeSize.y*m_volumeSize.x)+start.y*m_volumeSize.x+start.x;
    
    size_t strideX=m_volumeSize.x-size.x;
    size_t strideY=(m_volumeSize.y-size.y)*m_volumeSize.x;

    for(size_t z=0; z<size.z; z++)
    {
        for(size_t y=0; y<size.y; y++)
        {
            for(size_t x=0; x<size.x; x++)
            {
//                releaseChunkInfo(m_volume[index]);
                ContainerType *container=m_volume[index].container;
//
//                if(container)
//                {
////                    container->refCount--;
//                    releaseInfo(container);
//                }
                if(container)
                    release.push_back(container);
                m_volume[index].container=nullptr;
                index++;
            }
            index+=strideX;
        }
        index+=strideY;
    }
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::getRegion(const glm::ivec3 &start, const Index &startIndex, const glm::ivec3 &size, LoadRequests &load)
{
    glm::ivec3 regionSize=m_descriptors->getRegionSize();
    size_t index=start.z*(m_volumeSize.y*m_volumeSize.x)+start.y*m_volumeSize.x+start.x;

    size_t strideX=m_volumeSize.x-size.x;
    size_t strideY=(m_volumeSize.y-size.y)*m_volumeSize.x;

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
                ContainerType *container=getFreeContainer();

//                assert(container);

                if(container)
                {
                    m_volume[index].container=container;
                    typename Index::Handle handle=Index::getHandle(m_grid, renderIndex);

                    container->setAction(RenderAction::Idle);
                    container->setHandle(handle);
#ifdef DEBUG_RENDERERS
                    Log::debug("MainThread - Container %x %s setHandle", container, renderIndex.pos().c_str());
#endif//DEBUG_RENDERERS
                    load.emplace_back(m_volume[index].lod, container);
                }
                else
                {
#ifdef DEBUG_RENDERERS
                    Log::debug("*****  MainThread - Failed to get container %s", renderIndex.pos().c_str());
#endif//DEBUG_RENDERERS
                    m_volume[index].container=nullptr;
                }

                index++;
                renderIndex.incX();
            }

            index+=strideX;
            renderIndex.incY();
        }

        index+=strideY;
        renderIndex.incZ();
    }
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::getMissingContainers(LoadRequests &load)
{
    size_t index=0;
    Index startIndex=Index::offset(m_index, -m_volumeCenterIndex);
    Index renderIndex;

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
                    ContainerType *container=getFreeContainer();

                    if(!container)
                        continue;

                    m_volume[index].container=container;
                    typename Index::Handle handle=Index::getHandle(m_grid, renderIndex);

                    container->setAction(RenderAction::Idle);
                    container->setHandle(handle);
#ifdef DEBUG_RENDERERS
                    Log::debug("MainThread - Get missing Container %x %s setHandle", container, renderIndex.pos().c_str());
#endif//DEBUG_RENDERERS
                    load.emplace_back(m_volume[index].lod, container);
                }
                renderIndex.incX();
                ++index;
            }
            renderIndex.incY();
        }
        renderIndex.incZ();
    }
}

//template<typename _Grid, typename _Container, typename _Index>
//void ActiveVolume<_Grid, _Container, _Index>::updateRegion(glm::ivec3 &startRegionIndex, glm::ivec3 &startChunkIndex, glm::ivec3 &size)
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
//                ContainerType *container=m_volume[renderPos];
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
//                    renderInfo.container=m_containerQueue.get([&](ContainerType *container){container->build(); container->buildOutline(m_outlineInstanceId);});
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

//template<typename _Grid, typename _Container, typename _Index>
//void ActiveVolume<_Grid, _Container, _Index>::draw()
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
//template<typename _Grid, typename _Container, typename _Index>
//void ActiveVolume<_Grid, _Container, _Index>::drawInfo(const glm::mat4x4 &projectionViewMat)
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
//template<typename _Grid, typename _Container, typename _Index>
//void ActiveVolume<_Grid, _Container, _Index>::drawOutline()
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

template<typename _Grid, typename _Container, typename _Index>
glm::ivec3 ActiveVolume<_Grid, _Container, _Index>::relativeCameraIndex()
{
    return m_index.regionIndex()-(m_index.regionIndex()-m_cameraIndex.regionIndex());
}

template<typename _Grid, typename _Container, typename _Index>
typename ActiveVolume<_Grid, _Container, _Index>::ContainerType *ActiveVolume<_Grid, _Container, _Index>::getRenderInfo(const Index &index)
{
//    glm::ivec3 regionIndex=m_descriptors->getRegionIndex(key.regionHash);
//    glm::ivec3 chunkIndex=m_descriptors->getChunkIndex(key.chunkHash);
//    glm::ivec3 gridRegionSize=m_descriptors->getRegionSize();
//    glm::ivec3 offset=Index::difference(m_index, index, gridRegionSize);
    glm::ivec3 offset=_Index::difference(m_index, index);

    glm::ivec3 cubeIndex=m_volumeCenterIndex+offset; //0 index;

    if(cubeIndex.x<0)
        return nullptr;
    if(cubeIndex.x>=m_volumeSize.x)
        return nullptr;
    if(cubeIndex.y<0)
        return nullptr;
    if(cubeIndex.y>=m_volumeSize.y)
        return nullptr;
    if(cubeIndex.z<0)
        return nullptr;
    if(cubeIndex.z>=m_volumeSize.z)
        return nullptr;

    size_t renderIndex=cubeIndex.z*(m_volumeSize.y*m_volumeSize.x)+cubeIndex.y*m_volumeSize.x+cubeIndex.x;
    
#ifndef NDEBUG
//    if(m_volume[renderIndex])
//        assert(m_volume[renderIndex]->getChunkHandle()->key().hash==key.hash);
#endif//NDEBUG
    
    return m_volume[renderIndex].container;
//    auto iter=m_volumeMap.find(key.hash);
//
//    if(iter==m_volumeMap.end())
//        return nullptr;
//
//    return &m_volume[iter->second];
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::releaseInfo(ContainerType *container)
{
    if(!container)
        return;

//    assert(container->getAction()==RenderAction::Idle);
    //we are dumping this container so chunk no longer need.
    if(container->getHandle()->action()!=HandleAction::Idle)
        Index::cancelLoad(m_grid, container->getHandle());

    //can only release the container if it is not busy
    if((container->getAction()==RenderAction::Idle) )//&& (container->getChunkHandle()->action()==HandleAction::Idle))
        releaseFreeContainer(container);
    else
        m_containerReleaseQueue.push_back(container);
}

template<typename _Grid, typename _Container, typename _Index>
typename ActiveVolume<_Grid, _Container, _Index>::ContainerType *ActiveVolume<_Grid, _Container, _Index>::getFreeContainer()
{
    //release any queued containers
    if(!m_containerReleaseQueue.empty())
    {
        for(auto iter=m_containerReleaseQueue.begin(); iter!=m_containerReleaseQueue.end(); )
        {
            ContainerType *container=*iter;

            if((container->getAction() == RenderAction::Idle) )//&& (container->getChunkHandle()->action() == HandleAction::Idle))
            {
                releaseContainer(container);
                iter=m_containerReleaseQueue.erase(iter);
            }
            else
                ++iter;
        }
    }

//    ContainerType *container=m_containerQueue.get([&](ContainerType *container){container->build(); });
    ContainerType *container=getContainer();
    
    return container;
}

template<typename _Grid, typename _Container, typename _Index>
void ActiveVolume<_Grid, _Container, _Index>::releaseFreeContainer(ContainerType *container)
{
//    MeshBuffer mesh=container->clearMesh();
//
//    if(mesh.valid)
//        m_meshHandler->releaseMesh(mesh);

    container->clear();
//    m_containerQueue.release(container);
    releaseContainer(container);
}

}//namespace voxigen