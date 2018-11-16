#ifndef _voxigen_renderCube_h_
#define _voxigen_renderCube_h

#include "voxigen/regularGrid.h"
#include "voxigen/freeQueue.h"
//#include "voxigen/renderPrepThread.h"

#include <memory>

namespace voxigen
{

//enum class RenderState
//{
//    Idle,
//    RequestRenderer,
//    Meshing,
//    Updating
//};
//
//template<typename _SharedChunkHandle, typename _ChunkRenderer>
//struct ChunkRenderInfo
//{
//    typedef _SharedChunkHandle SharedChunkHandle;
//    typedef _ChunkRenderer ChunkRenderer;
//
//    ChunkRenderInfo():state(RenderState::Idle), renderer(nullptr) {}
//
//    void release() { state=RenderState::Idle; chunkHandle.reset(); renderer=nullptr; }
//
//    RenderState state;
//    SharedChunkHandle chunkHandle;
//    ChunkRenderer *renderer;
//};

template<typename _Region>
struct RegionIndex
{
    typedef std::shared_ptr<RegionHandle<_Region>> Handle;

    RegionIndex(){}
    RegionIndex(const glm::ivec3 &index):index(index){}

    template<typename _Grid>
    static Handle getHandle(_Grid *grid, const RegionIndex<_Region> &index)
    {
        return grid->getRegion(index.index);
    }

    template<typename _Grid>
    static void load(_Grid *grid, Handle handle, size_t lod)
    {
        //lets at least go for 1+ lod
        grid->loadRegion(handle, lod>0?lod:1);
    }

    template<typename _Grid>
    static void cancelLoad(_Grid *grid, Handle handle)
    {
        grid->cancelLoadRegion(handle);
    }

    static glm::ivec3 difference(const RegionIndex &index1, const RegionIndex &index2)
    {
        return index2.index-index1.index;
    }

    static RegionIndex offset(const RegionIndex &startIndex, const glm::ivec3 &delta)
    {
        RegionIndex regionIndex;

        regionIndex.index=startIndex.index+delta;
        return regionIndex;
    }

    static glm::ivec3 cells(glm::ivec3 &offset)
    {
        return glm::ivec3(0, 0, 0);
    }

    glm::ivec3 regionIndex()
    {
        return index;
    }

    bool operator==(const RegionIndex &that) const
    {
        return (index==that.index);
    }

    void setX(const RegionIndex &setIndex)
    {
        index.x=setIndex.index.x;
    }

    void setY(const RegionIndex &setIndex)
    {
        index.y=setIndex.index.y;
    }

    void setZ(const RegionIndex &setIndex)
    {
        index.z=setIndex.index.z;
    }

    void incX()
    {
        index.x++;
    }

    void incY()
    {
        index.y++;
    }

    void incZ()
    {
        index.z++;
    }

    glm::ivec3 index;
};

template<typename _Region>
glm::ivec3 operator+(const RegionIndex<_Region> &value1, const RegionIndex<_Region> &value2)
{
    return value1.index+value2.index;
}

template<typename _Region>
glm::ivec3 operator+(const RegionIndex<_Region> &value1, const glm::ivec3 &value2)
{
    return value1.index+value2;
}
template<typename _Region>
glm::ivec3 operator-(const RegionIndex<_Region> &value1, const RegionIndex<_Region> &value2)
{
    return value1.index-value2.index;
}

template<typename _Region>
glm::ivec3 operator-(const RegionIndex<_Region> &value1, const glm::ivec3 &value2)
{
    return value1.index-value2;
}

template<typename _Region, typename _Chunk>
struct RegionChunkIndex
{
    typedef std::shared_ptr<ChunkHandle<_Chunk>> Handle;

    template<typename _Grid>
    static Handle getHandle(_Grid *grid, const RegionChunkIndex<_Region, _Chunk> &index)
    {
        return grid->getChunk(index.region, index.chunk);
    }

    template<typename _Grid>
    static void load(_Grid *grid, Handle handle, size_t lod)
    {
        grid->loadChunk(handle, lod);
    }

    template<typename _Grid>
    static void cancelLoad(_Grid *grid, Handle handle)
    {
        grid->cancelLoadChunk(handle);
    }

    static glm::ivec3 difference(const RegionChunkIndex &index, const RegionChunkIndex &index2)
    {
//        glm::ivec3 regionSize=details::regionSize<_Region>();

        return details::difference<_Region>(index.region, index.chunk, index2.region, index2.chunk);
    }

    static RegionChunkIndex offset(const RegionChunkIndex &startIndex, const glm::ivec3 &delta)
    {
        RegionChunkIndex rcIndex;

        details::offsetIndexes<_Region>(startIndex.region, startIndex.chunk, delta, rcIndex.region, rcIndex.chunk);
        return rcIndex;
    }

    static glm::ivec3 cells(glm::ivec3 &offset)
    {
        return offset*details::regionCellSize<_Region, _Chunk>();
    }

    glm::ivec3 regionIndex()
    {
        return region;
    }

    bool operator==(const RegionChunkIndex &that) const
    {
        if(region!=that.region)
            return false;
        if(chunk!=that.chunk)
            return false;
        return true;
    }

