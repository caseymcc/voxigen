#ifndef _voxigen_processQueue_h_
#define _voxigen_processQueue_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"
#include "voxigen/updateQueue.h"
#include "voxigen/volume/chunkHandle.h"

#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

namespace voxigen
{

namespace Process
{
    enum Type
    {
        UpdateQueue,
        Generate,
        Read,
        Write,
        Update,
        Cancel,
        Release,
        GenerateRegion
    };
}

struct ProcessRequest
{
    ProcessRequest(Process::Type type, unsigned int priority):type(type), priority(priority)
    {
    }
    virtual ~ProcessRequest() {}

    virtual float distance(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex) { return 0.0; }

    Process::Type type;
    unsigned int priority;
};

template<typename _Region, typename _Chunk>
struct ChunkProcessRequest:public ProcessRequest
{
    typedef ChunkProcessRequest<_Region, _Chunk> Type;

    typedef ChunkHandle<_Chunk> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
    typedef std::weak_ptr<ChunkHandleType> WeakChunkHandle;

    ChunkProcessRequest(Process::Type type, unsigned int priority, glm::ivec3 regionIndex, glm::ivec3 chunkIndex):ProcessRequest(type, priority), regionIndex(regionIndex), chunkIndex(chunkIndex)
    {
#ifdef LOG_PROCESS_QUEUE
        distance=-1.0f;
#endif
    }
    virtual ~ChunkProcessRequest() {}

    float distance(const glm::ivec3 &currentRegionIndex, const glm::ivec3 &currentChunkIndex) override
    {
        return details::distance<_Region, _Chunk>(currentRegionIndex, currentChunkIndex, regionIndex, chunkIndex);
    }

    virtual SharedChunkHandle getChunkHandle()=0;

#ifdef LOG_PROCESS_QUEUE
    float distance;
#endif

    glm::ivec3 regionIndex;
    glm::ivec3 chunkIndex;
};

typedef std::shared_ptr<ProcessRequest> SharedProcessRequest;


struct UpdateQueueRequest:public ProcessRequest
{
    UpdateQueueRequest(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex):ProcessRequest(Process::Type::UpdateQueue, 20), regionIndex(regionIndex), chunkIndex(chunkIndex){}

    glm::ivec3 regionIndex;
    glm::ivec3 chunkIndex;
};

template<typename _Region>
struct GenerateRegionRequest:public ProcessRequest
{
    typedef RegionHandle<_Region> RegionHandleType;
    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;
    typedef std::weak_ptr<RegionHandleType> WeakRegionHandle;

    GenerateRegionRequest(SharedRegionHandle handle, size_t lod):ProcessRequest(Process::Type::GenerateRegion, 110), handle(handle), lod(lod), regionIndex(handle->getRegionIndex()){}

    float distance(const glm::ivec3 &currentRegionIndex, const glm::ivec3 &currentChunkIndex) override
    {
        return glm::length(glm::vec3(currentRegionIndex-regionIndex));
    }

    SharedRegionHandle getHandle() { return handle.lock(); }

    WeakRegionHandle handle;
    glm::ivec3 regionIndex;
    size_t lod;

#ifdef LOG_PROCESS_QUEUE
    float distance;
#endif
};

template<typename _Region, typename _Chunk>
struct GenerateRequest:public ChunkProcessRequest<_Region, _Chunk>
{
    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;

    GenerateRequest(SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Generate, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod){}
    GenerateRequest(unsigned int priority, SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Generate, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}

    SharedChunkHandle getChunkHandle() override{ return chunkHandle.lock(); }

    WeakChunkHandle chunkHandle;
    size_t lod;
};

template<typename _Region, typename _Chunk>
struct ReadRequest:public ChunkProcessRequest<_Region, _Chunk>
{
    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;

    ReadRequest(SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Read, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}
    ReadRequest(unsigned int priority, SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Read, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}

    SharedChunkHandle getChunkHandle() override{ return chunkHandle.lock(); }

    WeakChunkHandle chunkHandle;
    size_t lod;
};

template<typename _Region, typename _Chunk>
struct WriteRequest:public ChunkProcessRequest<_Region, _Chunk>
{
    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;

    WriteRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Write, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}
    WriteRequest(unsigned int priority, SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Write, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()){}

    SharedChunkHandle getChunkHandle() override { return chunkHandle; }

    SharedChunkHandle chunkHandle;
};

template<typename _Region, typename _Chunk>
struct UpdateRequest:public ChunkProcessRequest<_Region, _Chunk>
{
    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;

    UpdateRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Update, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}
    UpdateRequest(unsigned int priority, SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Update, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}

    SharedChunkHandle getChunkHandle() override { return chunkHandle.lock(); }

    WeakChunkHandle chunkHandle;
};

template<typename _Region, typename _Chunk>
struct CancelRequest:public ChunkProcessRequest<_Region, _Chunk>
{
    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;
    CancelRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Cancel, 9, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}

    SharedChunkHandle getChunkHandle() override { return chunkHandle.lock(); }

    WeakChunkHandle chunkHandle;
};

template<typename _Region, typename _Chunk>
struct ReleaseRequest:public ChunkProcessRequest<_Region, _Chunk>
{
    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;

    ReleaseRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Release, 10, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}
    ReleaseRequest(unsigned int priority, SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Update, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}

    SharedChunkHandle getChunkHandle() override { return chunkHandle.lock(); }

    WeakChunkHandle chunkHandle;
};

