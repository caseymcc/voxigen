#include "voxigen/processingThread.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace voxigen
{

unsigned int getProcessorCount()
{
    unsigned int numProcessors=1;

#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    
    numProcessors=sysinfo.dwNumberOfProcessors;
#else
    numProcessors=sysconf(_SC_NPROCESSORS_ONLN);
#endif

    return numProcessors;
}

ProcessThread &getProcessThread()
{
    static ProcessThread processThread;

    return processThread;
}

ProcessThread::ProcessThread():
    m_ioThread(&m_event),
    m_workerThread(&m_event),
    m_requests(64)
#ifndef NDEBUG
    , m_requestThreadIdSet(false)
#endif
{
    m_workerThread.setCallback(std::bind(&ProcessThread::processWorkerRequest, this, std::placeholders::_1));

    processChunkRequest=std::bind(&ProcessThread::defaultCallback, this, std::placeholders::_1);
    processMeshRequest=std::bind(&ProcessThread::defaultCallback, this, std::placeholders::_1);
}

void ProcessThread::setRequestSize(size_t size)
{
    m_requests.setMaxSize(size);
}

void ProcessThread::setSizes(glm::ivec3 &regionSize, glm::ivec3 &chunkSize)
{
    process::Request::regionSize=regionSize;
    process::Request::chunkSize=chunkSize;
}

void ProcessThread::setIoRequestCallback(process::Callback callback)
{
    m_workerThread.setCallback(callback);
}

void ProcessThread::setChunkRequestCallback(process::Callback callback)
{
    processChunkRequest=callback;
}

void ProcessThread::setMeshRequestCallback(process::Callback callback)
{
    processMeshRequest=callback;
}

void ProcessThread::updateQueues(RequestQueue &completedRequests)
{
    assert(checkRequestThread());

    bool update=false;
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        //send all cached request to coordination thread
        if(!m_requestQueue.empty())
        {
            m_requestThreadQueue.insert(m_requestThreadQueue.end(), m_requestQueue.begin(), m_requestQueue.end());
            m_requestQueue.clear();
//#ifdef DEBUG_THREAD
//            for(process::Request *request:m_requestThreadQueue)
//                Log::debug("ProcessThread m_requestThreadQueue requests %llx", request->data.chunk.handle);
//#endif//DEBUG_RENDERERS
            update=true;
        }

        //get all completed request
        if(!m_requestsComplete.empty())
        {
            completedRequests.insert(completedRequests.end(), m_requestsComplete.begin(), m_requestsComplete.end());
            m_requestsComplete.clear();
        }
    }

    if(update)
        m_event.notify_all();
}


void ProcessThread::start()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=true;
    }

    m_thread=std::thread(std::bind(&ProcessThread::processThread, this));

    m_ioThread.start();

    //want the number of physical processors vs threads
//    int hardwareThreads=std::thread::hardware_concurrency()-1;
    int hardwareThreads=getProcessorCount()-1;

    if(hardwareThreads<0)
        hardwareThreads=1;

    m_workerThread.start(hardwareThreads);
}


void ProcessThread::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=false;
    }

    m_event.notify_all();
    m_thread.join();

    m_ioThread.stop();
    m_workerThread.stop();
}

bool ProcessThread::updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk)
{
    process::Request *request=getRequest();

#ifdef DEBUG_REQUESTS
    Log::debug("updatePosition get request %llx", request);
#endif

    if(!request)
        return false;

    request->type=process::Type::UpdatePos;
    request->priority=process::Priority::UpdatePos;

    request->position.region=region;
    request->position.chunk=chunk;

    insertRequest(request);
    return true;
}

bool ProcessThread::requestMeshAction(process::Type type, size_t priority, void *renderer, void *mesh, const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    process::Request *request=getRequest();

#ifdef DEBUG_REQUESTS
    Log::debug("requestMeshAction get request %llx", request);
#endif

    if(!request)
        return false;

    request->type=type;
    request->priority=priority;

    request->position.region=regionIndex;
    request->position.chunk=chunkIndex;

    request->data.buildMesh.renderer=(void *)renderer;
    request->data.buildMesh.mesh=mesh;

    insertRequest(request);
    return true;
}

bool ProcessThread::requestChunkAction(process::Type type, size_t priority, void *chunkHandle, size_t lod, const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    process::Request *request=getRequest();

#ifdef DEBUG_REQUESTS
    Log::debug("requestChunkAction get request %llx", request);
#endif

    if(!request)
        return false;

    request->type=type;
    request->priority=priority;

    request->position.region=regionIndex;
    request->position.chunk=chunkIndex;

    request->data.chunk.handle=(void *)chunkHandle;
    request->data.chunk.lod=lod;

    insertRequest(request);
    return true;
}

process::Request *ProcessThread::getRequest()
{
    assert(checkRequestThread());

    return m_requests.get();
}

void ProcessThread::insertRequest(process::Request *request)
{
    assert(checkRequestThread());

    assert(std::find(m_requestQueue.begin(), m_requestQueue.end(), request)==m_requestQueue.end());

    m_requestQueue.push_back(request);
    m_event.notify_all();
}

