#ifndef _voxigen_chunkCache_h_
#define _voxigen_chunkCache_h_

namespace voxigen
{

template<typename _Chunk>
class ChunkCache()
{
public:
    typedef _Chunk ChunkType;

    ChunkCache();

    void load(std::string directory);

    void updateCache(ChunkType *chunk);
private:


    
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
void ChunkCache<_Chunk>::updateCache(ChunkType *chunk)
{

}

} //namespace voxigen

#endif //_voxigen_chunkCache_h_