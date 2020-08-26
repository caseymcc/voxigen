#include "voxigen/queueThread.h"
#include "voxigen/fileio/log.h"

namespace voxigen
{

QueueThread::QueueThread(std::condition_variable *completeEvent):
    m_completeEvent(completeEvent)
{
    processRequest=std::bind(&QueueThread::defaultCallback, this, std::placeholders::_1
);
}

void QueueThread::setCallback(process::Callback callback)
{
    processRequest=callback;
}

void QueueThread::start(size_t threadCount)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_run=true;
    }

    //create 
    for(unsigned int i=0; i<threadCount; ++i)
    {
        std::thread workerThread=std::thread(std::bind(&QueueThread::process, this));
        
        m_threads.push_back(std::move(workerThread));
    }
}


void QueueThread::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_run=false;
    }

    m_event.notify_all();
    for(unsigned int i=0; i<m_threads.size(); ++i)
        m_threads[i].join();
}

void QueueThread::updateQueue(RequestQueue &queue, RequestQueue &cancelQueue, RequestQueue &completedQueue, bool forceResort)
{
    bool notify=false;
    bool resort=forceResort;

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        //remove canceled request
        if(!cancelQueue.empty())
            resort=resort || removeRequests(m_queue, cancelQueue, completedQueue);
        
        //insert request to workerQueue
        if(!queue.empty())
        {
            if(resort)//we are going to resort so just insert them
            {
#ifdef DEBUG_THREAD
                for(process::Request *request:queue)
                    Log::debug("ProcessThread bulk inserting chunk %llx", request->data.chunk.handle);
#endif//DEBUG_RENDERERS
                m_queue.insert(m_queue.end(), queue.begin(), queue.end());
                queue.clear();
            }
            else
                insertRequests(m_queue, queue);
            notify=true;
        }

        if(resort)
            resortQueue(m_queue);

        if(!m_completedQueue.empty())
        {
#ifdef DEBUG_THREAD
            for(process::Request *request:m_completedQueue)
                Log::debug("ProcessThread completed request %llx", request);
#endif//DEBUG_THREAD
            completedQueue.insert(completedQueue.end(), m_completedQueue.begin(), m_completedQueue.end());
            m_completedQueue.clear();
        }
    }

    if(notify)
        m_event.notify_all();
}

void QueueThread::process()
{
    bool run=true;
    process::Request *request=nullptr;

    while(run)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            run=m_run;
            if(!run)
                break;

            if(request)
            {
#ifdef DEBUG_THREAD
                Log::debug("ProcessThread request complete %llx", request);
#endif//DEBUG_THREAD
                m_completedQueue.push_back(request);
                m_completeEvent->notify_all();
                request=nullptr;
            }

            if(!m_queue.empty())
            {
                std::pop_heap(m_queue.begin(), m_queue.end(), process::Compare());
                request=m_queue.back();
                m_queue.pop_back();
            }
            else
            {
                m_event.wait(lock);
                continue;
            }
        }

#ifdef DEBUG_THREAD
        Log::debug("ProcessThread processing request %llx", request);
#endif//DEBUG_THREAD
        processRequest(request);
    }
}

void QueueThread::insertRequests(RequestQueue &queue, RequestQueue &requests)
{
    for(process::Request *request:requests)
    {
#ifdef DEBUG_THREAD
        Log::debug("ProcessThread inserting chunk %llx", request->data.chunk.handle);
#endif//DEBUG_RENDERERS
        queue.push_back(request);
        std::push_heap(queue.begin(), queue.end(), process::Compare());
    }
    requests.clear();
}

void QueueThread::resortQueue(RequestQueue &queue)
{
    std::make_heap(queue.begin(), queue.end(), process::Compare());
}

bool QueueThread::removeRequests(RequestQueue &queue, RequestQueue &cancelQueue, RequestQueue &completed)
{
    bool resort=false;
    size_t removed=0;

    for(size_t i=0; i<queue.size(); )
    {
        bool increment=true;
        process::Request *request=queue[i];

        for(size_t j=0; j<cancelQueue.size(); ++j)
        {
            if(cancelQueue[j]==request)
            {
                //set request as canceled and add to completed
                queue[i]->result=process::Result::Canceled;
                completed.push_back(queue[i]);

                //remove current from queue by swapping the back
                queue[i]=queue.back();
                queue.pop_back();

                //remove request from cancelQueue by swapping the back
                cancelQueue[j]=cancelQueue.back();
                cancelQueue.pop_back();

                removed++;
                increment=false;
                break;
            }
        }

        //only increment if nothing removed
        if(increment)
            ++i;
    }

    if(removed>0)
        resort=true;//we removed some so going to need to resort

        //return all cancels that were not found
    for(size_t j=0; j<cancelQueue.size(); ++j)
        completed.push_back(cancelQueue[j]);
    cancelQueue.clear();

    return resort;
}

}//namespace voxigen