void ProcessThread::releaseRequest(process::Request *request)
{
    assert(checkRequestThread());

    m_requests.release(request);
}

void ProcessThread::processThread()
{
    bool run=true;
    RequestQueue requestQueue;
    RequestQueue completedQueue;
    RequestQueue cancelRequests;

    RequestQueue currentQueue;
    RequestQueue ioQueue;
    RequestQueue ioCancelQueue;
    RequestQueue workerQueue;
    RequestQueue workerCancelQueue;
    RequestQueue meshWaitQueue;

//    RequestQueue priorityQueue;

    while(run)
    {
//        bool completed;

        //check for request
//        if(requestQueue.empty())// || (m_requestCached>0)) <-possible use of atomic rather than lock
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            //update status
            run=m_run;

            //check if any new request have been added.
            if(!m_requestThreadQueue.empty())
            {
                requestQueue.insert(requestQueue.end(), m_requestThreadQueue.begin(), m_requestThreadQueue.end());
                m_requestThreadQueue.clear();
            }

            //while we have the lock update anything that is complete
            if(!completedQueue.empty())
            {
                m_requestsComplete.insert(m_requestsComplete.end(), completedQueue.begin(), completedQueue.end());
                completedQueue.clear();
            }

            if(run && requestQueue.empty() && meshWaitQueue.empty())
                m_event.wait(lock);

            //            continue;
        }

        //push any mesh requests
        if(!meshWaitQueue.empty())
        {
            requestQueue.insert(requestQueue.end(), meshWaitQueue.begin(), meshWaitQueue.end());
            meshWaitQueue.clear();
        }

        //need to sort request into current thread, ioThread and general workerThread
        for(size_t i=0; i<requestQueue.size(); ++i)
        {
            process::Request *request=requestQueue[i];

            switch(request->type)
            {
            case process::Type::UpdatePos:
                currentQueue.push_back(request);
                break;
            case process::Type::Read:
            case process::Type::Write:
                ioQueue.push_back(request);
                break;
            case process::Type::CancelRead:
            case process::Type::CancelWrite:
                ioCancelQueue.push_back(request);
            case process::Type::Generate:
                workerQueue.push_back(request);
                break;
            case process::Type::Mesh:
                {
//                    ChunkTextureMesh *mesh=m_meshHeap.get();
//
//                    if(mesh)
//                    {
//                        request->data.buildMesh.mesh=mesh;
//                        workerQueue.push_back(request);
//                    }
//                    else
//                    {
//                        //no mesh, need to wait for mesh
//                        meshWaitQueue.push_back(request);
//                    }
                    workerQueue.push_back(request);
                }
                break;
            case process::Type::MeshReturn:
                currentQueue.push_back(request);
                break;
            case process::Type::CancelGenerate:
            case process::Type::CancelMesh:
                workerCancelQueue.push_back(request);
                break;
            }
        }
        requestQueue.clear();

        bool forceResort=false;
        //process updates
        for(size_t i=0; i<currentQueue.size(); ++i)
        {
            process::Request *request=currentQueue[i];

            if(request->type==process::Type::UpdatePos)
            {
                process::Compare::currentRegion=request->position.region;
                process::Compare::currentChunk=request->position.chunk;

#ifdef DEBUG_THREAD
                Log::debug("ProcessThread running update pos");
#endif//DEBUG_RENDERERS

                forceResort=true;
            }
//            else if(request->type==process::Type::MeshReturn)
//            {
//#ifdef DEBUG_THREAD
//                Log::debug("ProcessThread releasing mesh %llx", request->data.buildMesh.mesh);
//#endif//DEBUG_RENDERERS
//                m_meshHeap.release(request->data.buildMesh.mesh);
//                request->data.buildMesh.mesh=nullptr;
//            }

            completedQueue.push_back(request);
        }
        currentQueue.clear();

        m_ioThread.updateQueue(ioQueue, ioCancelQueue, completedQueue, forceResort);
        assert(ioQueue.size()==0);
//#ifdef DEBUG_THREAD
//        for(process::Request *request:workerQueue)
//            Log::debug("ProcessThread queue requests %llx", request->data.chunk.handle);
//#endif//DEBUG_RENDERERS
        m_workerThread.updateQueue(workerQueue, workerCancelQueue, completedQueue, forceResort);
        assert(workerQueue.size()==0);
    }
}

bool ProcessThread::processWorkerRequest(process::Request *request)
{
    switch(request->type)
    {
    case process::Type::Generate:
        processChunkRequest(request);
        break;
    case process::Type::Mesh:
        processMeshRequest(request);
        break;
    default:
        break;
    }
    return true;
}

#ifndef NDEBUG
bool ProcessThread::checkRequestThread()
{
    if(!m_requestThreadIdSet)
    {
        m_requestThreadId=std::this_thread::get_id();
        m_requestThreadIdSet=true;
    }
    return (std::this_thread::get_id()==m_requestThreadId);
}
#endif

}//namespace voxigen

