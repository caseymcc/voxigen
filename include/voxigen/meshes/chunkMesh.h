#ifndef _voxigen_chunkMesh_h_
#define _voxigen_chunkMesh_h_

//#include "voxigen/defines.h"
//#include "voxigen/chunk.h"

//#include <array>
#include <glm/glm.hpp>
#include <array>
#include <vector>

namespace voxigen
{

struct ChunkMeshVertex
{
    uint8_t x, y, z;
    uint8_t tx, ty;
    uint32_t data;
};

class ChunkMesh
{
public:
    ChunkMesh() {};

//    void addFace(const std::vector<glm::ivec3>& blockFace, glm::ivec3 position);
//    void addFace(const std::vector<glm::ivec3>& blockFace, glm::ivec3 position, unsigned int data);
    void addFace(size_t face, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 4> &quad);

    std::vector<ChunkMeshVertex> &getVertexes() { return m_verticies; }
    std::vector<int> &getIndexes() { return m_indices; }

    size_t memoryUsed();

private:
    std::vector<ChunkMeshVertex> m_verticies;
    std::vector<int> m_indices;
    
};

inline void ChunkMesh::addFace(size_t face, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 4> &quad)
{
    size_t index=m_verticies.size();
    int vertIndex=(int)index;
    m_verticies.resize(index+4);

    for(size_t i=0; i<4; ++i)
    {
//        m_verticies[index].x=quad[i].x;
//        m_verticies[index].y=quad[i].y;
//        m_verticies[index].z=quad[i].z;
        m_verticies[index].tx=0;
        m_verticies[index].ty=0;
        m_verticies[index].data=cellType;
        index++;
    }

    size_t indicesIndex=m_indices.size();
    m_indices.resize(indicesIndex+6);

    m_indices[indicesIndex++]=vertIndex;
    m_indices[indicesIndex++]=vertIndex+1;
    m_indices[indicesIndex++]=vertIndex+2;
    m_indices[indicesIndex++]=vertIndex+2;
    m_indices[indicesIndex++]=vertIndex+3;
    m_indices[indicesIndex]=vertIndex;
}

//inline void ChunkMesh::addFace(const std::vector<glm::ivec3> &face, glm::ivec3 position)
//{
//    size_t index=m_verticies.size();
//    size_t vertIndex=index;
//    m_verticies.resize(index+4);
//
//    for(size_t i=0; i<4; ++i)
//    {
//        m_verticies[index].x=face[i].x+position.x;
//        m_verticies[index].y=face[i].y+position.y;
//        m_verticies[index].z=face[i].z+position.z;
//        m_verticies[index].data=0;
//        index++;
//    }
//
//    size_t indicesIndex=m_indices.size();
//    m_indices.resize(indicesIndex+6);
//
//    m_indices[indicesIndex++]=vertIndex;
//    m_indices[indicesIndex++]=vertIndex+1;
//    m_indices[indicesIndex++]=vertIndex+2;
//    m_indices[indicesIndex++]=vertIndex+2;
//    m_indices[indicesIndex++]=vertIndex+3;
//    m_indices[indicesIndex]=vertIndex;
//}
//
//inline void ChunkMesh::addFace(const std::vector<glm::ivec3> &face, glm::ivec3 position, unsigned int data)
//{
//    size_t index=m_verticies.size();
//    size_t vertIndex=index;
//    m_verticies.resize(index+4);
//
//    for(size_t i=0; i<4; ++i)
//    {
//        m_verticies[index].x=face[i].x+position.x;
//        m_verticies[index].y=face[i].y+position.y;
//        m_verticies[index].z=face[i].z+position.z;
//        m_verticies[index].data=data;
//        index++;
//    }
//
//    size_t indicesIndex=m_indices.size();
//    m_indices.resize(indicesIndex+6);
//
//    m_indices[indicesIndex++]=vertIndex;
//    m_indices[indicesIndex++]=vertIndex+1;
//    m_indices[indicesIndex++]=vertIndex+2;
//    m_indices[indicesIndex++]=vertIndex+2;
//    m_indices[indicesIndex++]=vertIndex+3;
//    m_indices[indicesIndex]=vertIndex;
//}

inline size_t ChunkMesh::memoryUsed()
{
    size_t memoryUsed=0;

    memoryUsed+=m_verticies.size()*sizeof(ChunkMeshVertex);
    memoryUsed+=m_indices.size()*sizeof(int);

    return memoryUsed;
}

} //namespace voxigen

#endif //_voxigen_chunkMesh_h_