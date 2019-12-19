#ifndef _voxigen_texturedMesh_h_
#define _voxigen_texturedMesh_h_

#include "voxigen/defines.h"
#include "voxigen/texturing/textureAtlas.h"

#include <array>

namespace voxigen
{

class TexturedMesh
{
public:
    struct Vertex
    {
        uint16_t x, y, z;
        uint16_t tx, ty;
        uint16_t data;
    };

    TexturedMesh(){};
    TexturedMesh(TextureAtlas const *textureAtlas):m_textureAtlas(textureAtlas) {};

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

    inline void addQuad(size_t textureFace, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 4> &quad);

    std::vector<Vertex> &getVertexes() { return m_vertexes; }
    std::vector<int> &getIndexes() { return m_indexes; }

    size_t memoryUsed();

    void reserve(size_t vertexCount, size_t indexCount);
    void clear();

private:
    TextureAtlas const *m_textureAtlas;
    std::array<short, 4> textOffsetX;
    std::array<short, 4> textOffsetY;

    std::vector<Vertex> m_vertexes;
    std::vector<int> m_indexes;
};

inline void TexturedMesh::addQuad(size_t textureFace, unsigned int cellType, const glm::ivec3 &position, const std::array<glm::ivec3, 4> &quad)
{
    size_t index=m_vertexes.size();
    size_t vertIndex=index;
    m_vertexes.resize(index+4);

    const TextureAtlas::BlockEntry &block=m_textureAtlas->getBlockEntry(cellType);
    const TextureAtlas::TextureEntry &entry=block.faces[textureFace];

    short texX=entry.x;
    short texY=entry.y;
    short resolution=(short)m_textureAtlas->resolution();

    if(entry.method==LayerMethod::repeat)
    {
        glm::ivec2 texPos;

        if(textureFace<=Face::back)
        {
            if(textureFace<=Face::right)
            {
                texPos.x=position.y%entry.tileX;
                if(textureFace==Face::right)
                    texPos.x=entry.tileX-texPos.x;
            }
            else
            {
                texPos.x=position.x%entry.tileX;
                if(textureFace==Face::back)
                    texPos.x=entry.tileX-texPos.x;
            }
            texPos.y=position.z%entry.tileY;
        }
        else
        {
            texPos.x=position.x%entry.tileX;
            texPos.y=position.y%entry.tileY;
            if(textureFace==Face::top)
            {
                texPos.x=entry.tileX-texPos.x;
                texPos.y=entry.tileY-texPos.y;
            }
        }

        texX=texX+(texPos.x%entry.tileX)*resolution;
        texY=texY+(texPos.y%entry.tileY)*resolution;
    }

    for(size_t i=0; i<4; ++i)
    {
        m_vertexes[index].x=quad[i].x;
        m_vertexes[index].y=quad[i].y;
        m_vertexes[index].z=quad[i].z;
        m_vertexes[index].tx=texX+textOffsetX[i];
        m_vertexes[index].ty=texY+textOffsetY[i];
        m_vertexes[index].data=cellType;
        index++;
    }

    size_t indicesIndex=m_indexes.size();
    m_indexes.resize(indicesIndex+6);

    m_indexes[indicesIndex++]=vertIndex;
    m_indexes[indicesIndex++]=vertIndex+1;
    m_indexes[indicesIndex++]=vertIndex+2;
    m_indexes[indicesIndex++]=vertIndex;
    m_indexes[indicesIndex++]=vertIndex+2;
    m_indexes[indicesIndex]=vertIndex+3;
}

inline size_t TexturedMesh::memoryUsed()
{
    size_t memoryUsed=0;

    memoryUsed+=m_vertexes.size()*sizeof(Vertex);
    memoryUsed+=m_indexes.size()*sizeof(int);

    return memoryUsed;
}

inline void TexturedMesh::reserve(size_t vertexCount, size_t indexCount)
{
    m_vertexes.reserve(vertexCount);
    m_indexes.reserve(indexCount);
}

inline void TexturedMesh::clear()
{
    m_vertexes.clear();
    m_indexes.clear();
}

} //namespace voxigen

#endif //_voxigen_texturedMesh_h_