#ifndef _voxigen_updateQueue_h_
#define _voxigen_updateQueue_h_

#include "voxigen/defines.h"

#include <memory>
#include <thread>
#include <queue>
#include <mutex>

namespace voxigen
{

class UpdateQueue
{
public:
    void add(Key hash)
    {
        std::unique_lock<std::mutex> lock(m_chunkUpdatedMutex);

        m_chunksUpdated.push_back(hash);
    }

    std::vector<Key> get()
    {
        std::unique_lock<std::mutex> lock(m_chunkUpdatedMutex);

        std::vector<Key> updatedChunks(m_chunksUpdated);
        m_chunksUpdated.clear();
        lock.unlock();

        return updatedChunks;
    }

private:
    //Status updates
    std::mutex m_chunkUpdatedMutex;
    std::vector<Key> m_chunksUpdated;
};



}//namespace voxigen

#endif //_voxigen_updateQueue_h_