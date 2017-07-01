#ifndef _voxigen_chunkHandler_h_
#define _voxigen_chunkHandler_h_

#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"
#include "voxigen/worldGenerator.h"

#include <thread>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <queue>

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
        Memory,
    };

    Status status;
    UniqueChunk chunk;
};

template<typename _Chunk>
class ChunkHandler
{
public:
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkType> SharedChunkHandle;
    typedef std::weak_ptr<ChunkType> WeakChunkHandle;
    typedef std::unordered_map<unsigned int, WeakChunkHandle> WeakChunkHandleMap;

    ChunkHandler(WorldDescriptors *descriptors);

    void init();

    void load(const std::string &name);

    void ioThread();
    void generatorThread();
    
    SharedChunkHandle getChunk(unsigned int hash);
    void removeHandle(ChunkHandleType *chunkHandle);

private:
    WorldDescriptors *m_descriptors;
    WorldGenerator<ChunkType> m_worldGenerator;

    std::string &m_cacheDirectory;

    std::thread m_ioThread;

    std::mutex m_generatorMutex;
    std::thread m_generatorThread;
    std::queue<SharedChunkHandle> m_generatorQueue;
    std::condition_variable m_generatorEvent;
    bool m_generatorThreadRun;

    WeakChunkHandleMap m_chunkHandles;
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

    m_generatorThreadRun=true;
    m_generatorThread=std::thread(std::bind(&ChunkHandler<_Chunk>::generatorThread, this));
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::load(const std::string &name)
{

}

template<typename _Chunk>
void ChunkHandler<_Chunk>::ioThread()
{
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::generatorThread()
{
    std::unique_lock<std::mutex> lock(m_generatorMutex);

    while(m_generatorThreadRun)
    {
        if(m_generatorQueue.empty())
        {
            m_generatorEvent.wait(lock);
            continue;
        }

        SharedChunkHandle chunkHandle=m_generatorQueue.pop_front();

        if(!chunkHandle)
            return;

        m_worldGenerator->generatechunk(chunkHandle->)
    }
}

template<typename _Chunk>
typename ChunkHandler<_Chunk>::SharedChunkHandle ChunkHandler<_Chunk>::getChunk(unsigned int hash)
{
    auto iter=m_chunkHandles.find(hash);

    if(iter!=m_chunkHandles.end())
    {
        if(!iter.second.expired())
            return iter.second.lock();
    }
        

    SharedChunkHandle sharedHandle;
    ChunkHandleType *chunkHandle=new ChunkHandleType();

    sharedHandle.reset(chunkHandle, std::bind(&ChunkHandler<_Chunk>::removeHandle, this, std::placeholders::_1));
    m_chunkHandles[hash]=sharedHandle;

    {
        std::unique_lock<std::mutex> lock(m_generatorMutex);

        m_generatorQueue.push_back(sharedHandle);
        m_generatorEvent.notify();
    }
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::removeHandle(ChunkHandleType *chunkHandle)
{
    auto iter=m_chunkHandles.find(chunkHandle->getHash());

    if(iter!=m_chunkHandles.end())
        m_chunkHandles.erase(iter);

    //update cache if not already saved

}

} //namespace voxigen

#endif //_voxigen_chunkHandler_h_