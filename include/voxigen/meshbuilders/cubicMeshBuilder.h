#ifndef _voxigen_cubicMeshBuilder_h_
#define _voxigen_cubicMeshBuilder_h_

#include "voxigen/defines.h"
#include "voxigen/volume/chunk.h"
#include "voxigen/meshes/chunkMesh.h"
#include "voxigen/meshes/faces.h"
#include "voxigen/indexing.h"

#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/integer.hpp>

namespace voxigen
{
//#define SIMPLE_MESH

#ifdef SIMPLE_MESH
template<typename _Chunk, bool _XNegFace=true, bool _XPosFace=true, bool _YNegFace=true, bool _YPosFace=true, bool _ZNegFace=true, bool _ZPosFace=true, bool _Neighbors=false>
void checkCell(ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position, _Chunk *neighbor=nullptrx)
{
    _Chunk::CellType &cell=cells[index];

    if(empty(cell))
        return;

    unsigned int cellType=type(cell);

    if(_XNegFace)
    {
        size_t adjIndex=index-1;

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::xNeg], position, cellType);
    }
    else
    {
        if(_Neighbors)
        {
            _Chunk::Cells &neighborCells=neighbor->getCells();
            glm::ivec3 coord=chunkIndexToCoords<_Chunk>(index);

            coord.x=_Chunk::sizeX::value-1;
            size_t neighborIndex=chunkCoordsToIndex(coord);

            if(empty(neighborCells[neighborIndex]))
                mesh.addFace(faces[Face::xNeg], position, cellType);
        }
        else
            mesh.addFace(faces[Face::xNeg], position, cellType);
    }

    if(_XPosFace)
    {
        size_t adjIndex=index+1;

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::xPos], position, cellType);
    }
    else
    {
        if(_Neighbors)
        {
            _Chunk::Cells &neighborCells=neighbor->getCells();
            glm::ivec3 coord=chunkIndexToCoords<_Chunk>(index);

            coord.x=0;
            size_t neighborIndex=chunkCoordsToIndex(coord);

            if(empty(neighborCells[neighborIndex]))
                mesh.addFace(faces[Face::xNeg], position, cellType);
        }
        else
            mesh.addFace(faces[Face::xPos], position, cellType);
    }

    if(_YNegFace)
    {
        size_t adjIndex=index-_Chunk::sizeX::value;

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::yNeg], position, cellType);
    }
    else
    {
        if(_Neighbors)
        {
            _Chunk::Cells &neighborCells=neighbor->getCells();
            glm::ivec3 coord=chunkIndexToCoords<_Chunk>(index);

            coord.y=_Chunk::sizeY::value-1;
            size_t neighborIndex=chunkCoordsToIndex(coord);

            if(empty(neighborCells[neighborIndex]))
                mesh.addFace(faces[Face::xNeg], position, cellType);
        }
        else
            mesh.addFace(faces[Face::yNeg], position, cellType);
    }

    if(_YPosFace)
    {
        size_t adjIndex=index+_Chunk::sizeX::value;

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::yPos], position, cellType);
    }
    else
    {
        if(_Neighbors)
        {
            _Chunk::Cells &neighborCells=neighbor->getCells();
            glm::ivec3 coord=chunkIndexToCoords<_Chunk>(index);

            coord.y=0;
            size_t neighborIndex=chunkCoordsToIndex(coord);

            if(empty(neighborCells[neighborIndex]))
                mesh.addFace(faces[Face::xNeg], position, cellType);
        }
        else
            mesh.addFace(faces[Face::yPos], position, cellType);
    }

    if(_ZNegFace)
    {
        size_t adjIndex=index-(_Chunk::sizeX::value*_Chunk::sizeY::value);

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::zNeg], position, cellType);
    }
    else
    {
        if(_Neighbors)
        {
            _Chunk::Cells &neighborCells=neighbor->getCells();
            glm::ivec3 coord=chunkIndexToCoords<_Chunk>(index);

            coord.z=_Chunk::sizeZ::value-1;
            size_t neighborIndex=chunkCoordsToIndex(coord);

            if(empty(neighborCells[neighborIndex]))
                mesh.addFace(faces[Face::xNeg], position, cellType);
        }
        else
            mesh.addFace(faces[Face::zNeg], position, cellType);
    }


    if(_ZPosFace)
    {
        size_t adjIndex=index+(_Chunk::sizeX::value*_Chunk::sizeY::value);

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::zPos], position, cellType);
    }
    else
    {
        if(_Neighbors)
        {
            _Chunk::Cells &neighborCells=neighbor->getCells();
            glm::ivec3 coord=chunkIndexToCoords<_Chunk>(index);

            coord.z=0;
            size_t neighborIndex=chunkCoordsToIndex(coord);

            if(empty(neighborCells[neighborIndex]))
                mesh.addFace(faces[Face::xNeg], position, cellType);
        }
        else
            mesh.addFace(faces[Face::zPos], position, cellType);
    }

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
    position.y=0;

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
//    position.z+=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename _Chunk>
constexpr size_t indicesSize()
{
    return (_Chunk::sizeX::value*_Chunk::sizeY::value*_Chunk::sizeZ::value)*6*6; //6 faces, 2 triangles
};

