#ifndef _voxigen_processingThread_h_
#define _voxigen_processingThread_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"
#include "voxigen/updateQueue.h"
#include "voxigen/volume/chunkHandle.h"
#include "voxigen/processRequests.h"
#include "voxigen/queueThread.h"
#include "voxigen/fileio/log.h"

#include <generic/objectHeap.h>

#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

namespace voxigen
{

VOXIGEN_EXPORT unsigned int getProcessorCount();

class VOXIGEN_EXPORT ProcessThread
{
public:
    typedef std::vector<process::Request *> RequestQueue;

    ProcessThread();

    void setRequestSize(size_t size);
    void setSizes(glm::ivec3 &regionSize, glm::ivec3 &chunkSize);

    void setIoRequestCallback(process::Callback callback);
    void setChunkRequestCallback(process::Callback callback);
    void setMeshRequestCallback(process::Callback callback);

    void updateQueues(RequestQueue &completedRequests);

    void start();
    void stop();

    //thread actions
    bool updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk);

    template<typename _Object>
    bool requestChunkGenerate(_Object *chunkHandle, size_t lod);
    
    template<typename _Object>
    bool cancelChunkGenerate(_Object *chunkHandle);
    
    template<typename _Object>
    bool requestChunkRead(_Object *chunkHandle, size_t lod);
    
    template<typename _Object>
    bool cancelChunkRead(_Object *chunkHandle);
    
    template<typename _Object>
    bool requestChunkWrite(_Object *chunkHandle, size_t lod);
    
    template<typename _Object>
    bool cancelChunkWrite(_Object *chunkHandle);
    
    template<typename _Object, typename _Mesh>
    bool requestChunkMesh(_Object *renderer, _Mesh *mesh);
    
    template<typename _Object>
    bool cancelChunkMesh(_Object *renderer);

//    template<typename _Object>
//    bool returnMesh(_Object *renderer, ChunkTextureMesh *mesh);

    process::Request *getRequest();
    void insertRequest(process::Request *request);
    void releaseRequest(process::Request *request);

    //coordination thread
    void processThread();
    
    // callback for worker threads
    bool processWorkerRequest(process::Request *request);

    bool defaultCallback(process::Request *request) { return true; }

private:
    bool requestMeshAction(process::Type type, size_t priority, void *renderer, void *mesh, const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);
    bool requestChunkAction(process::Type type, size_t priority, void *chunkHandle, size_t lod, const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);

//    void updatePriorityQueue();

#ifndef NDEBUG
    bool checkRequestThread();
#endif

    std::thread m_thread;
    std::condition_variable m_event;

    process::Callback processChunkRequest;
    process::Callback processMeshRequest;

//only accessible from 1 thread, generally main thread
    generic::ObjectHeap<process::Request> m_requests;
    RequestQueue m_requestQueue;
    RequestQueue m_requestsComplete;
#ifndef NDEBUG
    //used to verify single thread access
    std::thread::id m_requestThreadId;
    bool m_requestThreadIdSet;
#endif

//can only be accessed under lock
    std::mutex m_queueMutex;
    bool m_run;
    RequestQueue m_requestThreadQueue;
    RequestQueue m_completedThreadQueue;
#ifndef NDEBUG
    //used to verify single thread access
//    std::thread::id m_requestThreadId;
#endif
    
//    generic::ObjectHeap<ChunkTextureMesh> m_meshHeap;

    QueueThread m_ioThread;
    QueueThread m_workerThread;
};

VOXIGEN_EXPORT ProcessThread &getProcessThread();

template<typename _Object>
bool ProcessThread::requestChunkGenerate(_Object *chunkHandle, size_t lod)
{
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread request generate chunk(%d, %d): %llx, %d", chunkHandle->regionHash(), chunkHandle->hash(), chunkHandle, lod);
#endif//DEBUG_RENDERERS
    return requestChunkAction(process::Type::Generate, process::Priority::Generate, (void *)chunkHandle, lod, chunkHandle->regionIndex(), chunkHandle->chunkIndex()); 
}

template<typename _Object>
bool ProcessThread::cancelChunkGenerate(_Object *chunkHandle)
{ 
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread cancel generate chunk: %llx", chunkHandle);
#endif//DEBUG_RENDERERS
    return requestChunkAction(process::Type::CancelGenerate, process::Priority::CancelGenerate, (void *)chunkHandle, 0, chunkHandle->regionIndex(), chunkHandle->chunkIndex()); 
}

template<typename _Object>
bool ProcessThread::requestChunkRead(_Object *chunkHandle, size_t lod)
{ 
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread request read chunk: %llx, %d", chunkHandle, lod);
#endif//DEBUG_RENDERERS
    return requestChunkAction(process::Type::Read, process::Priority::Read, (void *)chunkHandle, lod, chunkHandle->regionIndex(), chunkHandle->chunkIndex()); 
}

template<typename _Object>
bool ProcessThread::cancelChunkRead(_Object *chunkHandle)
{ 
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread cancel read chunk: %llx", chunkHandle);
#endif//DEBUG_RENDERERS
    return requestChunkAction(process::Type::CancelRead, process::Priority::CancelRead, (void *)chunkHandle, 0, chunkHandle->regionIndex(), chunkHandle->chunkIndex()); 
}

template<typename _Object>
bool ProcessThread::requestChunkWrite(_Object *chunkHandle, size_t lod)
{ 
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread request write chunk: %llx, %d", chunkHandle, lod);
#endif//DEBUG_RENDERERS
    return requestChunkAction(process::Type::Write, process::Priority::Write, (void *)chunkHandle, lod, chunkHandle->regionIndex(), chunkHandle->chunkIndex()); 
}

template<typename _Object>
bool ProcessThread::cancelChunkWrite(_Object *chunkHandle)
{ 
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread cancel write chunk: %llx", chunkHandle);
#endif//DEBUG_RENDERERS
    return requestChunkAction(process::Type::CancelWrite, process::Priority::CancelWrite, (void *)chunkHandle, 0, chunkHandle->regionIndex(), chunkHandle->chunkIndex()); 
}

template<typename _Object, typename _Mesh>
bool ProcessThread::requestChunkMesh(_Object *renderer, _Mesh *mesh)
{ 
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread request mesh chunk: %llx, %llx", renderer->getHandle().get(), renderer);
#endif//DEBUG_RENDERERS
    return requestMeshAction(process::Type::Mesh, process::Priority::Mesh, (void *)renderer, (void *)mesh, renderer->getRegionIndex(), renderer->getChunkIndex());
}

template<typename _Object>
bool ProcessThread::cancelChunkMesh(_Object *renderer)
{ 
#ifdef DEBUG_THREAD
    Log::debug("MainThread - ProcessThread cancel mesh chunk: %llx, %llx", renderer->getHandle().get(), renderer);
#endif//DEBUG_RENDERERS
    return requestMeshAction(process::Type::CancelMesh, process::Priority::CancelMesh, (void *)renderer, nullptr, renderer->getRegionIndex(), renderer->getChunkIndex()); 
}

//template<typename _Object>
//bool ProcessThread::returnMesh(_Object *renderer, ChunkTextureMesh *mesh)
//{
//#ifdef DEBUG_THREAD
//    Log::debug("MainThread - ProcessThread mesh return: %llx", mesh);
//#endif//DEBUG_RENDERERS
//    return requestMeshAction(process::Type::MeshReturn, process::Priority::MeshReturn, (void *)nullptr , mesh, renderer->getRegionIndex(), renderer->getChunkIndex());
//}

}//namespace voxigen

#endif //_voxigen_processingThread_h_