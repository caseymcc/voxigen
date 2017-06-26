#ifndef _voxigen_simpleChunkRenderer_h_
#define _voxigen_simpleChunkRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/chunkInfo.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace voxigen
{

struct VOXIGEN_EXPORT SimpleCube
{
    static std::vector<float> vertCoords;
};

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleChunkRenderer
/////////////////////////////////////////////////////////////////////////////////////////
//template<typename _Block>
template<typename _Parent, typename _Chunk>
class SimpleChunkRenderer
{
public:
    typedef _Parent RenderType;
    typedef _Chunk ChunkType;

    SimpleChunkRenderer():m_state(Init), m_chunk(nullptr){}
    ~SimpleChunkRenderer() {}
    
    enum State
    {
        Init,
        Invalid,
        Dirty,
        Built,
        Empty
    };

    State getState() { return m_state; }

    void setParent(RenderType *parent);
    void setChunk(ChunkType *chunk);
    void build(unsigned int instanceData);
    void update();
    void invalidate();

    void draw();
    void drawOutline();

    const unsigned int getHash() { return m_chunk->getHash(); }
    const glm::ivec3 &getPosition() { return m_chunk->getPosition(); }
    
private:
    RenderType *m_parent;

    State m_state;
    ChunkType *m_chunk;
    bool m_empty;

    unsigned int m_validBlocks;
    unsigned int m_vertexArray;
    unsigned int m_offsetVBO;
    std::vector<glm::vec4> m_ChunkInfoOffset;
};

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::setParent(RenderType *parent)
{
    m_parent=parent;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::setChunk(ChunkType *chunk)
{
    m_chunk=chunk;

    if(m_state!=Init)
        m_state=Dirty;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::build(unsigned int instanceData)
{
    if(m_state !=Init)
        return;

    glGenVertexArrays(1, &m_vertexArray);

    glGenBuffers(1, &m_offsetVBO);
    update();

    glBindVertexArray(m_vertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceData);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*4096, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);
    
    if(m_chunk != nullptr)
        m_state=Dirty;
    else
        m_state=Invalid;

}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::update()
{
    if(m_state!=Dirty)
        return;

    auto &blocks=m_chunk->getBlocks();
//    glm::ivec3 &chunkSize=m_parent->getWorld()->getDescriptors().chunkSize;
//    std::vector<glm::vec4> translations(chunkSize.x*chunkSize.y*chunkSize.z);
    std::vector<glm::vec4> translations(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value);
//    glm::ivec3 position=m_chunk->getPosition();
    glm::vec3 position=m_chunk->getWorldOffset();
    glm::ivec3 pos=position;

    int index=0;
    
    for(int z=0; z<ChunkType::sizeZ::value; ++z)
    {
        pos.y=position.y;
        for(int y=0; y<ChunkType::sizeY::value; ++y)
        {
            pos.x=position.x;
            for(int x=0; x<ChunkType::sizeX::value; ++x)
            {
                unsigned int type=blocks[index].type;

                if(type>0)
                {
                    translations[index]=glm::vec4(pos, type);
                    index++;
                }
                pos.x+=1.0;
            }
            pos.y+=1.0;
        }
        pos.z+=1.0;
    }

    m_validBlocks=index;
    if(index==0)
    {
        m_state=Empty;
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*index, translations.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_state=Built;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::invalidate()
{
    m_state=Invalid;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::draw()
{
    if(m_state!=Built)
        return;

    glBindVertexArray(m_vertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, m_validBlocks);

}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::drawOutline()
{
//    glm::ivec3 &chunkSize=m_parent->getWorld()->getDescriptors().chunkSize;
//    glm::vec3 position=m_chunk->getWorldOffset();
//
//    glLineWidth(2.5);
//
//    glBegin(GL_LINES);
//        glVertex3f(position.x, position.y, position.z);
//        glVertex3f(position.x+chunkSize.x, position.y, position.z);
//
//        glVertex3f(position.x+chunkSize.x, position.y, position.z);
//        glVertex3f(position.x+chunkSize.x, position.y, position.z+chunkSize.z);
//            
//        glVertex3f(position.x+chunkSize.x, position.y, position.z+chunkSize.z);
//        glVertex3f(position.x, position.y, position.z+chunkSize.z);
//
//        glVertex3f(position.x, position.y, position.z+chunkSize.z);
//        glVertex3f(position.x, position.y, position.z);
//
//        glVertex3f(position.x+chunkSize.x, position.y, position.z);
//        glVertex3f(position.x+chunkSize.x, position.y+chunkSize.y, position.z);
//            
//        glVertex3f(position.x+chunkSize.x, position.y+chunkSize.y, position.z);
//        glVertex3f(position.x+chunkSize.x, position.y+chunkSize.y, position.z+chunkSize.z);
//            
//        glVertex3f(position.x+chunkSize.x, position.y+chunkSize.y, position.z+chunkSize.z);
//        glVertex3f(position.x+chunkSize.x, position.y, position.z+chunkSize.z);
//
//        glVertex3f(position.x+chunkSize.x, position.y+chunkSize.y, position.z);
//        glVertex3f(position.x, position.y+chunkSize.y, position.z);
//
//        glVertex3f(position.x, position.y+chunkSize.y, position.z);
//        glVertex3f(position.x, position.y+chunkSize.y, position.z+chunkSize.z);
//
//        glVertex3f(position.x, position.y+chunkSize.y, position.z+chunkSize.z);
//        glVertex3f(position.x+chunkSize.x, position.y+chunkSize.y, position.z+chunkSize.z);
//
//        glVertex3f(position.x, position.y+chunkSize.y, position.z);
//        glVertex3f(position.x, position.y, position.z);
//  
//        glVertex3f(position.x, position.y, position.z+chunkSize.z);
//        glVertex3f(position.x, position.y+chunkSize.y, position.z+chunkSize.z);
//    glEnd();
}

}//namespace voxigen

#endif //_voxigen_simpleChunkRenderer_h_