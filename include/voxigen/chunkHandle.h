#ifndef _voxigen_chunkHandle_h_
#define _voxigen_chunkHandle_h_

#include "voxigen/chunk.h"

#include <memory>

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
        Loading,
        Generating,
        Memory
    };

    ChunkHandle(unsigned int segmentHash, unsigned int chunkHash):segmentHash(segmentHash), hash(chunkHash), status(Unknown), cachedOnDisk(false), empty(false){}

    void release() { chunk.reset(nullptr); status=Unknown; }

    Status status;
    unsigned int segmentHash;
    unsigned int hash;
    UniqueChunk chunk;

    bool cachedOnDisk;
    bool empty;
};

} //namespace voxigen

#endif //_voxigen_chunkHandler_h_