#ifndef _voxigen_chunk_h_
#define _voxigen_chunk_h_

#include "voxigen/defines.h"
//#include "voxigen/boundingBox.h"
#include "voxigen/gridDescriptors.h"

#include <vector>
#include <memory>
#include <type_traits>

namespace voxigen
{

template<typename _Cell, size_t _x, size_t _y, size_t _z>
class Chunk//:public BoundingBox
{
public:
    Chunk(ChunkHash hash, unsigned int revision, const glm::ivec3 &index, glm::vec3 gridOffset);

    typedef std::vector<_Cell> Cells;
    typedef _Cell CellType;
    typedef std::integral_constant<size_t, _x> sizeX;
    typedef std::integral_constant<size_t, _y> sizeY;
    typedef std::integral_constant<size_t, _z> sizeZ;
    
    ChunkHash getHash() const { return m_hash; }
    Cells &getCells() { return m_cells; }
    
    unsigned int validCellCount() { return m_validCells; }
    void setValidCellCount(unsigned int count) { m_validCells=count; }
    
    const glm::ivec3 &getIndex() const{ return m_index; }
    const glm::vec3 &getGridOffset() const { return m_gridOffset; }

    _Cell &getCell(const glm::vec3 &position);

private:
    ChunkHash m_hash; //unique id used to look up chunk in region
    unsigned int m_revision; //incremented as changes are made

    Cells m_cells; //block info
    glm::ivec3 m_index; //grid index
    glm::vec3 m_gridOffset; //offset in grid coords
    unsigned int m_validCells;
    
};

template<typename _Cell, size_t _x, size_t _y, size_t _z>
using UniqueChunk=std::unique_ptr<Chunk<_Cell, _x, _y, _z>>;


template<typename _Cell, size_t _x, size_t _y, size_t _z>
Chunk<_Cell, _x, _y, _z>::Chunk(ChunkHash hash, unsigned int revision, const glm::ivec3 &index, glm::vec3 gridOffset):
//BoundingBox(dimensions, transform),
m_hash(hash),
m_revision(revision),
m_index(index),
m_gridOffset(gridOffset),
m_validCells(0)
{
    m_cells.resize(_x*_y*_z);
}

template<typename _Cell, size_t _x, size_t _y, size_t _z>
_Cell &Chunk<_Cell, _x, _y, _z>::getCell(const glm::vec3 &position)
{
    glm::ivec3 cellPos=glm::floor(position);
    unsigned int index=(_x*_y)*cellPos.z+_x*cellPos.y+cellPos.x;

    assert(index>=0);
    assert(index<m_cells.size());
    return m_cells[index];
}

} //namespace voxigen

#endif //_voxigen_chunk_h_