#ifndef _voxigen_processQueueThread_h_
#define _voxigen_processQueueThread_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"
#include "voxigen/updateQueue.h"
#include "voxigen/chunkHandle.h"

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

template<typename _Request>
class ProcessQueueThread
{
public:
    typedef _Request Request;
    typedef std::vector<Request *> Requests;
    typedef std::function<bool(Request *)> ProcessFunction;

    ProcessQueueThread(ProcessFunction function, int interuptCount=10);

    void start();
    void stop();
    void processThread();

    void setInteruptCount(int count);

    void addRequest(Request *);
    void returnRequest(Request *);

    void updateQueues(Requests &completedRequests);

private:
    std::thread m_thread;
    std::condition_variable m_event;

    ProcessFunction process;
    //expected to be accessed from requesting thread
    bool m_run;
    Requests m_requestCache;
    int m_interuptCount;

    std::mutex m_queueMutex;
    //can only be accessed under lock
    MeshRequests m_requestQueue;
    MeshRequests m_completedQueue;
    
};


}//namespace voxigen

#include "processQueueThread.inl"

#endif //_voxigen_processQueueThread_h_