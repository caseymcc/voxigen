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
        int8_t nx, ny, nz;
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
    void addFace(size_t face, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 5> &quad);

    std::vector<Vertex> &getVertexes() { return m_verticies; }
    std::vector<int> &getIndexes() { return m_indices; }

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

inline void ChunkTextureMesh::addFace(size_t face, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 5> &quad)
{
    size_t index=m_verticies.size();
    size_t vertIndex=index;
    m_verticies.resize(index+4);

    TextureAtlas::TextureEntry entry;

    if(cellType<m_textureAtlas->size())
    {
        const TextureAtlas::BlockEntry &block=m_textureAtlas->getBlockEntry(cellType);
        entry=block.faces[face];
    }
    else
    {
        entry.method=LayerMethod::unknown;
        entry.x=0;
        entry.y=0;
    }

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
        Vertex &vertex=m_verticies[index];

        vertex.x=quad[i].x;
        vertex.y=quad[i].y;
        vertex.z=quad[i].z;
        vertex.nx=quad[4].x;
        vertex.ny=quad[4].y;
        vertex.nz=quad[4].z;
        vertex.tx=texX+textOffsetX[i];
        vertex.ty=texY+textOffsetY[i];
        vertex.data=cellType;
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