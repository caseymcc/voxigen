#ifndef _voxigen_regionIndex_h_
#define _voxigen_regionIndex_h_

namespace voxigen
{

template<typename _Region>
struct RegionIndex
{
    typedef std::shared_ptr<RegionHandle<_Region>> Handle;

    RegionIndex():index(0,0,0){}
    RegionIndex(const glm::ivec3 &index):index(index) {}

    template<typename _Grid>
    static Handle getHandle(_Grid *grid, const RegionIndex<_Region> &index)
    {
        return grid->getRegion(index.index);
    }

    template<typename _Grid>
    static bool load(_Grid *grid, Handle handle, size_t lod)
    {
        //lets at least go for 1+ lod
        return grid->loadRegion(handle.get(), lod>0?lod:1);
    }

    template<typename _Grid>
    static bool cancelLoad(_Grid *grid, Handle handle)
    {
        return grid->cancelLoadRegion(handle.get());
    }

    template<typename _Grid>
    static glm::ivec3 difference(_Grid *grid, const RegionIndex &index1, const RegionIndex &index2)
    {
        return details::difference<_Region, true, true, false>(index1.index, index2.index, &grid->getRegionCount());
    }

    template<typename _Grid>
    static RegionIndex offset(_Grid *grid, const RegionIndex &startIndex, const glm::ivec3 &delta)
    {
        RegionIndex regionIndex;

        regionIndex.index=startIndex.index+delta;
        details::wrap<true, true, false>(grid->getRegionCount(), regionIndex.index);
        return regionIndex;
    }

    static glm::ivec3 cells(glm::ivec3 &offset)
    {
        return glm::ivec3(0, 0, 0);
    }

    const glm::ivec3 &regionIndex()
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

    template<typename _Grid>
    void incX(_Grid *grid)
    {
        index.x++;
        details::wrapDim<0>(grid->getRegionCount(), index);
    }

    template<typename _Grid>
    void incY(_Grid *grid)
    {
        index.y++;
        details::wrapDim<1>(grid->getRegionCount(), index);
    }

    template<typename _Grid>
    void incZ(_Grid *grid)
    {
        index.z++;
//        details::wrapDim<2>(grid->getRegionCount(), index);
    }

    std::string pos()
    {
        size_t size=std::snprintf(nullptr, 0, "(%d, %d, %d)", index.x, index.y, index.z);
        std::string value(size, 0);
        std::snprintf(&value[0], size+1, "(%d, %d, %d)", index.x, index.y, index.z);
        return value;
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

}//namespace voxigen

#endif //_voxigen_regionIndex_h_