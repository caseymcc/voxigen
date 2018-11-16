#include "voxigen/chunkFunctions.h"

namespace voxigen
{

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::RenderCube(GridType *grid, DescriptorType *descriptors, MeshHandler *meshHandler):
m_grid(grid),
m_descriptors(descriptors),
m_meshHandler(meshHandler),
m_viewRadius(64)
{
    //make it allocate 512 upfront
    m_rendererQueue.setMaxSize(512);
    m_rendererQueue.setGrowSize(512);
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::~RenderCube()
{

}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::setViewRadius(const glm::ivec3 &radius)
{
    m_viewRadius=radius;

    glm::ivec3 cubeSize=calcCubeSize(m_viewRadius);
    size_t rendererCount=cubeSize.x*cubeSize.y*cubeSize.z;

    rendererCount=(rendererCount/2)*3;
    m_rendererQueue.setMaxSize(rendererCount);
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
glm::ivec3 RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::calcCubeSize(const glm::ivec3 &radius)
{
    glm::ivec3 cubeSize;
//  glm::ivec3 chunkSize=m_descriptors->getChunkSize();
    glm::ivec3 renderSize=_Renderer::getSize();

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

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::init(const Index &index)
{
    glm::ivec3 regionSize=m_descriptors->getRegionSize();
//    glm::ivec3 chunkSize=m_descriptors->getChunkSize();
//    m_renderCubeSize=m_viewRadius/chunkSize;
//
//    //make everything odd
//    if(m_renderCubeSize.x%2==0)
//        m_renderCubeSize.x++;
//    if(m_renderCubeSize.y%2==0)
//        m_renderCubeSize.y++;
//    if(m_renderCubeSize.z%2==0)
//        m_renderCubeSize.z++;
    
    //release any currently existing renderer
    if(!m_renderCube.empty())
        releaseRegion(glm::ivec3(0, 0, 0), m_renderCubeSize);
    
    m_renderCubeSize=calcCubeSize(m_viewRadius);
    m_renderCube.resize(m_renderCubeSize.x*m_renderCubeSize.y*m_renderCubeSize.z);

    std::fill(m_renderCube.begin(), m_renderCube.end(), nullptr);

    m_renderCubeCenterIndex=(m_renderCubeSize/2);
//    glm::ivec3 centerPos=chunkIndex;

//    glm::ivec3 startRegion;// =regionIndex;
//    glm::ivec3 startChunk;// =m_chunkIndex-m_renderCubeCenter+glm::ivec3(1, 1, 1);
    Index startIndex;
    size_t renderPos=0;

//    Index::offset(regionIndex, chunkIndex, -m_renderCubeCenterIndex, startRegion, startChunk);
    startIndex=Index::offset(index, -m_renderCubeCenterIndex);

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
//    getRegion(glm::ivec3(0, 0, 0), startRegion, startChunk, m_renderCubeSize);
    getRegion(glm::ivec3(0, 0, 0), startIndex, m_renderCubeSize);
//    updateRegion(startRegion, startChunk, m_renderCubeSize);

    m_index=index;
//    m_chunkIndex=chunkIndex;
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::updateCamera(const Index &index)
{
//    m_cameraRegionIndex=regionIndex;
//    m_cameraChunkIndex=chunkIndex;
    m_cameraIndex=index;
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::update(const Index &index)
{
    //no changes, skip update
//    if((regionIndex==m_regionIndex)&&(chunkIndex==m_chunkIndex))
//        return;
    if(m_index==index)
        return;

//    glm::ivec3 gridRegionSize=m_descriptors->getRegionSize();
    glm::ivec3 offset=_Index::difference(m_index, index);

    bool rebuild=false;
    glm::ivec3 direction=glm::abs(offset);

    if(direction.x>m_renderCubeSize.x/2)
        rebuild=true;
    else if(direction.y>m_renderCubeSize.y/2)
        rebuild=true;
    if(direction.z>m_renderCubeSize.z/2)
        rebuild=true;

    if(rebuild)
    {
        init(index);
        return;
    }

    glm::ivec3 releaseSize=direction;
    glm::ivec3 copySize=m_renderCubeSize-direction;

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
        copyTo.x=m_renderCubeSize.x-1;
    else
        copyTo.x=0;
    copyFrom.x=copyTo.x+offset.x;

    if(direction.y<0)
        copyTo.y=m_renderCubeSize.y-1;
    else
        copyTo.y=0;
    copyFrom.y=copyTo.y+offset.y;

    if(direction.z<0)
        copyTo.z=m_renderCubeSize.z-1;
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
        regionStart.z=m_renderCubeSize.z-releaseSize.z;

//    regionStart.z=std::min(copyTo.z, copyFrom.z);

    regionSize.x=m_renderCubeSize.x;
    regionSize.y=m_renderCubeSize.y;
    regionSize.z=releaseSize.z;
    
    releaseRegion(regionStart, regionSize);

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
        regionStart.y=m_renderCubeSize.y-releaseSize.y;

    releaseRegion(regionStart, regionSize);

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
        regionStart.x=m_renderCubeSize.x-releaseSize.x;

    releaseRegion(regionStart, regionSize);

//    size_t index=copyTo.z*(m_renderCubeSize.y*m_renderCubeSize.x)+copyTo.y*m_renderCubeSize.x+copyTo.x;
//    size_t strideY=(dir.y-dir.x)*m_renderCubeSize.x;
//
//    //release info that is going to be removed
//    for(size_t z=0; z<releaseSize.z; z++)
//    {
//        //take out entire plane
//        for(size_t y=0; y<m_renderCubeSize.y; y++)
//        {
//            for(size_t x=0; x<m_renderCubeSize.x; x++)
//            {
//                releaseChunkInfo(_renderCube[indexTo]);
//                index+=dir.x;
//            }
//            index+=strideY;
//        }
//    }
//
//    size_t alignX=(dir.x)*copySize.x;
//    strideZ=(dir.z-dir.y)*m_renderCubeSize.y*m_renderCubeSize.x;
//    
//    for(size_t z=0; z<copySize.z; z++)
//    {
//        for(size_t y=0; y<releaseSize.y; y++)
//        {
//            for(size_t x=0; x<m_renderCubeSize.x; x++)
//            {
//                releaseChunkInfo(_renderCube[indexTo]);
//                index+=dir.x;
//            }
//            index+=strideY;
//        }
//
//        for(size_t y=0; y<copySize.y; y++)
//        {
//            for(size_t x=0; x<releaseSize.x; x++)
//            {
//                releaseChunkInfo(_renderCube[indexTo]);
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

    indexFrom=copyFrom.z*(m_renderCubeSize.y*m_renderCubeSize.x)+copyFrom.y*m_renderCubeSize.x+copyFrom.x;
    indexTo=copyTo.z*(m_renderCubeSize.y*m_renderCubeSize.x)+copyTo.y*m_renderCubeSize.x+copyTo.x;

    glm::ivec3 stride(dir.x, (dir.y-dir.x)*m_renderCubeSize.x, (dir.z-dir.y)*m_renderCubeSize.y*m_renderCubeSize.x);

    stride.y+=dir.x*(m_renderCubeSize.x-copySize.x);
    stride.z+=dir.y*(m_renderCubeSize.y-copySize.y)*m_renderCubeSize.x;

    //copy info
    for(size_t z=0; z<copySize.z; z++)
    {
        for(size_t y=0; y<copySize.y; y++)
        {
            for(size_t x=0; x<copySize.x; x++)
            {
//                assert(m_renderCube[indexTo] == nullptr);
                if(m_renderCube[indexTo])
                    m_renderCube[indexTo]->refCount--;

                m_renderCube[indexTo]=m_renderCube[indexFrom];
                m_renderCube[indexTo]->refCount++;

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
        regionStart.z=m_renderCubeSize.z-releaseSize.z;
    else
        regionStart.z=0;

    regionSize.x=m_renderCubeSize.x;
    regionSize.y=m_renderCubeSize.y;
    regionSize.z=releaseSize.z;

//    glm::ivec3 updateRegionIndex;
//    glm::ivec3 updateChunkIndex;
    Index updateIndex=Index::offset(index, (regionStart-m_renderCubeCenterIndex));
    getRegion(regionStart, updateIndex, regionSize);

    if(copyTo.z <= copyFrom.z)
        regionStart.z=0;
    else
        regionStart.z=releaseSize.z;

    regionSize.z=copySize.z;
    
    if(copyTo.y<copyFrom.y)
        regionStart.y=m_renderCubeSize.y-releaseSize.y;
    else
        regionStart.y=0;
    regionSize.y=releaseSize.y;

    updateIndex=Index::offset(index, (regionStart-m_renderCubeCenterIndex));
    getRegion(regionStart, updateIndex, regionSize);

    if(copyTo.y <= copyFrom.y)
        regionStart.y=0;
    else
        regionStart.y=releaseSize.y;

    regionSize.y=copySize.y;
    
    if(copyTo.x<=copyFrom.x)
        regionStart.x=m_renderCubeSize.x-releaseSize.x;
    else
        regionStart.x=0;
    regionSize.x=releaseSize.x;

    updateIndex=Index::offset(index, (regionStart-m_renderCubeCenterIndex));
    getRegion(regionStart, updateIndex, regionSize);

    m_index=index;

//    if(direction.x < 0)
//        start.x=
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::releaseRegion(const glm::ivec3 &start, const glm::ivec3 &size)
{
    size_t index=start.z*(m_renderCubeSize.y*m_renderCubeSize.x)+start.y*m_renderCubeSize.x+start.x;
    
    size_t strideX=m_renderCubeSize.x-size.x;
    size_t strideY=(m_renderCubeSize.y-size.y)*m_renderCubeSize.x;

    for(size_t z=0; z<size.z; z++)
    {
        for(size_t y=0; y<size.y; y++)
        {
            for(size_t x=0; x<size.x; x++)
            {
//                releaseChunkInfo(m_renderCube[index]);
                RendererType *renderer=m_renderCube[index];

                if(renderer)
                {
                    renderer->refCount--;
                    releaseInfo(renderer);
                }
                m_renderCube[index]=nullptr;
                index++;
            }
            index+=strideX;
        }
        index+=strideY;
    }
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::getRegion(const glm::ivec3 &start, const Index &startIndex, const glm::ivec3 &size)
{
    glm::ivec3 regionSize=m_descriptors->getRegionSize();
    size_t index=start.z*(m_renderCubeSize.y*m_renderCubeSize.x)+start.y*m_renderCubeSize.x+start.x;

    size_t strideX=m_renderCubeSize.x-size.x;
    size_t strideY=(m_renderCubeSize.y-size.y)*m_renderCubeSize.x;

//    glm::ivec3 regionIndex;
//    glm::ivec3 chunkIndex;
    Index renderIndex;

//    regionIndex.z=startRegionIndex.z;
//    chunkIndex.z=startChunkIndex.z;
    renderIndex.setZ(startIndex);

    for(size_t z=0; z<size.z; z++)
    {
//        regionIndex.y=startRegionIndex.y;
//        chunkIndex.y=startChunkIndex.y;
        renderIndex.setY(startIndex);

        for(size_t y=0; y<size.y; y++)
        {
//            regionIndex.x=startRegionIndex.x;
//            chunkIndex.x=startChunkIndex.x;
            renderIndex.setX(startIndex);

            for(size_t x=0; x<size.x; x++)
            {
                RendererType *renderer=getFreeRenderer();

                assert(renderer);

                if(m_renderCube[index])
                {
                    m_renderCube[index]->refCount--;
                    assert(m_renderCube[index]->refCount>0);//suppose to be refferenced by something else
                }

                if(renderer)
                {
                    m_renderCube[index]=renderer;
                    renderer->refCount++;


                    //up    date chunk
//                    Key key(m_descriptors->getRegionHash(regionIndex), m_descriptors->getChunkHash(chunkIndex));
//                    SharedChunkHandle chunkHandle=m_grid->getChunk(key);
                    typename Index::Handle handle=Index::getHandle(m_grid, renderIndex);

                    renderer->setAction(RenderAction::Idle);
                    renderer->setHandle(handle);
#ifdef DEBUG_RENDERERS
                    Log::debug("MainThread - ChunkRenderer %x (%d, %d) setChunk", renderer, renderer->getRegionHash(), renderer->getChunkHash());
#endif//DEBUG_RENDERERS

                    //                    m_renderCubeMap[key.hash]=index;
                    //                    renderInfo.chunkHandle=chunkHandle;

                                        //if not loaded, load it
                    if(handle->state()!=HandleState::Memory)
                    {
                        //make sure we are not already requesting something from it
                        if(handle->action()==HandleAction::Idle)
                            Index::load(m_grid, handle, 0);
                    }

                    
                }
                else
                    m_renderCube[index]=nullptr;

                index++;
                renderIndex.incX();
//                chunkIndex.x++;
//                if(chunkIndex.x>=regionSize.x)
//                {
//                    chunkIndex.x-=regionSize.x;
//                    regionIndex.x++;
//                }
            }

            index+=strideX;
            renderIndex.incY();
//            chunkIndex.y++;
//            if(chunkIndex.y>=regionSize.y)
//            {
//                chunkIndex.y-=regionSize.y;
//                regionIndex.y++;
//            }
        }

        index+=strideY;
        renderIndex.incZ();
//        chunkIndex.z++;
//        if(chunkIndex.z>=regionSize.z)
//        {
//            chunkIndex.z-=regionSize.z;
//            regionIndex.z++;
//        }
    }
}

//template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
//void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::updateRegion(glm::ivec3 &startRegionIndex, glm::ivec3 &startChunkIndex, glm::ivec3 &size)
//{
//    glm::ivec3 regionIndex;
//    glm::ivec3 chunkIndex;
//    size_t renderPos=0;
//
//    glm::ivec3 regionSize=m_descriptors->getRegionSize();
//    glm::ivec3 offset=m_renderCubeSize-size;
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
//                RendererType *renderer=m_renderCube[renderPos];
//                
//                //get rid of any existing info/chunks/renderers
//                releaseChunkInfo(renderer);
//                m_renderCube[renderPos]=nullptr;
//                
//                Key key(m_descriptors->getRegionHash(regionIndex), m_descriptors->getChunkHash(chunkIndex));
//                SharedChunkHandle chunkHandle=m_grid->getChunk(key);
//
////                m_renderCubeMap[key.hash]=renderPos;
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
//                //if we dont have a renderer go ahead and request it
//                if(renderInfo.renderer == nullptr)
//                {
////                    renderInfo.state=RenderState::RequestRenderer;
////                    m_renderPrepThread->requestRenderer(renderInfo.chunkHandle);
//                    renderInfo.renderer=m_rendererQueue.get([&](RendererType *renderer){renderer->build(); renderer->buildOutline(m_outlineInstanceId);});
//
//                    if(renderInfo.renderer)
//                        renderInfo.renderer->setChunk(chunkHandle);
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

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::draw()
{
    glm::ivec3 regionIndex=m_index.regionIndex()-(m_index.regionIndex()-m_cameraIndex.regionIndex());

    for(auto renderer:m_renderCube)
    {
        if(renderer)
        {
            glm::ivec3 regionOffset=renderer->getRegionIndex()-regionIndex;
            glm::ivec3 offset=regionOffset*renderer->getRegionCellSize();

            renderer->draw(offset);
        }
    }
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::drawInfo(const glm::mat4x4 &projectionViewMat)
{
    glm::ivec3 regionIndex=m_index.regionIndex()-(m_index.regionIndex()-m_cameraIndex.regionIndex());

    for(auto renderer:m_renderCube)
    {
        if(renderer)
        {
            glm::ivec3 regionOffset=renderer->getRegionIndex()-regionIndex;
            glm::ivec3 offset=regionOffset*renderer->getRegionCellSize();

            renderer->drawInfo(projectionViewMat, offset);
        }
    }
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::drawOutline()
{
    glm::ivec3 regionIndex=m_index.regionIndex()-(m_index.regionIndex()-m_cameraIndex.regionIndex());

    for(auto renderer:m_renderCube)
    {
        if(renderer)
        {
            glm::ivec3 regionOffset=renderer->getRegionIndex()-regionIndex;
            glm::ivec3 offset=regionOffset*renderer->getRegionCellSize();

            renderer->drawOutline(offset);
        }
    }
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
typename RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::RendererType *RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::getRenderInfo(const Index &index)
{
//    glm::ivec3 regionIndex=m_descriptors->getRegionIndex(key.regionHash);
//    glm::ivec3 chunkIndex=m_descriptors->getChunkIndex(key.chunkHash);
//    glm::ivec3 gridRegionSize=m_descriptors->getRegionSize();
//    glm::ivec3 offset=Index::difference(m_index, index, gridRegionSize);
    glm::ivec3 offset=_Index::difference(m_index, index);

    glm::ivec3 cubeIndex=m_renderCubeCenterIndex+offset; //0 index;

    if(cubeIndex.x<0)
        return nullptr;
    if(cubeIndex.x>=m_renderCubeSize.x)
        return nullptr;
    if(cubeIndex.y<0)
        return nullptr;
    if(cubeIndex.y>=m_renderCubeSize.y)
        return nullptr;
    if(cubeIndex.z<0)
        return nullptr;
    if(cubeIndex.z>=m_renderCubeSize.z)
        return nullptr;

    size_t renderIndex=cubeIndex.z*(m_renderCubeSize.y*m_renderCubeSize.x)+cubeIndex.y*m_renderCubeSize.x+cubeIndex.x;
    
#ifndef NDEBUG
//    if(m_renderCube[renderIndex])
//        assert(m_renderCube[renderIndex]->getChunkHandle()->key().hash==key.hash);
#endif//NDEBUG
    
    return m_renderCube[renderIndex];
//    auto iter=m_renderCubeMap.find(key.hash);
//
//    if(iter==m_renderCubeMap.end())
//        return nullptr;
//
//    return &m_renderCube[iter->second];
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::releaseInfo(RendererType *renderer)
{
    if(!renderer)
        return;

//    assert(renderer->getAction()==RenderAction::Idle);
    //we are dumping this renderer so chunk no longer need.
    if(renderer->getHandle()->action()!=HandleAction::Idle)
        Index::cancelLoad(m_grid, renderer->getHandle());

    //can only release the renderer if it is not busy
    if((renderer->getAction()==RenderAction::Idle) )//&& (renderer->getChunkHandle()->action()==HandleAction::Idle))
        releaseRenderer(renderer);
    else    
        m_rendererReleaseQueue.push_back(renderer);
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
typename RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::RendererType *RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::getFreeRenderer()
{
    //release any queued renderers
    if(!m_rendererReleaseQueue.empty())
    {
        for(auto iter=m_rendererReleaseQueue.begin(); iter!=m_rendererReleaseQueue.end(); )
        {
            RendererType *renderer=*iter;

            if((renderer->getAction() == RenderAction::Idle) )//&& (renderer->getChunkHandle()->action() == HandleAction::Idle))
            {
                releaseRenderer(renderer);
                iter=m_rendererReleaseQueue.erase(iter);
            }
            else
                ++iter;
        }
    }

    RendererType *renderer=m_rendererQueue.get([&](RendererType *renderer){renderer->build();});
    
    return renderer;
}

template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
void RenderCube<_Grid, _Renderer, _MeshHandler, _Index>::releaseRenderer(RendererType *renderer)
{
    MeshBuffer mesh=renderer->clearMesh();

    if(mesh.valid)
        m_meshHandler->releaseMesh(mesh);

    renderer->clear();
    m_rendererQueue.release(renderer);
}

}//namespace voxigen