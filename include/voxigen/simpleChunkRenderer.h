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

    SimpleChunkRenderer():m_state(Init),m_chunkOffset(0.0f,0.0f,0.0f){}
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
    void setSegmentHash(SegmentHash hash);
    void setChunk(SharedChunkHandle chunk);
    void setChunkOffset(glm::vec3 chunkOffset) { m_chunkOffset=chunkOffset; m_state=Dirty; }
    const glm::vec3 &getChunkOffset() { return m_chunkOffset; }

    void build(unsigned int instanceData);
#ifndef NDEBUG
    void buildOutline(unsigned int instanceData);
#endif //NDEBUG

    void update();
    void invalidate();

    void draw();
#ifndef NDEBUG
    void drawOutline();
#endif //NDEBUG

    const SegmentHash getSegmentHash() { return m_segmentHash; }
    const ChunkHash getChunkHash() { return m_chunkHandle->hash; }
    const glm::ivec3 &getPosition() { return m_chunkHandle->chunk->getPosition(); }
    
private:
    RenderType *m_parent;

    State m_state;
    SegmentHash m_segmentHash;
    SharedChunkHandle m_chunkHandle;
    bool m_empty;

    glm::vec3 m_chunkOffset;
    unsigned int m_validBlocks;
    unsigned int m_vertexArray;
    unsigned int m_offsetVBO;
    std::vector<glm::vec4> m_ChunkInfoOffset;

#ifndef NDEBUG
    unsigned int m_outlineVertexArray;
    unsigned int m_outlineOffsetVBO;
#endif //NDEBUG
};

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::setParent(RenderType *parent)
{
    m_parent=parent;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::setSegmentHash(SegmentHash hash)
{
    m_segmentHash=hash;
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
    
    glBindVertexArray(0);
    
    if(m_chunkHandle)
        m_state=Dirty;
    else
        m_state=Invalid;

}

#ifndef NDEBUG
template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::buildOutline(unsigned int instanceData)
{
    glGenVertexArrays(1, &m_outlineVertexArray);
    glGenBuffers(1, &m_outlineOffsetVBO);

    glBindVertexArray(m_outlineVertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceData);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineOffsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}
#endif //NDEBUG

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::update()
{
    if(m_state!=Dirty)
        return;

    if(m_chunkHandle->status!=ChunkHandleType::Memory) //not loaded yet need to wait
    {
#ifndef NDEBUG
        //chunk is not going to be valid till loaded, so going to hack together the offset from
        //the hash info
        glm::vec4 position=glm::vec4(m_parent->getGrid()->getDescriptors().chunkOffset(m_chunkHandle->hash)+m_chunkOffset, 1.0f);

        glBindBuffer(GL_ARRAY_BUFFER, m_outlineOffsetVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, glm::value_ptr(position), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif //NDEBUG
        return;
    }

    if(m_chunkHandle->empty)
    {
        m_state=Empty;
        return;
    }

    ChunkType *chunk=m_chunkHandle->chunk.get();

    auto &cells=chunk->getCells();
//    glm::ivec3 &chunkSize=m_parent->getWorld()->getDescriptors().chunkSize;
//    std::vector<glm::vec4> translations(chunkSize.x*chunkSize.y*chunkSize.z);
    std::vector<glm::vec4> translations(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value);
//    glm::ivec3 position=m_chunk->getPosition();
    glm::vec3 position=chunk->getGridOffset()+m_chunkOffset;
    glm::ivec3 pos=position;

    int index=0;
    int validCells=0;
    
    for(int z=0; z<ChunkType::sizeZ::value; ++z)
    {
        pos.y=position.y;
        for(int y=0; y<ChunkType::sizeY::value; ++y)
        {
            pos.x=position.x;
            for(int x=0; x<ChunkType::sizeX::value; ++x)
            {
                unsigned int type=cells[index].type;

                if(type>0)
                {
                    translations[validCells]=glm::vec4(pos, type);
                    validCells++;
                }
                pos.x+=1.0;
                index++;
            }
            pos.y+=1.0;
        }
        pos.z+=1.0;
    }

    m_validBlocks=validCells;
    if(validCells==0)
    {
        m_state=Empty;
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*validCells, translations.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_state=Built;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::invalidate()
{
    m_state=Invalid;
    m_chunkHandle.reset(nullptr);
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::draw()
{
    if(m_state==Built)
    {
        glBindVertexArray(m_vertexArray);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, m_validBlocks);
    }
}

#ifndef NDEBUG
template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::drawOutline()
{
    if(m_state==Invalid)
        return;
    if(m_state==Built)
        return;
    if(m_state==Empty)
        return;

    glBindVertexArray(m_outlineVertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1);
}
#endif //NDEBUG

}//namespace voxigen

#endif //_voxigen_simpleChunkRenderer_h_