    void setX(const RegionChunkIndex &index)
    {
        region.x=index.region.x;
        chunk.x=index.chunk.x;
    }

    void setY(const RegionChunkIndex &index)
    {
        region.y=index.region.y;
        chunk.y=index.chunk.y;
    }

    void setZ(const RegionChunkIndex &index)
    {
        region.z=index.region.z;
        chunk.z=index.chunk.z;
    }

    void incX()
    {
        chunk.x++;
        if(chunk.x>=details::regionSize<_Region>().x)
        {
            chunk.x-=details::regionSize<_Region>().x;
            region.x++;
        }
    }

    void incY()
    {
        chunk.y++;
        if(chunk.y>=details::regionSize<_Region>().y)
        {
            chunk.y-=details::regionSize<_Region>().y;
            region.y++;
        }
    }

    void incZ()
    {
        chunk.z++;
        if(chunk.z>=details::regionSize<_Region>().z)
        {
            chunk.z-=details::regionSize<_Region>().z;
            region.z++;
        }
    }

    glm::ivec3 region;
    glm::ivec3 chunk;
};

template<typename _Region, typename _Chunk>
glm::ivec3 operator+(const RegionChunkIndex<_Region, _Chunk> &value1, const RegionChunkIndex<_Region, _Chunk> &value2)
{
    return value1.region+value2.region;
}

template<typename _Region, typename _Chunk>
glm::ivec3 operator+(const RegionChunkIndex<_Region, _Chunk> &value1, const glm::ivec3 &value2)
{
    return value1.region+value2;
}

template<typename _Region, typename _Chunk>
glm::ivec3 operator-(const RegionChunkIndex<_Region, _Chunk> &value1, const RegionChunkIndex<_Region, _Chunk> &value2)
{
    return value1.region-value2.region;
}

template<typename _Region, typename _Chunk>
glm::ivec3 operator-(const RegionChunkIndex<_Region, _Chunk> &value1, const glm::ivec3 &value2)
{
    return value1.region-value2;
}

/////////////////////////////////////////////////////////////////////////////////////////
//RenderCube
/////////////////////////////////////////////////////////////////////////////////////////
//template<typename _Grid, typename _ChunkRenderer>
template<typename _Grid, typename _Renderer, typename _MeshHandler, typename _Index>
class RenderCube//:public RegularGridTypes<_Grid>
{
public:
    typedef typename _Grid::GridType GridType;
    typedef typename _Grid::DescriptorType DescriptorType;
//    typedef typename _Grid::ChunkType ChunkType;
//    typedef typename _Grid::ChunkHandleType ChunkHandleType;
//    typedef typename _Grid::SharedChunkHandle SharedChunkHandle;

    typedef _Renderer RendererType;
    typedef std::vector<RendererType> Renderers;
//    typedef ChunkRenderInfo<SharedChunkHandle, _ChunkRenderer> ChunkRenderInfoType;

    typedef _MeshHandler MeshHandler;

    typedef _Index Index;

    RenderCube(GridType *grid, DescriptorType *descriptors, MeshHandler *meshHandler);
    ~RenderCube();

    void setViewRadius(const glm::ivec3 &radius);

    void init(const Index &index);
    void updateCamera(const Index &index);
    void update(const Index &index);

//    void draw();
    void draw();
    void drawInfo(const glm::mat4x4 &projectionViewMat);
    void drawOutline();

//    ChunkRenderInfoType *getChunkRenderInfo(const Key &key);
    RendererType *getRenderInfo(const Index &index);// const Key &key);

private:
    glm::ivec3 calcCubeSize(const glm::ivec3 &radius);

    _Renderer *getFreeRenderer();
    void releaseRenderer(_Renderer *renderer);

//    void updateRegion(glm::ivec3 &startRegionIndex, glm::ivec3 &startChunkIndex, glm::ivec3 &size);
    void releaseRegion(const glm::ivec3 &start, const glm::ivec3 &size);
//    void getRegion(const glm::ivec3 &start, const glm::ivec3 &startRegionIndex, const glm::ivec3 &startChunkIndex, const glm::ivec3 &size);
    void getRegion(const glm::ivec3 &start, const Index &startIndex, const glm::ivec3 &size);

//    void releaseChunkInfo(ChunkRenderInfoType &renderInfo);
    void releaseInfo(_Renderer *renderInfo);
    
    GridType *m_grid;
    const DescriptorType *m_descriptors;
    MeshHandler *m_meshHandler;

    glm::ivec3 m_viewRadius;

//    glm::ivec3 m_cameraRegionIndex;
//    glm::ivec3 m_cameraChunkIndex;
    Index m_cameraIndex;

    Index m_index;
//    glm::ivec3 m_regionIndex;
//    glm::ivec3 m_chunkIndex;

//    std::unordered_map<Key::Type, size_t> m_renderCubeMap;
//    std::vector<ChunkRenderInfoType> m_renderCube;
    std::vector<RendererType *> m_renderCube;
    glm::ivec3 m_renderCubeSize;
    glm::ivec3 m_renderCubeCenterIndex;

    std::vector<RendererType *>m_rendererReleaseQueue;
    FreeQueue<RendererType> m_rendererQueue;
};

}//namespace voxigen

#include "renderCube.inl"

#endif //_voxigen_renderCube_h