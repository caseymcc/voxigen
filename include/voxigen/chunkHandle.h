#ifndef _voxigen_chunkHandle_h_
#define _voxigen_chunkHandle_h_

#include "voxigen/chunk.h"
#include "voxigen/handleState.h"
#include <memory>

#ifdef DEBUG_ALLOCATION
#include <glog/logging.h>
#endif//DEBUG_ALLOCATION

#ifndef NDEBUG
#include <thread>
#include <cassert>
#endif

namespace voxigen
{

class Generator;

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
class RegularGrid;

#ifdef DEBUG_ALLOCATION
std::atomic<int> allocated=0;
#endif

template<typename _Chunk>
class ChunkHandle
{
public:
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    ChunkHandle(RegionHash regionHash, const glm::ivec3 &regionIndex, ChunkHash chunkHash, const glm::ivec3 &chunkIndex):
        m_key(regionHash, chunkHash),
        m_regionHash(regionHash), 
        m_regionIndex(regionIndex), 
        m_hash(chunkHash), 
        m_chunkIndex(chunkIndex), 
        m_state(HandleState::Unknown),
        m_action(HandleAction::Idle),
        m_cachedOnDisk(false), 
        m_empty(false), 
#ifndef NDEBUG
        m_stateThreadIdSet(false),
        m_actionThreadIdSet(false),
#endif
        m_memoryUsed(0)
        
    {
        m_regionOffset=chunkIndex*glm::ivec3(_Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value);
    }

    //these functions are likely called from another thread
    void generate(IGridDescriptors *descriptors, Generator *generator, size_t lod=0);
    void read(IGridDescriptors *descriptors, const std::string &fileName, size_t lod=0);
    void write(IGridDescriptors *descriptors, const std::string &fileName, size_t lod=0);

    glm::ivec3 size() { return glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value); }

    HandleState state(){ return m_state; }
    void setState(HandleState state)
    {
#ifndef NDEBUG
        //lets make sure only one thread is changing the state
        if(!m_stateThreadIdSet)
        {
            m_stateThreadId=std::this_thread::get_id();
            m_stateThreadIdSet=true;
        }

        assert(std::this_thread::get_id()==m_stateThreadId);
#endif
        m_state=state;
    }

    HandleAction action() { return m_action; }
    void setAction(HandleAction action)
    {
#ifndef NDEBUG
        //lets make sure only one thread is changing the action
        if(!m_actionThreadIdSet)
        {
            m_actionThreadId=std::this_thread::get_id();
            m_actionThreadIdSet=true;
        }

        assert(std::this_thread::get_id()==m_actionThreadId);
#endif
        m_action=action;
    }

    bool cachedOnDisk() { return m_cachedOnDisk; }
    void setCachedOnDisk(bool cached) { m_cachedOnDisk=cached; }
    bool empty() { return m_empty; }
    void setEmpty(bool empty=true) { m_empty=empty; if(empty) { m_memoryUsed=0; /*setState(HandleState::Memory);*/ } }

    Key &key() { return m_key; }

    ChunkHash hash() { return m_hash; }
    const glm::ivec3 &chunkIndex() { return m_chunkIndex; }

    RegionHash regionHash() { return m_regionHash; }
    const glm::ivec3 &regionIndex() { return m_regionIndex; }
    const glm::ivec3 &regionOffset() { return m_regionOffset; }
    void setRegionOffset(const glm::ivec3 &offset) { m_regionOffset=offset; }

    ChunkType *chunk() { return m_chunk.get(); }

    size_t memoryUsed() { return m_memoryUsed; }

private:
    template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
    friend class RegularGrid;
    //this has to be done in the process thread, need to ask the grid to do this
    void release();
    
/////////////////////////////////////////////////////////
//status and action are to be only updated by one thread
    HandleState m_state;
    HandleAction m_action;
/////////////////////////////////////////////////////////

    Key m_key;
    RegionHash m_regionHash;
    glm::ivec3 m_regionIndex;
    ChunkHash m_hash;
    glm::ivec3 m_chunkIndex;
    UniqueChunk m_chunk;

    glm::ivec3 m_regionOffset;

    size_t m_memoryUsed;
    bool m_cachedOnDisk;
    bool m_empty;

#ifndef NDEBUG
    std::thread::id m_stateThreadId;
    bool m_stateThreadIdSet;

    std::thread::id m_actionThreadId;
    bool m_actionThreadIdSet;
#endif
};

} //namespace voxigen

#include "voxigen/chunkHandle.inl"

#endif //_voxigen_chunkHandler_h_