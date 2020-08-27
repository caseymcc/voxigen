#ifndef _voxigen_processQueue_h_
#define _voxigen_processQueue_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"
#include "voxigen/updateQueue.h"
#include "voxigen/volume/chunkHandle.h"

#include <generic/objectHeap.h>

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

namespace Priority
{
    const size_t UpdateQueue=10;
    const size_t Generate=30;
    const size_t Read=30;
    const size_t Write=50;
    const size_t Update=30;
    const size_t Cancel=20;
    const size_t GenerateRegion=40;
//    const size_t Release=15;
}//namespace Priority

namespace Result
{
    const size_t Success=0;
    const size_t Canceled=1;
}//namespace Result

template<typename _Region, typename _Chunk>
struct Request
{
    typedef RegionHandle<_Region> RegionHandleType;
    typedef ChunkHandle<_Chunk> ChunkHandleType;

    Request()
    {
#ifdef LOG_PROCESS_QUEUE
        distanceValue=-1.0f;
#endif
    }
    Request(Process::Type type, unsigned int priority):type(type), priority(priority)
    {
#ifdef LOG_PROCESS_QUEUE
        distanceValue=-1.0f;
#endif
    }
    virtual ~Request() {}

    virtual float distance(const glm::ivec3 &currentRegionIndex, const glm::ivec3 &currentChunkIndex) const
    { 
        return details::distance<_Region, _Chunk>(currentRegionIndex, currentChunkIndex, regionIndex, chunkIndex);
    }


#ifdef LOG_PROCESS_QUEUE
    mutable float distanceValue;
#endif
    Process::Type type;
    unsigned int priority;

    union Data
    {
        RegionHandleType *region;
        ChunkHandleType *chunk;
    } handle;

    glm::ivec3 regionIndex;
    glm::ivec3 chunkIndex;
    size_t lod;
    size_t result;
};

}//namespace Process



//template<typename _Region, typename _Chunk>
//struct ChunkProcessRequest:public ProcessRequest
//{
//    typedef ChunkProcessRequest<_Region, _Chunk> Type;
//
//    typedef ChunkHandle<_Chunk> ChunkHandleType;
//    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
//    typedef std::weak_ptr<ChunkHandleType> WeakChunkHandle;
//
//    ChunkProcessRequest(Process::Type type, unsigned int priority, glm::ivec3 regionIndex, glm::ivec3 chunkIndex):ProcessRequest(type, priority), regionIndex(regionIndex), chunkIndex(chunkIndex)
//    {
//    }
//    virtual ~ChunkProcessRequest() {}
//
//    float distance(const glm::ivec3 &currentRegionIndex, const glm::ivec3 &currentChunkIndex) override
//    {
//        return details::distance<_Region, _Chunk>(currentRegionIndex, currentChunkIndex, regionIndex, chunkIndex);
//    }
//
//    virtual SharedChunkHandle getChunkHandle()=0;
//
//    glm::ivec3 regionIndex;
//    glm::ivec3 chunkIndex;
//};
//
//typedef std::shared_ptr<ProcessRequest> SharedProcessRequest;
//
//
//struct UpdateQueueRequest:public ProcessRequest
//{
//    UpdateQueueRequest(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex):ProcessRequest(Process::Type::UpdateQueue, 20), regionIndex(regionIndex), chunkIndex(chunkIndex){}
//
//    glm::ivec3 regionIndex;
//    glm::ivec3 chunkIndex;
//};
//
//template<typename _Region>
//struct GenerateRegionRequest:public ProcessRequest
//{
//    typedef RegionHandle<_Region> RegionHandleType;
//    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;
//    typedef std::weak_ptr<RegionHandleType> WeakRegionHandle;
//
//    GenerateRegionRequest(SharedRegionHandle handle, size_t lod):ProcessRequest(Process::Type::GenerateRegion, 110), handle(handle), lod(lod), regionIndex(handle->getRegionIndex()){}
//
//    float distance(const glm::ivec3 &currentRegionIndex, const glm::ivec3 &currentChunkIndex) override
//    {
//        return glm::length(glm::vec3(currentRegionIndex-regionIndex));
//    }
//
//    SharedRegionHandle getHandle() { return handle.lock(); }
//
//    WeakRegionHandle handle;
//    glm::ivec3 regionIndex;
//    size_t lod;
//};
//
//template<typename _Region, typename _Chunk>
//struct GenerateRequest:public ChunkProcessRequest<_Region, _Chunk>
//{
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;
//
//    GenerateRequest(SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Generate, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod){}
//    GenerateRequest(unsigned int priority, SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Generate, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}
//
//    SharedChunkHandle getChunkHandle() override{ return chunkHandle.lock(); }
//
//    WeakChunkHandle chunkHandle;
//    size_t lod;
//};
//
//template<typename _Region, typename _Chunk>
//struct ReadRequest:public ChunkProcessRequest<_Region, _Chunk>
//{
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;
//
//    ReadRequest(SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Read, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}
//    ReadRequest(unsigned int priority, SharedChunkHandle chunkHandle, size_t lod):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Read, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}
//
//    SharedChunkHandle getChunkHandle() override{ return chunkHandle.lock(); }
//
//    WeakChunkHandle chunkHandle;
//    size_t lod;
//};
//
//template<typename _Region, typename _Chunk>
//struct WriteRequest:public ChunkProcessRequest<_Region, _Chunk>
//{
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;
//
//    WriteRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Write, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}
//    WriteRequest(unsigned int priority, SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Write, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()){}
//
//    SharedChunkHandle getChunkHandle() override { return chunkHandle; }
//
//    SharedChunkHandle chunkHandle;
//};
//
//template<typename _Region, typename _Chunk>
//struct UpdateRequest:public ChunkProcessRequest<_Region, _Chunk>
//{
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;
//
//    UpdateRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Update, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}
//    UpdateRequest(unsigned int priority, SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Update, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}
//
//    SharedChunkHandle getChunkHandle() override { return chunkHandle.lock(); }
//
//    WeakChunkHandle chunkHandle;
//};
//
//template<typename _Region, typename _Chunk>
//struct CancelRequest:public ChunkProcessRequest<_Region, _Chunk>
//{
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;
//    CancelRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Cancel, 9, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}
//
//    SharedChunkHandle getChunkHandle() override { return chunkHandle.lock(); }
//
//    WeakChunkHandle chunkHandle;
//};
//
//template<typename _Region, typename _Chunk>
//struct ReleaseRequest:public ChunkProcessRequest<_Region, _Chunk>
//{
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::SharedChunkHandle SharedChunkHandle;
//    typedef typename ChunkProcessRequest<_Region, _Chunk>::WeakChunkHandle WeakChunkHandle;
//
//    ReleaseRequest(SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Release, 10, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}
//    ReleaseRequest(unsigned int priority, SharedChunkHandle chunkHandle):ChunkProcessRequest<_Region, _Chunk>(Process::Type::Update, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}
//
//    SharedChunkHandle getChunkHandle() override { return chunkHandle.lock(); }
//
//    WeakChunkHandle chunkHandle;
//};

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

