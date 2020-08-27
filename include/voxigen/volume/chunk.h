#ifndef _voxigen_chunk_h_
#define _voxigen_chunk_h_

#include "voxigen/defines.h"
//#include "voxigen/boundingBox.h"
#include "voxigen/volume/gridDescriptors.h"

#include <vector>
#include <memory>
#include <type_traits>

#ifdef DEBUG_ALLOCATION
#include "voxigen/fileio/log.h"
#endif//DEBUG_ALLOCATION

namespace voxigen
{

template<typename _Cell, size_t _x, size_t _y, size_t _z>
class Chunk//:public BoundingBox
{
public:
    Chunk();
    Chunk(ChunkHash hash, unsigned int revision, const glm::ivec3 &index, glm::vec3 gridOffset, size_t lod);
    ~Chunk();

    typedef std::vector<_Cell> Cells;
    typedef _Cell CellType;
    typedef std::integral_constant<size_t, _x> sizeX;
    typedef std::integral_constant<size_t, _y> sizeY;
    typedef std::integral_constant<size_t, _z> sizeZ;
    
    void setChunk(ChunkHash hash, const glm::ivec3 &index, const glm::vec3 gridOffset);
    void allocate(size_t lod);

    ChunkHash getHash() const { return m_hash; }
    Cells &getCells() { return m_cells; }
    
    unsigned int validCellCount() { return m_validCells; }
    void setValidCellCount(unsigned int count) { m_validCells=count; }
    
    const glm::ivec3 &getIndex() const{ return m_index; }
    const glm::vec3 &getGridOffset() const { return m_gridOffset; }
    size_t getLod() { return m_lod; }

    _Cell &getCell(const glm::vec3 &position);

    bool hasNeighbors() { return m_hasNeighbors; }
    std::vector<Chunk *> &getNeighbors() { return m_neighbors; }

private:
    ChunkHash m_hash; //unique id used to look up chunk in region
    unsigned int m_revision; //incremented as changes are made

    Cells m_cells; //block info
    glm::ivec3 m_index; //grid index
    glm::vec3 m_gridOffset; //offset in grid coords
    size_t m_lod;
    unsigned int m_validCells;

    bool m_hasNeighbors;
    std::vector<Chunk *> m_neighbors;
};

template<typename _Cell, size_t _x, size_t _y, size_t _z>
using UniqueChunk=std::unique_ptr<Chunk<_Cell, _x, _y, _z>>;


template<typename _Cell, size_t _x, size_t _y, size_t _z>
Chunk<_Cell, _x, _y, _z>::Chunk():
    m_hash(0),
    m_revision(0),
    m_validCells(0),
    m_lod(0),
    m_hasNeighbors(false),
    m_neighbors(9)
{
}

template<typename _Cell, size_t _x, size_t _y, size_t _z>
Chunk<_Cell, _x, _y, _z>::Chunk(ChunkHash hash, unsigned int revision, const glm::ivec3 &index, glm::vec3 gridOffset, size_t lod):
    //BoundingBox(dimensions, transform),
    m_hash(hash),
    m_revision(revision),
    m_index(index),
    m_gridOffset(gridOffset),
    m_validCells(0),
    m_lod(lod),
    m_hasNeighbors(false),
    m_neighbors(9)
{
    size_t size=(_x*_y*_z)/(lod+1);
    m_cells.resize(size);

    MEMORY_CHECK
    std::fill(m_neighbors.begin(), m_neighbors.end(), nullptr);
    MEMORY_CHECK
    
#ifdef DEBUG_ALLOCATION
    Log::debug("Chunk::Chunk %llx hash:%d allocate cells - data %llx size %d\n", this, m_hash, m_cells.data(), size);
#endif
}

template<typename _Cell, size_t _x, size_t _y, size_t _z>
Chunk<_Cell, _x, _y, _z>::~Chunk()
{
#ifdef DEBUG_ALLOCATION
    Log::debug("Chunk::~Chunk %llx hash:%d  free cells - data %llx\n", this, m_hash, m_cells.data());
#endif
};

template<typename _Cell, size_t _x, size_t _y, size_t _z>
void Chunk<_Cell, _x, _y, _z>::setChunk(ChunkHash hash, const glm::ivec3 &index, const glm::vec3 gridOffset)
{
    m_hash=hash;
    m_index=index;
    m_gridOffset=gridOffset;
}

template<typename _Cell, size_t _x, size_t _y, size_t _z>
void Chunk<_Cell, _x, _y, _z>::allocate(size_t lod)
{
    m_lod=lod;
    size_t size=(_x*_y*_z)/(m_lod+1);
    m_cells.resize(size);
}

template<typename _Cell, size_t _x, size_t _y, size_t _z>
_Cell &Chunk<_Cell, _x, _y, _z>::getCell(const glm::vec3 &position)
{
    size_t lod=m_lod+1;
    size_t x=_x/lod;
    size_t y=_y/lod;

    glm::ivec3 cellPos=glm::ivec3(glm::floor(position))/glm::ivec3(lod);
    unsigned int index=(x*y)*cellPos.z+x*cellPos.y+cellPos.x;

    assert(index>=0);
    assert(index<m_cells.size());
    return m_cells[index];
}

} //namespace voxigen

#endif //_voxigen_chunk_h_