#ifndef _voxigen_regionChunkIndex_h_
#define _voxigen_regionChunkIndex_h_

namespace voxigen
{

template<typename _Region, typename _Chunk>
struct RegionChunkIndex
{
    typedef std::shared_ptr<ChunkHandle<_Chunk>> Handle;

    RegionChunkIndex():region(0, 0, 0), chunk(0, 0, 0) {}

    template<typename _Grid>
    static Handle getHandle(_Grid *grid, const RegionChunkIndex<_Region, _Chunk> &index)
    {
        return grid->getChunk(index.region, index.chunk);
    }

    template<typename _Grid>
    static bool load(_Grid *grid, Handle handle, size_t lod)
    {
        return grid->loadChunk(handle, lod);
    }

    template<typename _Grid>
    static bool cancelLoad(_Grid *grid, Handle handle)
    {
        return grid->cancelLoadChunk(handle.get());
    }

    template<typename _Grid>
    static glm::ivec3 difference(_Grid *grid, const RegionChunkIndex &index, const RegionChunkIndex &index2)
    {
        //        glm::ivec3 regionSize=details::regionSize<_Region>();
        return details::difference<_Region, true, true, false>(index.region, index.chunk, index2.region, index2.chunk, &grid->getRegionCount());
    }

    template<typename _Grid>
    static RegionChunkIndex offset(_Grid *grid, const RegionChunkIndex &startIndex, const glm::ivec3 &delta)
    {
        RegionChunkIndex rcIndex;

        details::offsetIndexes<_Region>(startIndex.region, startIndex.chunk, delta, rcIndex.region, rcIndex.chunk);
        details::wrap<true, true, false>(grid->getRegionCount(), rcIndex.region);

        return rcIndex;
    }

    static glm::ivec3 cells(glm::ivec3 &offset)
    {
        return offset*details::regionCellSize<_Region, _Chunk>();
    }

    const glm::ivec3 &regionIndex()
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

    template<typename _Grid>
    void incX(_Grid *grid)
    {
        chunk.x++;
        if(chunk.x>=details::regionSize<_Region>().x)
        {
            chunk.x-=details::regionSize<_Region>().x;
            region.x++;
        }
        details::wrapDim<0>(grid->getRegionCount(), region);
    }

    template<typename _Grid>
    void incY(_Grid *grid)
    {
        chunk.y++;
        if(chunk.y>=details::regionSize<_Region>().y)
        {
            chunk.y-=details::regionSize<_Region>().y;
            region.y++;
        }
        details::wrapDim<1>(grid->getRegionCount(), region);
    }

    template<typename _Grid>
    void incZ(_Grid *grid)
    {
        chunk.z++;
        if(chunk.z>=details::regionSize<_Region>().z)
        {
            chunk.z-=details::regionSize<_Region>().z;
            region.z++;
        }
//        details::wrapDim<2>(grid->getRegionCount(), region);
    }

    glm::ivec3 region;
    glm::ivec3 chunk;

    std::string pos()
    {
        size_t size=std::snprintf(nullptr, 0, "(%d, %d, %d) (%d, %d, %d)", region.x, region.y, region.z, chunk.x, chunk.y, chunk.z);
        std::string value(size, 0);
        std::snprintf(&value[0], size+1, "(%d, %d, %d) (%d, %d, %d)", region.x, region.y, region.z, chunk.x, chunk.y, chunk.z);
        return value;
    }
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

}//namespace voxigen

#endif //_voxigen_regionChunkIndex_h_