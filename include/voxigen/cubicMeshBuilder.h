#ifndef _voxigen_cubicMeshBuilder_h_
#define _voxigen_cubicMeshBuilder_h_

#include "voxigen/defines.h"
#include "voxigen/chunk.h"
#include "voxigen/chunkMesh.h"
#include "voxigen/faces.h"

#include <array>

namespace voxigen
{
//#define SIMPLE_MESH

#ifdef SIMPLE_MESH
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
            mesh.addFace(faces[Face::xNeg], position, cellType);
    }
    else
        mesh.addFace(faces[Face::xNeg], position, cellType);

    if(_XPosFace)
    {
        size_t adjIndex=index+1;

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::xPos], position, cellType);
    }
    else
        mesh.addFace(faces[Face::xPos], position, cellType);

    if(_YNegFace)
    {
        size_t adjIndex=index-_Chunk::sizeX::value;

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::yNeg], position, cellType);
    }
    else
        mesh.addFace(faces[Face::yNeg], position, cellType);

    if(_YPosFace)
    {
        size_t adjIndex=index+_Chunk::sizeX::value;

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::yPos], position, cellType);
    }
    else
        mesh.addFace(faces[Face::yPos], position, cellType);

    if(_ZNegFace)
    {
        size_t adjIndex=index-(_Chunk::sizeX::value*_Chunk::sizeY::value);

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::zNeg], position, cellType);
    }
    else
        mesh.addFace(faces[Face::zNeg], position, cellType);


    if(_ZPosFace)
    {
        size_t adjIndex=index+(_Chunk::sizeX::value*_Chunk::sizeY::value);

        if(empty(cells[adjIndex]))
            mesh.addFace(faces[Face::zPos], position, cellType);
    }
    else
        mesh.addFace(faces[Face::zPos], position, cellType);

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
    glm::ivec3 pos;
    unsigned int vertIndex[4];

    auto faceQuad=faces[face];
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
    _Chunk::CellType &cell=cells[index];

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
    size_t stride=glm::pow(2, chunk->getLod());
    glm::ivec3 size(_Chunk::sizeX::value/stride, _Chunk::sizeY::value/stride, _Chunk::sizeZ::value/stride);

    const int requiredScratchSize=(size.x+1)*(size.y+1)*(size.z+1);
    const int requiredIndices=(size.x*size.y*size.z)*6*4;
    const int requiredVertices=requiredScratchSize;

//    mesh.indicesSize=0;
//    mesh.verticesSize=0;
//
//    //currently the algorithm is greedy and is expecting you to provide memory for worst case where every cube is draw
//    //this memory is expected to be used over and over, the user is expected to copy the results from this memory
//    if((mesh.indicesCapacity<requiredIndices)||
//        (mesh.verticesCapacity<requiredVertices)||
//        (mesh.scratchCapacity<requiredScratchSize))
//    {//dont have enough space to work with
//        return;
//    }
//
//    for(size_t i=0; i<requiredScratchSize; ++i)
//        mesh.scratch[i]=requiredScratchSize;
////    memset((void *)mesh.scratch, 0, requiredScratchSize*sizeof(unsigned int));

    _Chunk::Cells &cells=chunk->getCells();
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

//    unsigned int *scratch=mesh.scratch;
//    size_t scratchIndex=0;
//    size_t verticesIndex=0;
//    Vertex vertex;
//
//    for(vertex.z=0; vertex.z<=_Chunk::sizeZ::value; ++vertex.z)
//    {
//        for(vertex.y=0; vertex.y<=_Chunk::sizeY::value; ++vertex.y)
//        {
//            for(vertex.x=0; vertex.x<=_Chunk::sizeX::value; ++vertex.x)
//            {
//                if(scratch[scratchIndex] > 0)
//                {
//                    mesh.vertices[verticesIndex]=vertex;
//                    scratch[scratchIndex]=verticesIndex;
//
//                    verticesIndex++;
//                }
//                scratchIndex++;
//            }
//        }
//    }
//    mesh.verticesSize=verticesIndex;

//    for(size_t i=0; i<mesh.indicesSize; ++i)
//    {
//        size_t index=mesh.indices[i];
//
//        if(scratch[index]==requiredScratchSize)
//        {
//            scratch[index]=verticesIndex;
//            mesh.vertices[verticesIndex]=indexToVertex<_Chunk>(index, stride);
//            verticesIndex++;
//        }
//
//        mesh.indices[i]=scratch[index];
//    }
//    mesh.verticesSize=verticesIndex;
}

//Mesh g_mesh;//global memory for handling meshing
//
//template<typename _Chunk>
//void buildCubicMesh(ChunkMesh &mesh, _Chunk *chunk)
//{
//    if(g_mesh.indicesCapacity<indicesSize<_Chunk>())
//    {
//        if(g_mesh.indices!=nullptr)
//            delete g_mesh.indices;
//        g_mesh.indicesCapacity=indicesSize<_Chunk>();
//        g_mesh.indices=(unsigned int *)malloc(sizeof(unsigned int)*g_mesh.indicesCapacity);
//        g_mesh.indicesSize=0;
//    }
//
//    if(g_mesh.verticesCapacity<verticesSize<_Chunk>())
//    {
//        if(g_mesh.vertices!=nullptr)
//            delete g_mesh.vertices;
//        if(g_mesh.scratch!=nullptr)
//            delete g_mesh.scratch;
//        g_mesh.verticesCapacity=verticesSize<_Chunk>();
//        g_mesh.scratchCapacity=g_mesh.verticesCapacity;
//        g_mesh.vertices=(Vertex *)malloc(sizeof(Vertex)*g_mesh.verticesCapacity);
//        g_mesh.scratch=(unsigned int *)malloc(sizeof(unsigned int)*g_mesh.scratchCapacity);
//        g_mesh.verticesSize=0;
//    }
//
//    buildCubicMesh(g_mesh, chunk);
//    
//    if((g_mesh.indicesSize<=0)||(g_mesh.verticesSize<=0))
//        return;
//
//    std::vector<int> &indices=mesh.getIndices();
//    std::vector<ChunkMeshVertex> &vertices=mesh.getVerticies();
//
//    indices.resize(g_mesh.indicesSize);
//    vertices.resize(g_mesh.verticesSize);
//
//    for(size_t i=0; i<g_mesh.indicesSize; ++i)
//        indices[i]=g_mesh.indices[i];
//
//    for(size_t i=0; i<g_mesh.verticesSize; ++i)
//    {
//        vertices[i].x=g_mesh.vertices[i].x;
//        vertices[i].y=g_mesh.vertices[i].y;
//        vertices[i].z=g_mesh.vertices[i].z;
//    }
//}

#endif//SIMPLE_MESH

} //namespace voxigen

#endif //_voxigen_cubicMeshBuilder_h_