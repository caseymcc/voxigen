#ifndef _voxigen_simpleChunkRenderer_h_
#define _voxigen_simpleChunkRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace voxigen
{

template<typename _Block>
class SimpleRenderer;

struct VOXIGEN_EXPORT SimpleCube
{
    static std::vector<float> vertCoords;
};

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleChunkRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Block>
class SimpleChunkRenderer
{
public:
    SimpleChunkRenderer():m_state(Invalid){}
    ~SimpleChunkRenderer() {}
    
    enum State
    {
        Invalid,
        Init,
        Dirty,
        Built,
        Empty
    };

    State getState() { return m_state; }

    void setParent(SimpleRenderer<_Block> *parent);
    void setChunk(Chunk<_Block> *chunk);
    void build(unsigned int instanceData);
    void update();
    void invalidate();

    void draw();

    const unsigned int getHash() { return m_chunk->getHash(); }
    const glm::ivec3 &getPosition() { return m_chunk->getPosition(); }
    
private:
    SimpleRenderer<_Block> *m_parent;

    State m_state;
    Chunk<_Block> *m_chunk;
    bool m_empty;

    unsigned int m_validBlocks;
    unsigned int m_vertexArray;
    unsigned int m_offsetVBO;
    std::vector<glm::vec4> m_blockOffset;
};

template<typename _Block>
void SimpleChunkRenderer<_Block>::setParent(SimpleRenderer<_Block> *parent)
{
    m_parent=parent;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::setChunk(Chunk<_Block> *chunk)
{
    m_chunk=chunk;
    m_state=Dirty;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::build(unsigned int instanceData)
{
    if(m_state != Init)
        return;

    glGenVertexArrays(1, &m_vertexArray);

    glGenBuffers(1, &m_offsetVBO);
    update();

    glBindVertexArray(m_vertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceData);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(1, 1);
    m_state=Dirty;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::update()
{
    if(m_state!=Dirty)
        return;

    auto &blocks=m_chunk->getBlocks();
    glm::ivec3 &chunkSize=m_parent->getWorld()->getDescriptors().chunkSize;
    std::vector<glm::vec4> translations(chunkSize.x*chunkSize.y*chunkSize.z);
//    glm::ivec3 position=m_chunk->getPosition();
    glm::vec3 position=m_chunk->getWorldOffset();
    glm::ivec3 pos=position;

    int index=0;
    
    for(int z=0; z<chunkSize.z; ++z)
    {
        pos.y=position.y;
        for(int y=0; y<chunkSize.y; ++y)
        {
            pos.x=position.x;
            for(int x=0; x<chunkSize.x; ++x)
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
    m_state=Built;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::invalidate()
{
    m_state=Invalid;
}

template<typename _Block>
void SimpleChunkRenderer<_Block>::draw()
{
    if(m_state!=Built)
        return;

    glBindVertexArray(m_vertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 12, m_validBlocks);


}

}//namespace voxigen

#endif //_voxigen_simpleChunkRenderer_h_