template<typename _Chunk>
constexpr size_t verticesSize()
{
    return (_Chunk::sizeX::value+1)*(_Chunk::sizeY::value+1)*(_Chunk::sizeZ::value+1);
};

struct Vertex
{
    uint8_t x, y, z;
};

struct Mesh
{
    Mesh():vertices(nullptr), verticesSize(0), verticesCapacity(0), indices(nullptr), indicesSize(0), indicesCapacity(0), scratch(nullptr), scratchCapacity(0){}
    ~Mesh() 
    {
        if(vertices!=nullptr)
            delete vertices;
        if(indices!=nullptr)
            delete indices;
        if(scratch!=nullptr)
            delete scratch;
    }

    Vertex *vertices;
    size_t verticesSize;
    size_t verticesCapacity;
    unsigned int *indices;
    size_t indicesSize;
    size_t indicesCapacity;

    unsigned int *scratch;
    size_t scratchCapacity;
};

template<typename _Chunk>
size_t vertexToIndex(const glm::ivec3 &pos, size_t stride)
{
    glm::ivec3 size(_Chunk::sizeX::value/stride+1, _Chunk::sizeY::value/stride+1, _Chunk::sizeZ::value/stride+1);

    return (pos.z*size.x*size.y)+(pos.y*size.x)+pos.x;
    //return (pos.z*(_Chunk::sizeX::value+1)*(_Chunk::sizeY::value+1))+(pos.y*(_Chunk::sizeX::value+1))+pos.x;
}

template<typename _Chunk>
Vertex indexToVertex(size_t index, size_t stride)
{
    Vertex pos;

    glm::ivec3 size(_Chunk::sizeX::value/stride+1, _Chunk::sizeY::value/stride+1, _Chunk::sizeZ::value/stride+1);

    pos.z=index/(size.x*size.y);
    index-=pos.z*(size.x*size.y);
    pos.y=index/(size.x);
    pos.x=index-(pos.y*(size.x));
    
    pos.x*=stride;
    pos.y*=stride;
    pos.z*=stride;

//    pos.z=index/((_Chunk::sizeX::value+1)*(_Chunk::sizeY::value+1));
//    index-=pos.z*((_Chunk::sizeX::value+1)*(_Chunk::sizeY::value+1));
//    pos.y=index/(_Chunk::sizeX::value+1);
//    pos.x=index-(pos.y*(_Chunk::sizeX::value+1));

    return pos;
}

template<typename _Chunk, typename _ChunkMesh>
void addFace(_ChunkMesh &mesh, size_t face, const glm::ivec3 &position, unsigned int cellType, size_t stride)
{
//    glm::ivec3 pos;
//    unsigned int vertIndex[4];

//    auto faceQuad=faces[face];
    auto faceQuad=facesWithNormal[face];

    for(size_t i=0; i<4; ++i)
    {
        faceQuad[i]=(faceQuad[i]+position)*(int)stride;
//        pos=faceQuad[i]+position;
//        vertIndex[i]=vertexToIndex<_Chunk>(pos, stride);
    }

//    unsigned int *indices=&mesh.indices[mesh.indicesSize];
//
//    indices[0]=vertIndex[0];
//    indices[1]=vertIndex[1];
//    indices[2]=vertIndex[2];
//    indices[3]=vertIndex[2];
//    indices[4]=vertIndex[3];
//    indices[5]=vertIndex[0];
//
//    mesh.indicesSize+=6;
    mesh.addFace(face, cellType, position, faceQuad);
}

