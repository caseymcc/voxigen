#ifndef _voxigen_gridDescriptors_h_
#define _voxigen_gridDescriptors_h_

#include "voxigen/defines.h"
#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>
//#include <rapidjson/prettywriter.h>
//#include <rapidjson/filewritestream.h>
//#include <rapidjson/filereadstream.h>
//#include <rapidjson/document.h>

#include <string>

#pragma warning(push)
#pragma warning(disable:4251)

namespace voxigen
{

struct IGridDescriptors
{
    IGridDescriptors() {};
    virtual ~IGridDescriptors() {};

    virtual const char *getName() const=0;
    virtual void setName(const char *value)=0;

    virtual unsigned int getSeed() const=0;
    virtual void setSeed(unsigned int value)=0;

    virtual glm::ivec3 getSize() const=0;
    virtual void setSize(const glm::ivec3 &value)=0;

    virtual glm::ivec3 getRegionSize() const=0;
    virtual void setRegionSize(const glm::ivec3 &value)=0;

    virtual glm::ivec3 getRegionCellSize() const=0;
    
    virtual glm::ivec3 getRegionCount() const=0;

    virtual glm::ivec3 getRegionStride() const=0;

    virtual glm::ivec3 getChunkSize() const=0;
    virtual void setChunkSize(const glm::ivec3 &value)=0;

    virtual glm::ivec3 getChunkCount() const=0;

    virtual glm::ivec3 getChunkStride() const=0;

    virtual const char *getGenerator() const=0;
    virtual void setGenerator(const char *value)=0;

    virtual const char *getGeneratorDescriptors() const=0;
    virtual void setGeneratorDescriptors(const char *value)=0;
};

VOXIGEN_EXPORT bool loadDescriptors(IGridDescriptors *descriptors, const char *fileName);
VOXIGEN_EXPORT bool saveDescriptors(IGridDescriptors *descriptors, const char *fileName);

template<typename _Grid>
struct GridDescriptors:IGridDescriptors
{
    GridDescriptors();
    virtual ~GridDescriptors() {}
    
    void create(std::string name, int seed, const glm::ivec3 &size);
    bool load(std::string fileName);
    bool save(std::string fileName);

    void init();

    RegionHash regionHash(const glm::ivec3 &index) const;
    glm::ivec3 regionIndex(RegionHash hash) const;
    glm::ivec3 regionIndex(const glm::vec3 &pos) const;
    glm::vec3 regionOffset(RegionHash hash) const;

    glm::vec3 adjustRegion(glm::ivec3 &regionIndex, glm::ivec3 &chunkIndex) const;
    float distance(glm::ivec3 &regionIndex1, glm::ivec3 &chunkIndex1, glm::ivec3 &regionIndex2, glm::ivec3 &chunkIndex2) const;

    ChunkHash chunkHash(const glm::ivec3 &index) const;
    glm::ivec3 chunkIndex(ChunkHash hash) const;
    glm::vec3 chunkOffset(ChunkHash hash) const;

    std::string m_name;
    unsigned int m_seed;
    std::string m_generator;
    std::string m_generatorDescriptors;

    glm::ivec3 m_size;
    glm::ivec3 m_regionSize; //from compile time value
    glm::ivec3 m_regionCellSize; //calculated
    glm::ivec3 m_regionCount; //calculated
    glm::ivec3 m_regionStride; //calculated
    glm::ivec3 m_chunkSize; //from compile time value
    glm::ivec3 m_chunkCount; //calculated
    glm::ivec3 m_chunkStride; //calculated

//    float m_noiseScale;
//    float m_contientFrequency;
//    int m_contientOctaves;
//    float m_contientLacunarity;
//
//    float m_seaLevel;
//    float m_continentaShelf;

//IGridDescriptor Interface
    virtual const char *getName() const { return m_name.c_str(); }
    virtual void setName(const char *value) { m_name=value; }

    virtual unsigned int getSeed() const { return m_seed; }
    virtual void setSeed(unsigned int value) { m_seed=value; }

    virtual glm::ivec3 getSize() const { return m_size; }
    virtual void setSize(const glm::ivec3 &value) { m_size=value; }

    virtual glm::ivec3 getRegionSize() const { return m_regionSize; }
    virtual void setRegionSize(const glm::ivec3 &value) { m_regionSize=value; }

    virtual glm::ivec3 getRegionCellSize() const { return m_regionCellSize; }

    virtual glm::ivec3 getRegionCount() const { return m_regionCount; }

    virtual glm::ivec3 getRegionStride() const { return m_regionStride; }

    virtual glm::ivec3 getChunkSize() const { return m_chunkSize; }
    virtual void setChunkSize(const glm::ivec3 &value) { m_chunkSize=value; }

    virtual glm::ivec3 getChunkCount() const { return m_chunkCount; }

    virtual glm::ivec3 getChunkStride() const { return m_chunkStride; }

    virtual const char *getGenerator() const { return m_generator.c_str(); }
    virtual void setGenerator(const char *value) { m_generator=value; }

