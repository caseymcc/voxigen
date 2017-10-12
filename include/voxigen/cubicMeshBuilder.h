#ifndef _voxigen_cubicMeshBuilder_h_
#define _voxigen_cubicMeshBuilder_h_

#include "voxigen/defines.h"
#include "voxigen/chunk.h"
#include "voxigen/chunkMesh.h"

#include <array>

namespace voxigen
{

namespace Face
{

const std::array<uint8_t, 12> xNeg
{
    0, 0, 0,
    0, 0, 1,
    0, 1, 1,
    0, 1, 0
};

const std::array<uint8_t, 12> xPos
{
    1, 0, 1,
    1, 0, 0,
    1, 1, 0,
    1, 1, 1
};

const std::array<uint8_t, 12> zNeg
{
    1, 0, 0,
    0, 0, 0,
    0, 1, 0,
    1, 1, 0
};

const std::array<uint8_t, 12> zPos
{
    0, 0, 1,
    1, 0, 1,
    1, 1, 1,
    0, 1, 1
};

const std::array<uint8_t, 12> yNeg
{
    0, 0, 0,
    1, 0, 0,
    1, 0, 1,
    0, 0, 1
};

const std::array<uint8_t, 12> yPos
{
    0, 1, 1,
    1, 1, 1,
    1, 1, 0,
    0, 1, 0
};

}//namespace Face


template<typename _Chunk, bool _XNegFace=true, bool _XPosFace=true, bool _YNegFace=true, bool _YPosFace=true, bool _ZNegFace=true, bool _ZPosFace=true>
void checkCell(ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position)
{
    _Chunk::CellType &cell=cells[index];

    if(empty(cell))
        return;

    unsigned int cellType=type(cell);

    if(_XNegFace)
    {
        size_t adjIndex=index-1;

        if(empty(cells[adjIndex]))
            mesh.addFace(Face::xNeg, position, cellType);
    }
    else
        mesh.addFace(Face::xNeg, position, cellType);

    if(_XPosFace)
    {
        size_t adjIndex=index+1;

        if(empty(cells[adjIndex]))
            mesh.addFace(Face::xPos, position, cellType);
    }
    else
        mesh.addFace(Face::xPos, position, cellType);

    if(_YNegFace)
    {
        size_t adjIndex=index-_Chunk::sizeX::value;

        if(empty(cells[adjIndex]))
            mesh.addFace(Face::yNeg, position, cellType);
    }
    else
        mesh.addFace(Face::yNeg, position, cellType);

    if(_YPosFace)
    {
        size_t adjIndex=index+_Chunk::sizeX::value;

        if(empty(cells[adjIndex]))
            mesh.addFace(Face::yPos, position, cellType);
    }
    else
        mesh.addFace(Face::yPos, position, cellType);

    if(_ZNegFace)
    {
        size_t adjIndex=index-(_Chunk::sizeX::value*_Chunk::sizeY::value);

        if(empty(cells[adjIndex]))
            mesh.addFace(Face::zNeg, position, cellType);
    }
    else
        mesh.addFace(Face::zNeg, position, cellType);


    if(_ZPosFace)
    {
        size_t adjIndex=index+(_Chunk::sizeX::value*_Chunk::sizeY::value);

        if(empty(cells[adjIndex]))
            mesh.addFace(Face::zPos, position, cellType);
    }
    else
        mesh.addFace(Face::zPos, position, cellType);

}

template<typename _Chunk, bool _YNegFace=true, bool _YPosFace=true, bool _ZNegFace=true, bool _ZPosFace=true>
void checkX(ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position)
{
    position.x=0;

    checkCell<_Chunk, false, true, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace>(mesh, cells, index, position);
    index++;
    position.x+=1;

    for(size_t x=1; x<_Chunk::sizeX::value-1; x++)
    {
        checkCell<_Chunk, true, true, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace>(mesh, cells, index, position);
        index++;
        position.x+=1;
    }

    checkCell<_Chunk, true, false, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace>(mesh, cells, index, position);
    index++;
}

template<typename _Chunk, bool _ZNegFace=true, bool _ZPosFace=true>
void checkY(ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position)
{
    position.y=0.0;

    checkX<_Chunk, false, true, _ZNegFace, _ZPosFace>(mesh, cells, index, position);
    position.y+=1;

    for(size_t y=1; y<_Chunk::sizeY::value-1; y++)
    {
        position.x=0;

        checkX<_Chunk, true, true, _ZNegFace, _ZPosFace>(mesh, cells, index, position);
        position.y+=1;
    }

    checkX<_Chunk, true, false, _ZNegFace, _ZPosFace>(mesh, cells, index, position);
}

template<typename _Chunk>
void buildCubicMesh(ChunkMesh &mesh, _Chunk *chunk)
{
    _Chunk::Cells &cells=chunk->getCells();
    glm::ivec3 position(0, 0, 0);

    size_t index=0;

    checkY<_Chunk, false, true>(mesh, cells, index, position);
    position.z+=1;

    for(size_t z=1; z<_Chunk::sizeZ::value-1; z++)
    {
        checkY<_Chunk, true, true>(mesh, cells, index, position);
        position.z+=1;
    }
    
    checkY<_Chunk, true, false>(mesh, cells, index, position);
    position.z+=1;
}

} //namespace voxigen

#endif //_voxigen_cubicMeshBuilder_h_