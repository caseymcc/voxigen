
namespace voxigen
{

template<typename _Request>
ProcessQueueThread<_Request>::ProcessQueueThread<_Request>(ProcessFunction function, int interuptCount=10):
    process(function),
    m_interuptCount(interuptCount);
{
}

template<typename _Request>
void ProcessQueueThread<_Request>::addRequest(Request *request)
{
    m_requestCache.push_back(request);
}

template<typename _Request>
void ProcessQueueThread<_Request>::updateQueues(Requests &completedQueue)
{
    bool update=false;

    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        if(!m_requestCache.empty())
        {
            m_requestQueue.insert(m_requestQueue.end(), m_requestCache.begin(), m_requestCache.end());
            m_requestCache.clear();
            update=true;

        }

        if(!m_completedQueue.empty())
        {
            completedQueue.insert(completedQueue.end(), m_completedQueue.begin(), m_completedQueue.end());
            m_completedQueue.clear();
        }
    }

    if(update)
        m_event.notify_all();
}

template<typename _Request>
void ProcessQueueThread<_Request>::start()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=true;
    }

    m_thread=std::thread(std::bind(&ProcessQueueThread<_Request>::processThread, this));
}


template<typename _Request>
void ProcessQueueThread<_Request>::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=false;
    }

    m_event.notify_all();
    m_thread.join();
}


template<typename _Request>
void ProcessQueueThread<_Request>::processThread()
{
    bool run=true;
    int interuptCount=1;
    int count=0;
    std::deque<Request *> requestQueue;
    Requests completedQueue;

    while(run)
    {
        bool completed;

        if(requestQueue.empty() || (count>interuptCount))
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            //update status
            run=m_run;
            interuptCount=m_interuptCount;

            //check if any new request have been added.
            if(!m_requestQueue.empty())
            {
                requestQueue.insert(requestQueue.end(), m_requestQueue.begin(), m_requestQueue.end());
                m_requestQueue.clear();
            }

            //while we have the lock update anything that is complete
            if(!completedQueue.empty())
            {
                m_completedQueue.insert(m_completedQueue.end(), completedQueue.begin(), completedQueue.end());
                completedQueue.clear();
            }

            if(run && requestQueue.empty())
                m_event.wait(lock);

            count=0;
            continue;
        }

        Request *request=requestQueue.front();

        requestQueue.pop_front();
        completed=process(request);

        if(completed)
            completedQueue.push_back(request);
        else//wasn't able to process, lets try again, careful could cause crazy process usage if it cannot be processed ever
            requestQueue.push_front(request);
    }
}

void ProcessQueueThread<_Request>::setInteruptCount(int count)
{
    m_interuptCount=count;
}

}//namespace voxigen