//template<typename _Chunk>
//ChunkHandle<_Chunk> *getChunkHandleFromRequest(ProcessRequest<_Chunk> *request)
//{
//    ChunkHandle<_Chunk> *chunkHandle;
//
//    switch(request->type)
//    {
//    case Process::Type::Generate:
//        {
//            GenerateRequest<_Chunk> *generateRequest=(GenerateRequest<_Chunk> *)request;
//
//            chunkHandle=generateRequest->chunkHandle.lock().get();
//        }
//        break;
//    case Process::Type::Read:
//        {
//            ReadRequest<_Chunk> *readRequest=(ReadRequest<_Chunk> *)request;
//
//            chunkHandle=readRequest->chunkHandle.lock().get();
//        }
//        break;
//    case Process::Type::Write:
//        {
//            WriteRequest<_Chunk> *writeRequest=(WriteRequest<_Chunk> *)request;
//
//            chunkHandle=writeRequest->chunkHandle.get();
//        }
//        break;
//    case Process::Type::Update:
//        {
//            UpdateRequest<_Chunk> *updateRequest=(UpdateRequest<_Chunk> *)request;
//
//            chunkHandle=updateRequest->chunkHandle.lock().get();
//        }
//        break;
//    case Process::Type::Release:
//        {
//            ReleaseRequest<_Chunk> *releaseRequest=(UpdateRequest<_Chunk> *)request;
//
//            chunkHandle=releaseRequest->chunkHandle.lock().get();
//        }
//        break;
//    }
//
//    return chunkHandle;
//}

template<typename _Chunk>
struct ProcessCompare
{
    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    bool operator()(const SharedProcessRequest &request1, const SharedProcessRequest &request2) const
    {
        if(request1->priority!=request2->priority)
            return (request1->priority>request2->priority);
        else
        {
            float handle1Distance=request1->distance(currentRegion, currentChunk);
            float handle2Distance=request2->distance(currentRegion, currentChunk);

#ifdef LOG_PROCESS_QUEUE
            request1->distance=handle1Distance;
            request2->distance=handle2Distance;
#endif
            return (handle1Distance>handle2Distance);
        }
    }

    static IGridDescriptors *descriptor;
    static glm::ivec3 currentRegion;
    static glm::ivec3 currentChunk;
};

template<typename _Chunk>
IGridDescriptors *ProcessCompare<_Chunk>::descriptor=nullptr;

template<typename _Chunk>
glm::ivec3 ProcessCompare<_Chunk>::currentRegion=glm::ivec3(0, 0, 0);

template<typename _Chunk>
glm::ivec3 ProcessCompare<_Chunk>::currentChunk=glm::ivec3(0, 0, 0);

template<typename _Grid>
class ProcessQueue
{
public:
    typedef typename _Grid::DescriptorType DescriptorType;

    typedef typename _Grid::RegionType Region;
    typedef RegionHandle<Region> RegionHandleType;
    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;

    typedef typename _Grid::ChunkType ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
    typedef SharedProcessRequest SharedRequest;

    typedef std::vector<SharedProcessRequest> RequestQueue;

    typedef UpdateQueueRequest UpdateQueueRequestType;
    typedef GenerateRegionRequest<Region> GenerateRegionRequestType;
    typedef GenerateRequest<Region, ChunkType> GenerateRequestType;
    typedef ReadRequest<Region, ChunkType> ReadRequestType;
    typedef WriteRequest<Region, ChunkType> WriteRequestType;
    typedef UpdateRequest<Region, ChunkType> UpdateRequestType;
    typedef CancelRequest<Region, ChunkType> CancelRequestType;
    typedef ReleaseRequest<Region, ChunkType> ReleaseRequestType;

    ProcessQueue(DescriptorType *descriptor):
#ifndef NDEBUG
        m_inputThreadIdSet(false),
        m_outputThreadIdSet(false),
#endif
        m_checkForUpdates(0),
        m_descriptor(descriptor) 
    { ProcessCompare<ChunkType>::descriptor=descriptor; }

    void updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk);
    void addGenerateRegion(SharedRegionHandle handle, size_t lod);
    void addGenerate(SharedChunkHandle chunkHandle, size_t lod);
    void addRead(SharedChunkHandle chunkHandle, size_t lod);
    void addWrite(SharedChunkHandle chunkHandle);
    void addUpdate(SharedChunkHandle chunkHandle);
    void addCancel(SharedChunkHandle chunkHandle);
    void addRelease(SharedChunkHandle chunkHandle);

    bool empty() { return m_priorityQueue.empty(); }

    SharedRequest getNextProcessRequest();
    void removeRequests(RequestQueue &cancelRequests);
    
    void updateQueue();
    void updatePriorityQueue();

    std::unique_lock<std::mutex> getLock();
    void trigger(std::unique_lock<std::mutex> &lock);
    void wait(std::unique_lock<std::mutex> &lock);

    void addCompletedRequest(SharedRequest request);
    void updateCompletedQueue();
    void getCompletedQueue(RequestQueue &completedQueue);
    bool isCompletedQueueEmpty();

//    void updatePriority();
private:
    void checkInputThread();
    void checkOutputThread();

#ifndef NDEBUG
    std::thread::id m_inputThreadId;
    bool m_inputThreadIdSet;
    std::thread::id m_outputThreadId;
    bool m_outputThreadIdSet;
#endif

    DescriptorType *m_descriptor;
    std::mutex m_processMutex;

    std::mutex m_queueMutex;
    std::condition_variable m_queueEvent;
//    std::priority_queue<SharedProcessRequest<ChunkType>> m_queue;
    RequestQueue m_cacheQueue;
    RequestQueue m_queue;
    RequestQueue m_priorityQueue;

    std::mutex m_completedMutex;
    RequestQueue m_completedQueueCache;
    RequestQueue m_outputCompletedQueue;
    RequestQueue m_completedQueue;

    std::atomic<int> m_checkForUpdates;
};


}//namespace voxigen

#include "processQueue.inl"

#endif //_voxigen_processQueue_h_