template<typename _Chunk, typename _ChunkMesh, bool _XNegFace=true, bool _XPosFace=true, bool _YNegFace=true, bool _YPosFace=true, bool _ZNegFace=true, bool _ZPosFace=true>
void checkCell(_ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position, size_t stride)
{
    typename _Chunk::CellType &cell=cells[index];

    if(empty(cell))
        return;

    unsigned int cellType=type(cell);

    if(_XNegFace)
    {
        size_t adjIndex=index-1;

        if(empty(cells[adjIndex]))
            addFace<_Chunk, _ChunkMesh>(mesh, Face::xNeg, position, cellType, stride);
    }
    else
        addFace<_Chunk, _ChunkMesh>(mesh, Face::xNeg, position, cellType, stride);

    if(_XPosFace)
    {
        size_t adjIndex=index+1;

        if(empty(cells[adjIndex]))
            addFace<_Chunk, _ChunkMesh>(mesh, Face::xPos, position, cellType, stride);
    }
    else
        addFace<_Chunk, _ChunkMesh>(mesh, Face::xPos, position, cellType, stride);

    if(_YNegFace)
    {
        size_t adjIndex=index-(_Chunk::sizeX::value/stride);

        if(empty(cells[adjIndex]))
            addFace<_Chunk, _ChunkMesh>(mesh, Face::yNeg, position, cellType, stride);
    }
    else
        addFace<_Chunk, _ChunkMesh>(mesh, Face::yNeg, position, cellType, stride);

    if(_YPosFace)
    {
        size_t adjIndex=index+(_Chunk::sizeX::value/stride);

        if(empty(cells[adjIndex]))
            addFace<_Chunk, _ChunkMesh>(mesh, Face::yPos, position, cellType, stride);
    }
    else
        addFace<_Chunk, _ChunkMesh>(mesh, Face::yPos, position, cellType, stride);

    if(_ZNegFace)
    {
        size_t adjIndex=index-(_Chunk::sizeX::value*_Chunk::sizeY::value/(stride*stride));

        if(empty(cells[adjIndex]))
            addFace<_Chunk, _ChunkMesh>(mesh, Face::zNeg, position, cellType, stride);
    }
    else
        addFace<_Chunk, _ChunkMesh>(mesh, Face::zNeg, position, cellType, stride);


    if(_ZPosFace)
    {
        size_t adjIndex=index+(_Chunk::sizeX::value*_Chunk::sizeY::value/(stride*stride));

        if(empty(cells[adjIndex]))
            addFace<_Chunk, _ChunkMesh>(mesh, Face::zPos, position, cellType, stride);
    }
    else
        addFace<_Chunk, _ChunkMesh>(mesh, Face::zPos, position, cellType, stride);

}

template<typename _Chunk, typename _ChunkMesh, bool _YNegFace=true, bool _YPosFace=true, bool _ZNegFace=true, bool _ZPosFace=true>
void checkX(_ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position, size_t stride)
{
    position.x=0;

    checkCell<_Chunk, _ChunkMesh, false, true, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride);
    index++;
    position.x++;

    for(size_t x=stride; x<_Chunk::sizeX::value-stride; x+=stride)
    {
        checkCell<_Chunk, _ChunkMesh, true, true, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride);
        index++;
        position.x++;
    }

    checkCell<_Chunk, _ChunkMesh, true, false, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride);
    index++;
}

template<typename _Chunk, typename _ChunkMesh, bool _ZNegFace=true, bool _ZPosFace=true>
void checkY(_ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position, size_t stride)
{
    position.y=0;

    checkX<_Chunk, _ChunkMesh, false, true, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride);
    position.y++;

    for(size_t y=stride; y<_Chunk::sizeY::value-stride; y+=stride)
    {
        position.x=0;

        checkX<_Chunk, _ChunkMesh, true, true, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride);
        position.y++;
    }

    checkX<_Chunk, _ChunkMesh, true, false, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride);
}



