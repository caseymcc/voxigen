#ifndef _voxigen_queueThread_h_
#define _voxigen_queueThread_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/processRequests.h"

#include <memory>
#include <thread>
#include <queue>
#include <mutex>

namespace voxigen
{

class QueueThread
{
public:
    typedef std::vector<process::Request *> RequestQueue;

    QueueThread(std::condition_variable *completeEvent);

    void setCallback(process::Callback callback);

    void start(size_t threadCount=1);
    void stop();

    void updateQueue(RequestQueue &queue, RequestQueue &cancelQueue, RequestQueue &completedQueue, bool forceResort);

    //thread function
    void process();

    bool defaultCallback(process::Request *request) { return true; }

private:
    void insertRequests(RequestQueue &queue, RequestQueue &requests);
    void resortQueue(RequestQueue &queue);
    bool removeRequests(RequestQueue &queue, RequestQueue &cancelQueue, RequestQueue &completed);

    process::Callback processRequest;

    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
    std::condition_variable m_event;
    std::condition_variable *m_completeEvent;

    bool m_run;
    RequestQueue m_queue;
    RequestQueue m_completedQueue;
};

}//namespace voxigen

#endif //_voxigen_queueThread_h_