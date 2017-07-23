#ifndef _voxigen_simpleChunkRenderer_h_
#define _voxigen_simpleChunkRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/chunkHandle.h"
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
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
//    typedef std::shared_ptr<ChunkType> SharedChunk;

    SimpleChunkRenderer():m_state(Init){}
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
    void setChunk(SharedChunkHandle chunk);
    void build(unsigned int instanceData);
    void update();
    void invalidate();

    void draw();
    void drawOutline();

    const unsigned int getHash() { return m_chunkHandle->hash; }
    const glm::ivec3 &getPosition() { return m_chunkHandle->chunk->getPosition(); }
    
private:
    RenderType *m_parent;

    State m_state;
    SharedChunkHandle m_chunkHandle;
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
void SimpleChunkRenderer<_Parent, _Chunk>::setChunk(SharedChunkHandle chunk)
{
    m_chunkHandle=chunk;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*_Chunk::sizeX::value*_Chunk::sizeY::value*_Chunk::sizeZ::value, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);
    
    if(m_chunkHandle)
        m_state=Dirty;
    else
        m_state=Invalid;

}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::update()
{
    if(m_state!=Dirty)
        return;

    if(m_chunkHandle->status!=ChunkHandleType::Memory) //not loaded yet need to wait
        return;

    if(m_chunkHandle->empty)
    {
        m_state=Empty;
        return;
    }

    ChunkType *chunk=m_chunkHandle->chunk.get();

    auto &blocks=chunk->getBlocks();
//    glm::ivec3 &chunkSize=m_parent->getWorld()->getDescriptors().chunkSize;
//    std::vector<glm::vec4> translations(chunkSize.x*chunkSize.y*chunkSize.z);
    std::vector<glm::vec4> translations(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value);
//    glm::ivec3 position=m_chunk->getPosition();
    glm::vec3 position=chunk->getWorldOffset();
    glm::ivec3 pos=position;

    int index=0;
    int validBlocks=0;
    
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
                    translations[validBlocks]=glm::vec4(pos, type);
                    validBlocks++;
                }
                pos.x+=1.0;
                index++;
            }
            pos.y+=1.0;
        }
        pos.z+=1.0;
    }

    m_validBlocks=validBlocks;
    if(validBlocks==0)
    {
        m_state=Empty;
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*validBlocks, translations.data(), GL_STATIC_DRAW);
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