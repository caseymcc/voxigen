#include "voxigen/chunkMesh.h"


namespace voxigen
{

ChunkMesh::ChunkMesh()
{}

void ChunkMesh::addFace(const std::array<uint8_t, 12> &face, glm::ivec3 position)
{
    size_t index=m_verticies.size();
    size_t vertIndex=index;
    m_verticies.resize(index+4);

    for(size_t i=0, faceIndex=0; i<4; ++i)
    {
        m_verticies[index].x=face[faceIndex++]+position.x;
        m_verticies[index].y=face[faceIndex++]+position.y;
        m_verticies[index].z=face[faceIndex++]+position.z;
        m_verticies[index].data=0;
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

void ChunkMesh::addFace(const std::array<uint8_t, 12> &face, glm::ivec3 position, unsigned int data)
{
    size_t index=m_verticies.size();
    size_t vertIndex=index;
    m_verticies.resize(index+4);

    for(size_t i=0, faceIndex=0; i<4; ++i)
    {
        m_verticies[index].x=face[faceIndex++]+position.x;
        m_verticies[index].y=face[faceIndex++]+position.y;
        m_verticies[index].z=face[faceIndex++]+position.z;
        m_verticies[index].data=data;
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

} //namespace voxigen

