#ifndef _voxigen_chunkCache_h_
#define _voxigen_chunkCache_h_

namespace voxigen
{

//template<typename _Chunk>
//struct IORequest
//{
//    enum Type
//    {
//        Read,
//        Write
//    };
//
//
//};

template<typename _Chunk>
class ChunkCache()
{
public:
    typedef _Chunk ChunkType;

    ChunkCache();

    void load(std::string directory);

    void initialize();
    void terminate();

    void updateCache(ChunkType *chunk);

private:

//io thread
    std::thread m_ioThread;
    std::mutex m_ioMutex;
    std::queue<SharedChunkHandle> m_ioQueue;
    std::condition_variable m_ioEvent;
    bool m_ioThreadRun;
};

template<typename _Chunk>
ChunkCache<_Chunk>::ChunkCache()
{

}

template<typename _Chunk>
void ChunkCache<_Chunk>::load(std::string directory)
{

}


template<typename _Chunk>
void ChunkCache<_Chunk>::initialize()
{}


template<typename _Chunk>
void ChunkCache<_Chunk>::terminate()
{}

template<typename _Chunk>
void ChunkCache<_Chunk>::updateCache(ChunkType *chunk)
{

}

} //namespace voxigen

#endif //_voxigen_chunkCache_h_