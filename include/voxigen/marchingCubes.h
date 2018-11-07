#ifndef _voxigen_marchingCubes_h_
#define _voxigen_marchingCubes_h_

#include "voxigen/defines.h"
#include "voxigen/chunk.h"
#include "voxigen/chunkMesh.h"
#include "voxigen/faces.h"

#include <array>

namespace voxigen
{

template<typename _Cell, typename _Mesh, bool negY, bool posY, bool negX, bool posX>
void buildHeightMesh_cell(_Mesh &mesh, typename std::vector<Cell> &cells, size_t &index, glm::ivec3 &position, size_t stride)
{
    if(negX && negY)
    {
    }
    else if(negX)
    {
    }
    else if(negY)
}

template<typename _Mesh, typename _Cell, bool negY, bool posY>
void buildHeightMesh_x(_ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position, size_t stride)
{
    position.x=0;

    buildHeightMesh_cell<_Chunk, _ChunkMesh, negY, posY, false, true>(mesh, cells, index, position, stride);
    index++;
    position.x++;i

    for(size_t x=stride; x<_Chunk::sizeX::value-stride; x+=stride)
    {
        buildHeightMesh_cell<_Chunk, _ChunkMesh, negY, posY, true, true>(mesh, cells, index, position, stride);
        index++;
        position.x++;
    }

    buildHeightMesh_cell<_Chunk, _ChunkMesh, negY, posY, true, false>(mesh, cells, index, position, stride);
    index++;
}

template<typename _Mesh, typename _Cell>
void buildMesh(_Mesh &mesh, std::vector<_Cell> &cells, const glm::ivec2 &cellsSize, size_t lod)
{
    size_t stride=glm::pow(2, lod);
    glm::ivec2 size=cellsSize/stride;

    glm::ivec3 position(0, 0, 0);
    size_t index=0;
    
    buildHeightMesh_x<_Mesh, _Cell, false, true>(mesh, index, position, stride);
    for(size_t y=stride; y<cellsSize.y-stride; y+=stride)
    {
        for(size_t y=0; x<cellsSize.x; x+=stride)
        {
            buildHeightMesh_x<_Mesh, _Cell, true, true>(mesh, index, position, stride);
            index++;
        }
    }
    buildHeightMesh_x<_Mesh, _Cell, true, false>(mesh, index, position, stride);
}
#endif//SIMPLE_MESH

} //namespace voxigen

#endif //_voxigen_marchingCubes_h_