    virtual const char *getGeneratorDescriptors() const { return m_generatorDescriptors.c_str(); }
    virtual void setGeneratorDescriptors(const char *value) { m_generatorDescriptors=value; }
};

template<typename _Grid>
GridDescriptors<_Grid>::GridDescriptors()
{
    m_seed=0;

    m_size=glm::ivec3(1024, 1024, 256);
    m_regionSize=glm::ivec3(_Grid::RegionType::sizeX::value, _Grid::RegionType::sizeY::value, _Grid::RegionType::sizeZ::value);
    m_chunkSize=glm::ivec3(_Grid::ChunkType::sizeX::value, _Grid::ChunkType::sizeY::value, _Grid::ChunkType::sizeZ::value);

//    m_noiseScale=0.001;
//
//    //    contientFrequency=1.0;
//    m_contientFrequency=0.005;
//    m_contientOctaves=2;
//    m_contientLacunarity=2.2;
//
//    m_seaLevel=0.0f;
}

template<typename _Grid>
void GridDescriptors<_Grid>::create(std::string name, int seed, const glm::ivec3 &size)
{
    m_name=name;
    m_seed=seed;
    m_size=size;

    init();
}

template<typename _Grid>
bool GridDescriptors<_Grid>::load(std::string fileName)
{
    if(!loadDescriptors(this, fileName.c_str()))
        return false;
    init();
    return true;
}

template<typename _Grid>
bool GridDescriptors<_Grid>::save(std::string fileName)
{
    saveDescriptors(this, fileName.c_str());
    return true;
}

template<typename _Grid>
void GridDescriptors<_Grid>::init()
{
    m_regionCellSize=m_regionSize*m_chunkSize;
    m_regionCount=m_size/(m_regionCellSize);
    m_regionStride=glm::ivec3(1, m_regionCount.x, m_regionCount.x*m_regionCount.y);

    m_chunkCount=m_regionCellSize/m_chunkSize;
    m_chunkStride=glm::ivec3(1, m_chunkCount.x, m_chunkCount.x*m_chunkCount.y);
}

template<typename _Grid>
RegionHash GridDescriptors<_Grid>::regionHash(const glm::ivec3 &index) const
{
    return m_regionStride.x*index.x+m_regionStride.y*index.y+m_regionStride.z*index.z;
}

template<typename _Grid>
glm::ivec3 GridDescriptors<_Grid>::regionIndex(RegionHash hash) const
{
    glm::ivec3 index;

    index.z=hash/m_regionStride.z;
    hash=hash-(m_regionStride.z*index.z);
    index.y=hash/m_regionStride.y;
    index.x=hash-(m_regionStride.y*index.y);

    return index;
}

template<typename _Grid>
glm::ivec3 GridDescriptors<_Grid>::regionIndex(const glm::vec3 &pos) const
{
    glm::ivec3 position=glm::floor(pos);

    return position/m_regionCellSize;
}

template<typename _Grid>
glm::vec3 GridDescriptors<_Grid>::regionOffset(RegionHash hash) const
{
    glm::ivec3 index=regionIndex(hash);
    glm::vec3 offset=m_regionCellSize*index;

    return offset;
}

template<typename _Grid>
glm::vec3 GridDescriptors<_Grid>::adjustRegion(glm::ivec3 &regionIndex, glm::ivec3 &chunkIndex) const
{
    glm::ivec3 offset(0, 0, 0);

    if(chunkIndex.x<0)
    {
        offset.x--;
        chunkIndex.x=m_regionSize.x+chunkIndex.x;
    }
    else if(chunkIndex.x>=m_regionSize.x)
    {
        offset.x++;
        chunkIndex.x=chunkIndex.x-m_regionSize.x;
    }
    if(chunkIndex.y<0)
    {
        offset.y--;
        chunkIndex.y=m_regionSize.y+chunkIndex.y;
    }
    else if(chunkIndex.y>=m_regionSize.y)
    {
        offset.y++;
        chunkIndex.y=chunkIndex.y-m_regionSize.y;
    }
    if(chunkIndex.z<0)
    {
        offset.z--;
        chunkIndex.z=m_regionSize.z+chunkIndex.z;
    }
    else if(chunkIndex.z>=m_regionSize.z)
    {
        offset.z++;
        chunkIndex.z=chunkIndex.z-m_regionSize.z;
    }

    regionIndex+=offset;
    return glm::vec3(m_regionCellSize*offset);
}

template<typename _Grid>
float GridDescriptors<_Grid>::distance(glm::ivec3 &regionIndex1, glm::ivec3 &chunkIndex1, glm::ivec3 &regionIndex2, glm::ivec3 &chunkIndex2) const
{
    glm::ivec3 offset(0.0f, 0.0f, 0.0f);

    if(regionIndex1!=regionIndex2)
        offset=(regionIndex2-regionIndex1)*m_regionSize;

    glm::ivec3 chunkPos1=chunkIndex1*m_chunkSize;
    glm::ivec3 chunkPos2=(chunkIndex2+offset)*m_chunkSize;
    return glm::length(glm::vec3(chunkPos1-chunkPos2));
}

template<typename _Grid>
ChunkHash GridDescriptors<_Grid>::chunkHash(const glm::ivec3 &chunkIndex) const
{
    return (m_chunkStride.z*chunkIndex.z)+(m_chunkStride.y*chunkIndex.y)+chunkIndex.x;
    //    return (chunkStride.x*index.x)+(chunkStride.y*index.y)+index.z;
}

template<typename _Grid>
glm::ivec3 GridDescriptors<_Grid>::chunkIndex(ChunkHash chunkHash) const
{
    glm::ivec3 index;

    index.z=chunkHash/m_chunkStride.z;
    chunkHash=chunkHash-(m_chunkStride.z*index.z);
    index.y=chunkHash/m_chunkStride.y;
    index.x=chunkHash-(m_chunkStride.y*index.y);

    return index;
}

template<typename _Grid>
glm::vec3 GridDescriptors<_Grid>::chunkOffset(ChunkHash chunkHash) const
{
    glm::ivec3 index=chunkIndex(chunkHash);
    glm::vec3 offset=m_chunkSize*index;

    return offset;
}
#pragma warning(pop)

}//namespace voxigen

#endif //_voxigen_gridDescriptors_h_