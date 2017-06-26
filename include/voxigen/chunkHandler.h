#ifndef _voxigen_chunkHandler_h_
#define _voxigen_chunkHandler_h_

#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"

#include <thread>
#include <memory>
#include <unordered_map>

namespace voxigen
{

template<typename _Chunk>
struct ChunkHandle
{
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    enum Status
    {
        Unknown,
        Cached,
        Loading
        Memory,
    };

    UniqueChunk chunk;
};

template<typename _Chunk>
class ChunkHandler
{
public:
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    typedef std::unordered_map<unsigned int, UniqueChunk> ChunkHandleMap;

    ChunkHandler(WorldDescriptors *descriptors);

    void init();

    void ioThread();
    void generatorThread();
    

    UniqueChunk getChunk(unsigned int hash);
private:
    WorldDescriptors *m_descriptors;

    std::thread m_ioThread;
    std::thread m_generatorThread;

    ChunkHandleMap m_chunks;
};

template<typename _Chunk>
ChunkHandler<_Chunk>::ChunkHandler(WorldDescriptors *descriptors):
m_descriptors(descriptors)
{

}

template<typename _Chunk>
void ChunkHandler<_Chunk>::init()
{
    m_ioThread=std::thread(std::bind(&ChunkHandler<_Chunk>::ioThread, this));
    m_generatorThread=std::thread(std::bind(&ChunkHandler<_Chunk>::generatorThread, this));
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::ioThread()
{
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::generatorThread()
{
}

UniqueChunk getChunk(unsigned int hash)
{

}

} //namespace voxigen

#endif //_voxigen_chunkHandler_h_