template<typename _Chunk, typename _ChunkMesh>
void buildCubicMesh(_ChunkMesh &mesh, _Chunk *chunk)
{
    size_t stride=glm::pow(2u, (unsigned int)chunk->getLod());
    glm::ivec3 size(_Chunk::sizeX::value/stride, _Chunk::sizeY::value/stride, _Chunk::sizeZ::value/stride);

    const int requiredScratchSize=(size.x+1)*(size.y+1)*(size.z+1);
    const int requiredIndices=(size.x*size.y*size.z)*6*4;
    const int requiredVertices=requiredScratchSize;

    typename _Chunk::Cells &cells=chunk->getCells();
    glm::ivec3 position(0, 0, 0);

    size_t index=0;

    checkY<_Chunk, _ChunkMesh, false, true>(mesh, cells, index, position, stride);
    position.z++;

    for(size_t z=stride; z<_Chunk::sizeZ::value-stride; z+=stride)
    {
        checkY<_Chunk, _ChunkMesh, true, true>(mesh, cells, index, position, stride);
        position.z++;
    }

    checkY<_Chunk, _ChunkMesh, true, false>(mesh, cells, index, position, stride);


#endif//SIMPLE_MESH


////////////////////////////////////////////////////////////////////////////////////////////////////////
//Neighbor check
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename _Chunk, typename _ChunkMesh, bool _YNegFace=true, bool _YPosFace=true, bool _ZNegFace=true, bool _ZPosFace=true>
void checkX(_ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position, size_t stride, std::vector<_Chunk *> *neighbors=nullptr)
{
    position.x=0;

    checkCell<_Chunk, _ChunkMesh, false, true, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace, true>(mesh, cells, index, position, stride, neighbors);
    index++;
    position.x++;

    for(size_t x=stride; x<_Chunk::sizeX::value-stride; x+=stride)
    {
        checkCell<_Chunk, _ChunkMesh, true, true, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace, true>(mesh, cells, index, position, stride, neighbors);
        index++;
        position.x++;
    }

    checkCell<_Chunk, _ChunkMesh, true, false, _YNegFace, _YPosFace, _ZNegFace, _ZPosFace, true>(mesh, cells, index, position, stride, neighbors);
    index++;
}


template<typename _Chunk, typename _ChunkMesh, bool _ZNegFace=true, bool _ZPosFace=true>
void checkY_Neighbor(_ChunkMesh &mesh, typename _Chunk::Cells &cells, size_t &index, glm::ivec3 &position, size_t stride, std::vector<_Chunk *> *neighbors=nullptr)
{
    position.y=0;

    checkX<_Chunk, _ChunkMesh, false, true, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride, neighbors);
    position.y++;

    for(size_t y=stride; y<_Chunk::sizeY::value-stride; y+=stride)
    {
        position.x=0;

        checkX<_Chunk, _ChunkMesh, true, true, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride, neighbors);
        position.y++;
    }

    checkX<_Chunk, _ChunkMesh, true, false, _ZNegFace, _ZPosFace>(mesh, cells, index, position, stride, neighbors);
}


//neighbors vector follows the faces indexing, 0:-x, 1:+x, 2:-y, 3:+y, 4:-z, 5:+z
template<typename _Chunk, typename _ChunkMesh>
void buildCubicMesh_Neighbor(_ChunkMesh &mesh, _Chunk *chunk, std::vector<_Chunk *> *neighbors=nullptr)
{
    size_t stride=glm::pow(2u, (unsigned int)chunk->getLod());
    glm::ivec3 size(_Chunk::sizeX::value/stride, _Chunk::sizeY::value/stride, _Chunk::sizeZ::value/stride);

    const int requiredScratchSize=(size.x+1)*(size.y+1)*(size.z+1);
    const int requiredIndices=(size.x*size.y*size.z)*6*4;
    const int requiredVertices=requiredScratchSize;

    typename _Chunk::Cells &cells=chunk->getCells();
    glm::ivec3 position(0, 0, 0);

    size_t index=0;

    checkY_Neighbor<_Chunk, _ChunkMesh, false, true>(mesh, cells, index, position, stride);
    position.z++;

    for(size_t z=stride; z<_Chunk::sizeZ::value-stride; z+=stride)
    {
        checkY_Neighbor<_Chunk, _ChunkMesh, true, true>(mesh, cells, index, position, stride);
        position.z++;
    }

    checkY_Neighbor<_Chunk, _ChunkMesh, true, false>(mesh, cells, index, position, stride);
}

} //namespace voxigen

#endif //_voxigen_cubicMeshBuilder_h_