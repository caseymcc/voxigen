#include <glog/logging.h>

namespace voxigen
{

template<typename _Grid>
void ProcessQueue<_Grid>::updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    ProcessCompare<ChunkType>::currentRegion=region;
    ProcessCompare<ChunkType>::currentChunk=chunk;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"Setting chunk pos ("<<region.x<<", "<<region.y<<", "<<region.z<<"  "<<chunk.x<<", "<<chunk.y<<", "<<chunk.z<<")";
#endif//LOG_PROCESS_QUEUE
    std::make_heap(m_queue.begin(), m_queue.end(), ProcessCompare<ChunkType>());
}

template<typename _Grid>
void ProcessQueue<_Grid>::addGenerate(SharedChunkHandle chunkHandle, size_t lod)
{
    std::shared_ptr<GenerateRequestType> request=std::make_shared<GenerateRequestType>(chunkHandle, lod);

    {
#ifdef LOG_PROCESS_QUEUE
        if(chunkHandle)
        {
            glm::ivec3 regionIndex=chunkHandle->regionIndex();
            glm::ivec3 chunkIndex=chunkHandle->chunkIndex();

            LOG(INFO)<<"Adding generate chunk ("<<regionIndex.x<<", "<<regionIndex.y<<", "<<regionIndex.z<<"  "<<chunkIndex.x<<", "<<chunkIndex.y<<", "<<chunkIndex.z<<")";
        }
#endif

        std::unique_lock<std::mutex> lock(m_mutex);

//        m_queue.push(request);
        m_queue.push_back(request);
        std::push_heap(m_queue.begin(), m_queue.end(), ProcessCompare<ChunkType>());
    }
    m_event.notify_all();
}

template<typename _Grid>
void ProcessQueue<_Grid>::addRead(SharedChunkHandle chunkHandle, size_t lod)
{
    std::shared_ptr<ReadRequestType> request=std::make_shared<ReadRequestType>(chunkHandle, lod);

    {
        std::unique_lock<std::mutex> lock(m_mutex);

//        m_queue.push(request);
        m_queue.push_back(request);
        std::push_heap(m_queue.begin(), m_queue.end(), ProcessCompare<ChunkType>());
    }
    m_event.notify_all();
}

template<typename _Grid>
void ProcessQueue<_Grid>::addWrite(SharedChunkHandle chunkHandle)
{
    std::shared_ptr<WriteRequestType> request=std::make_shared<WriteRequestType>(chunkHandle);

    {
        std::unique_lock<std::mutex> lock(m_mutex);

//        m_queue.push(request);
        m_queue.push_back(request);
        std::push_heap(m_queue.begin(), m_queue.end(), ProcessCompare<ChunkType>());
    }
    m_event.notify_all();
}

template<typename _Grid>
void ProcessQueue<_Grid>::addUpdate(SharedChunkHandle chunkHandle)
{
    std::shared_ptr<UpdateRequestType> request=std::make_shared<UpdateRequestType>(chunkHandle);

    {
        std::unique_lock<std::mutex> lock(m_mutex);

//        m_queue.push(request);
        m_queue.push_back(request);
        std::push_heap(m_queue.begin(), m_queue.end(), ProcessCompare<ChunkType>());
    }
    m_event.notify_all();
}

template<typename _Grid>
std::unique_lock<std::mutex> ProcessQueue<_Grid>::getLock()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    return lock;
}

template<typename _Grid>
void ProcessQueue<_Grid>::wait(std::unique_lock<std::mutex> &lock)
{
    m_event.wait(lock);
}

template<typename _Grid>
typename ProcessQueue<_Grid>::SharedChunkHandle ProcessQueue<_Grid>::getNextProcessRequest(std::unique_lock<std::mutex> &lock, Process::Type &type, size_t &data)
{
    SharedProcessRequest<ChunkType> request;
    SharedChunkHandle chunkHandle;

    if(!m_queue.empty())
    {
//        request=m_queue.top();
//        m_queue.pop_back();
        std::pop_heap(m_queue.begin(), m_queue.end(), ProcessCompare<ChunkType>());
        request=m_queue.back();
        m_queue.pop_back();

        type=request->type;
        switch(request->type)
        {
        case Process::Type::Generate:
            {
                GenerateRequest<ChunkType> *generateRequest=(GenerateRequest<ChunkType> *)request.get();

                chunkHandle=generateRequest->chunkHandle.lock();
                data=generateRequest->lod;
            }
            break;
        case Process::Type::Read:
            {
                ReadRequest<ChunkType> *readRequest=(ReadRequest<ChunkType> *)request.get();

                chunkHandle=readRequest->chunkHandle.lock();
                data=readRequest->lod;
            }
            break;
        case Process::Type::Write:
            {
                WriteRequest<ChunkType> *writeRequest=(WriteRequest<ChunkType> *)request.get();

                chunkHandle=writeRequest->chunkHandle;
            }
            break;
        case Process::Type::Update:
            {
                UpdateRequestType *updateRequest=(UpdateRequestType *)request.get();

                chunkHandle=updateRequest->chunkHandle.lock();
            }
            break;
        }
    }

#ifdef LOG_PROCESS_QUEUE
    if(chunkHandle)
    {
        glm::ivec3 regionIndex=chunkHandle->regionIndex();
        glm::ivec3 chunkIndex=chunkHandle->chunkIndex();

        LOG(INFO)<<"Processing chunk ("<<regionIndex.x<<", "<<regionIndex.y<<", "<<regionIndex.z<<"  "<<chunkIndex.x<<", "<<chunkIndex.y<<", "<<chunkIndex.z<<") distance "<<request->distance;
    }
#endif

    return chunkHandle;
}

template<typename _Grid>
void ProcessQueue<_Grid>::updatePriority()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    std::make_heap(m_queue.begin(), m_queue.end(), ProcessCompare<ChunkType>());
}

}//namespace voxigen

