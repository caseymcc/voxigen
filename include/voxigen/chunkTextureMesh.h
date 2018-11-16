#ifndef _voxigen_chunkTextureMesh_h_
#define _voxigen_chunkTextureMesh_h_

#include "voxigen/defines.h"
#include "voxigen/chunk.h"
#include "voxigen/textureAtlas.h"
#include "voxigen/meshBuffer.h"

#include <array>

namespace voxigen
{

class ChunkTextureMesh
{
public:
    struct Vertex
    {
        uint8_t x, y, z;
        uint16_t tx, ty;
        uint32_t data;
    };

    ChunkTextureMesh(){};
    ChunkTextureMesh(TextureAtlas const *textureAtlas):m_textureAtlas(textureAtlas) {};

    void setTextureAtlas(TextureAtlas const *textureAtlas)
    {
        m_textureAtlas=textureAtlas; 

        short resolution=(short)m_textureAtlas->resolution();

        textOffsetX[0]=0;
        textOffsetX[1]=resolution;
        textOffsetX[2]=resolution;
        textOffsetX[3]=0;

        textOffsetY[0]=0;
        textOffsetY[1]=0;
        textOffsetY[2]=resolution;
        textOffsetY[3]=resolution;
    }
    void addFace(size_t face, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 4> &quad);

    std::vector<Vertex> &getVerticies() { return m_verticies; }
    std::vector<int> &getIndices() { return m_indices; }

    size_t memoryUsed();

    void reserve(size_t vertexCount, size_t indexCount);
    void clear();

private:
    TextureAtlas const *m_textureAtlas;
    std::array<short, 4> textOffsetX;
    std::array<short, 4> textOffsetY;

    std::vector<Vertex> m_verticies;
    std::vector<int> m_indices;
};

inline void ChunkTextureMesh::addFace(size_t face, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 4> &quad)
{
    size_t index=m_verticies.size();
    size_t vertIndex=index;
    m_verticies.resize(index+4);

    const TextureAtlas::BlockEntry &block=m_textureAtlas->getBlockEntry(cellType);
    const TextureAtlas::TextureEntry &entry=block.faces[face];

    short texX=entry.x;
    short texY=entry.y;
    short resolution=(short)m_textureAtlas->resolution();

    if(entry.method==LayerMethod::repeat)
    {
        glm::ivec2 texPos;

        if(face<=Face::back)
        {
            if(face<=Face::right)
            {
                texPos.x=position.y%entry.tileX;
                if(face==Face::right)
                    texPos.x=entry.tileX-texPos.x;
            }
            else
            {
                texPos.x=position.x%entry.tileX;
                if(face==Face::back)
                    texPos.x=entry.tileX-texPos.x;
            }
            texPos.y=position.z%entry.tileY;
        }
        else
        {
            texPos.x=position.x%entry.tileX;
            texPos.y=position.y%entry.tileY;
            if(face==Face::top)
            {
                texPos.x=entry.tileX-texPos.x;
                texPos.y=entry.tileY-texPos.y;
            }
        }

        texX=texX+(texPos.x%entry.tileX)*resolution;
        texY=texY+(texPos.y%entry.tileY)*resolution;
    }

//    std::vector<short> textOffsetX={0, resolution, resolution, 0};
//    std::vector<short> textOffsetY={0, 0, resolution, resolution};

    for(size_t i=0; i<4; ++i)
    {
        m_verticies[index].x=quad[i].x;
        m_verticies[index].y=quad[i].y;
        m_verticies[index].z=quad[i].z;
        m_verticies[index].tx=texX+textOffsetX[i];
        m_verticies[index].ty=texY+textOffsetY[i];
        m_verticies[index].data=cellType;
        index++;
    }

    size_t indicesIndex=m_indices.size();
    m_indices.resize(indicesIndex+6);

    m_indices[indicesIndex++]=vertIndex;
    m_indices[indicesIndex++]=vertIndex+1;
    m_indices[indicesIndex++]=vertIndex+2;
    m_indices[indicesIndex++]=vertIndex;
    m_indices[indicesIndex++]=vertIndex+2;
    m_indices[indicesIndex]=vertIndex+3;
}

inline size_t ChunkTextureMesh::memoryUsed()
{
    size_t memoryUsed=0;

    memoryUsed+=m_verticies.size()*sizeof(Vertex);
    memoryUsed+=m_indices.size()*sizeof(int);

    return memoryUsed;
}

inline void ChunkTextureMesh::reserve(size_t vertexCount, size_t indexCount)
{
    m_verticies.reserve(vertexCount);
    m_indices.reserve(indexCount);
}

inline void ChunkTextureMesh::clear()
{
    m_verticies.clear();
    m_indices.clear();
}

} //namespace voxigen

#endif //_voxigen_chunkTextureMesh_h_