template<typename _Region, typename _Chunk>
struct ProcessCompare
{
    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    bool operator()(const Process::Request<_Region, _Chunk> *request1, const Process::Request<_Region, _Chunk> *request2) const
    {
        if(request1->priority!=request2->priority)
            return (request1->priority>request2->priority);
        else
        {
            float handle1Distance=request1->distance(currentRegion, currentChunk);
            float handle2Distance=request2->distance(currentRegion, currentChunk);

#ifdef LOG_PROCESS_QUEUE
            request1->distanceValue=handle1Distance;
            request2->distanceValue=handle2Distance;
#endif
            return (handle1Distance>handle2Distance);
        }
    }

    static IGridDescriptors *descriptor;
    static glm::ivec3 currentRegion;
    static glm::ivec3 currentChunk;
};

template<typename _Region, typename _Chunk>
IGridDescriptors *ProcessCompare<_Region, _Chunk>::descriptor=nullptr;

template<typename _Region, typename _Chunk>
glm::ivec3 ProcessCompare<_Region, _Chunk>::currentRegion=glm::ivec3(0, 0, 0);

template<typename _Region, typename _Chunk>
glm::ivec3 ProcessCompare<_Region, _Chunk>::currentChunk=glm::ivec3(0, 0, 0);

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
//    typedef SharedProcessRequest SharedRequest;

    typedef Process::Request<Region, ChunkType> ProcessRequest;

    typedef std::vector<ProcessRequest *> RequestQueue;

//    typedef UpdateQueueRequest UpdateQueueRequestType;
//    typedef GenerateRegionRequest<Region> GenerateRegionRequestType;
//    typedef GenerateRequest<Region, ChunkType> GenerateRequestType;
//    typedef ReadRequest<Region, ChunkType> ReadRequestType;
//    typedef WriteRequest<Region, ChunkType> WriteRequestType;
//    typedef UpdateRequest<Region, ChunkType> UpdateRequestType;
//    typedef CancelRequest<Region, ChunkType> CancelRequestType;
//    typedef ReleaseRequest<Region, ChunkType> ReleaseRequestType;

    ProcessQueue(DescriptorType *descriptor):
        m_requests(1024),
#ifndef NDEBUG
        m_inputThreadIdSet(false),
        m_outputThreadIdSet(false),
#endif
        m_checkForUpdates(0),
        m_descriptor(descriptor) 
    { ProcessCompare<Region, ChunkType>::descriptor=descriptor; }

    bool updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk);
    bool addGenerateRegion(RegionHandleType *handle, size_t lod);
    bool addGenerate(ChunkHandleType *chunkHandle, size_t lod);
    bool addRead(ChunkHandleType *chunkHandle, size_t lod);
    bool addWrite(ChunkHandleType *chunkHandle);
    bool addUpdate(ChunkHandleType *chunkHandle);
    bool addCancel(ChunkHandleType *chunkHandle);
    bool addRelease(ChunkHandleType *chunkHandle);
    void releaseRequest(ProcessRequest *request);

    bool empty() { return m_priorityQueue.empty(); }

    ProcessRequest *getNextProcessRequest();
    void removeRequests(RequestQueue &cancelRequests);

    size_t getRequestSize() { return m_requests.getMaxSize(); }
    void setRequestSize(size_t size) { return m_requests.setMaxSize(size); }
    
    void updateQueue();
    void updatePriorityQueue();

    std::unique_lock<std::mutex> getLock();
    void trigger(std::unique_lock<std::mutex> &lock);
    void wait(std::unique_lock<std::mutex> &lock);

    void addCompletedRequest(ProcessRequest *request);
    void updateCompletedQueue();
    void getCompletedQueue(RequestQueue &completedQueue);
    bool isCompletedQueueEmpty();

//    void updatePriority();
private:
    void checkInputThread();
    void checkOutputThread();

    void sortPriorityQueue();

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

    generic::ObjectHeap<ProcessRequest> m_requests;
};


}//namespace voxigen

#include "processQueue.inl"

#endif //_voxigen_processQueue_h_