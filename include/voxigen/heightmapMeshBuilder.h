#ifndef _voxigen_heightMeshBuilder_h_
#define _voxigen_heightMeshBuilder_h_

#include "voxigen/defines.h"
#include "voxigen/faces.h"

#include <array>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/integer.hpp>

namespace voxigen
{

template<typename _Cell, bool negY, bool negX>
void buildVertices(std::vector<glm::ivec3> &vertices, size_t &vertIndex, const std::vector<_Cell> &cells, const glm::ivec2 &cellsSize, size_t &index, glm::ivec3 &position, size_t stride)
{
    const _Cell &cell=cells[index];

    size_t count=0;
    int cellHeight=0;

    if(type(cell)>=0)
    {
        count++;
        cellHeight+=height(cell);
    }

    if(negX)
    {
        size_t adjIndex=index-1;

        if(type(cells[adjIndex])>0)
        {
            count++;
            cellHeight+=height(cells[adjIndex]);
        }
    }

    if(negY)
    {
        size_t adjIndex=index-(cellsSize.x/stride);

        if(type(cells[adjIndex])>0)
        {
            count++;
            cellHeight+=height(cells[adjIndex]);
        }
    }

    if(negX && negY)
    {
        size_t adjIndex=index-(cellsSize.x/stride)-1;

        if(type(cells[adjIndex])>0)
        {
            count++;
            cellHeight+=height(cells[adjIndex]);
        }
    }

    if(count>0)
        cellHeight=cellHeight/count;
    else
        cellHeight=-1;

    vertices[vertIndex]=glm::ivec3(position.x, position.y, cellHeight);
}

template<typename _Cell, bool negY>
void buildVertices_x(std::vector<glm::ivec3> &vertexes, size_t &vertIndex, const std::vector<_Cell> &cells, const glm::ivec2 &cellsSize, size_t &cellIndex, glm::ivec3 &position, size_t stride, const glm::ivec2 &heightRange)
{
    position.x=0;

    buildVertices<_Cell, negY, false>(vertexes, vertIndex, cells, cellsSize, cellIndex, position, stride);
    cellIndex++;
    vertIndex++;
    position.x+=stride;

    for(size_t x=stride; x<cellsSize.x; x+=stride)
    {
        buildVertices<_Cell, negY, true>(vertexes, vertIndex, cells, cellsSize, cellIndex, position, stride);
        cellIndex++;
        vertIndex++;
        position.x+=stride;
    }

    cellIndex--;
    //run last one again without negative
    buildVertices<_Cell, negY, false>(vertexes, vertIndex, cells, cellsSize, cellIndex, position, stride);
    cellIndex++;
    vertIndex++;
}

template<typename _Mesh, typename _Cell>
void buildHeightmapMesh(_Mesh &mesh, const std::vector<_Cell> &cells, const glm::ivec2 &cellsSize, const glm::ivec2 &heightRange, size_t lod)
{
    size_t stride=glm::pow(2u, (unsigned int)lod);
    glm::ivec2 size=cellsSize/(int)stride;

    glm::ivec3 position(0, 0, 0);
    size_t index=0;
    size_t vertIndex=0;

    std::vector<glm::ivec3> vertexes((size.x+1)*(size.y+1));
    
    buildVertices_x<_Cell, false>(vertexes, vertIndex, cells, cellsSize, index, position, stride, heightRange);
    position.y+=stride;
    for(size_t y=stride; y<cellsSize.y; y+=stride)
    {
        buildVertices_x<_Cell, true>(vertexes, vertIndex, cells, cellsSize, index, position, stride, heightRange);
        position.y+=stride;
//        index++;
//        vertIndex++;
    }
    index-=size.x;
    buildVertices_x<_Cell, false>(vertexes, vertIndex, cells, cellsSize, index, position, stride, heightRange);

    std::vector<typename _Mesh::Vertex> &meshVertexes=mesh.getVertexes();
    std::vector<int> &meshIndexes=mesh.getIndexes();
    std::array<glm::ivec3, 4> quad;

    index=0;
    position=glm::ivec3(0, 0, 0);

    for(size_t y=0; y<size.y; y++)
    {
        position.x=0;
        for(size_t x=0; x<size.x; x++)
        {
            unsigned int cellType=type(cells[index]);

            if(cellType<0)
            {
                position.x+=stride;
                index++;
                continue;
            }
//            if(vertexes[index]<0)
//                continue;
//            if(vertexes[index+1]<0)
//                continue;
//            if(vertexes[index+(size.x+1)]<0)
//                continue;
//            if(vertexes[index+(size.x+1)+1]<0)
//                continue;

            quad[0]=vertexes[index];
            quad[1]=vertexes[index+1];
            quad[2]=vertexes[index+(size.x+1)+1];
            quad[3]=vertexes[index+(size.x+1)];

            position.z=glm::floor(quad[0].z);

//            if((abs(quad[0].x-quad[1].x) > stride)||(abs(quad[0].y-quad[1].y) > stride)||
//                (abs(quad[1].x-quad[2].x) > stride)||(abs(quad[1].y-quad[2].y) > stride)||
//                (abs(quad[2].x-quad[3].x) > stride)||(abs(quad[2].y-quad[3].y) > stride)||
//                (abs(quad[0].x-quad[3].x) > stride)||(abs(quad[0].y-quad[3].y) > stride))
//            {
//                index++;
//                continue;
//            }
//
//            if((abs(quad[0].x-quad[1].x) <=0)||
//                (abs(quad[1].y-quad[2].y) <=0)||
//                (abs(quad[2].x-quad[3].x) <=0)||
//                (abs(quad[0].y-quad[3].y) <=0))
//            {
//                index++;
//                continue;
//            }
                

            mesh.addQuad(Face::top, cellType, position, quad);

            position.x+=stride;
            index++;
        }
//        index++;
        
        position.y+=stride;
    }
}

} //namespace voxigen

#endif //_voxigen_heightMeshBuilder_h_