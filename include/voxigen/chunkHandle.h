#ifndef _voxigen_chunkHandle_h_
#define _voxigen_chunkHandle_h_

#include "voxigen/chunk.h"
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

enum class ChunkState
{
    Unknown,
    Cached,
    Memory
};

enum class ChunkAction
{
    Idle,
    Reading,
    Writing,
    Generating,
    Updating,
    Releasing
};

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
        m_state(ChunkState::Unknown),
        m_action(ChunkAction::Idle),
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

    ChunkState state(){ return m_state; }
    void setState(ChunkState state)
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
//    void setUnknown() { setState(Unknown); }
//    bool isUnknown() { return state()==Unknown; }
//    void setCached() { setState(Cached); }
//    bool isCached() { return state()==Cached; }
//    void setMemory() { setState(Memory); }
//    bool isMemory() { return state()==Memory; }

    ChunkAction action() { return m_action; }
    void setAction(ChunkAction action)
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

//    void setIdle() { setAction(ChunkAction::Idle); }
//    bool isIdle() { return action()==ChunkAction::Idle; }
//    void setReading() { setAction(ChunkAction::Reading); }
//    bool isReading() { return action()==ChunkAction::Reading; }
//    void setWriting() { setAction(ChunkAction::Writing); }
//    bool isWriting() { return action()==ChunkAction::Writing; }
//    void setGenerating() { setAction(ChunkAction::Generating); }
//    bool isGenerating() { return action()==ChunkAction::Generating; }
//    void setUpdating() { setAction(ChunkAction::Updating); }
//    bool isUpdating() { return action()==ChunkAction::Updating; }
//    void setReleasing() { setAction(ChunkAction::Releasing); }
//    bool isReleasing() { return action()==ChunkAction::Releasing; }

    bool cachedOnDisk() { return m_cachedOnDisk; }
    void setCachedOnDisk(bool cached) { m_cachedOnDisk=cached; }
    bool empty() { return m_empty; }
    void setEmpty(bool empty=true) { m_empty=empty; if(empty) { m_memoryUsed=0; /*setState(ChunkState::Memory);*/ } }

    Key &key() { return m_key; }

    ChunkHash hash() { return m_hash; }
    glm::ivec3 chunkIndex() { return m_chunkIndex; }

    RegionHash regionHash() { return m_regionHash; }
    glm::ivec3 regionIndex() { return m_regionIndex; }
    glm::ivec3 regionOffset() { return m_regionOffset; }
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
    ChunkState m_state;
    ChunkAction m_action;
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

template<typename _Chunk>
void ChunkHandle<_Chunk>::generate(IGridDescriptors *descriptors, Generator *generator, size_t lod)
{
    glm::ivec3 chunkIndex=descriptors->getChunkIndex(m_hash);
    glm::vec3 startPos=descriptors->getRegionOffset(m_regionHash);
    glm::vec3 chunkOffset=descriptors->getChunkOffset(m_hash);

    startPos+=chunkOffset;

#ifdef DEBUG_ALLOCATION
    allocated++;
    LOG(INFO)<<"ChunkHandle ("<<m_regionHash<<" ,"<<m_hash<<") allocating by generate\n";
#endif
    m_chunk=std::make_unique<ChunkType>(m_hash, 0, chunkIndex, chunkOffset, lod);

    if(!m_chunk)
        return;

    typename ChunkType::Cells &cells=m_chunk->getCells();

    unsigned int validCells=generator->generateChunk(startPos, glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value), cells.data(), cells.size()*sizeof(typename ChunkType::CellType), lod);

    m_chunk->setValidCellCount(validCells);
//    setState(ChunkState::Memory);

    if(validCells<=0)
    {
#ifdef DEBUG_ALLOCATION
        allocated++;
        LOG(INFO)<<"ChunkHandle ("<<m_regionHash<<" ,"<<m_hash<<") deleting by generate\n";
#endif
        m_chunk.reset(nullptr);//drop chunks that are empty
        setEmpty(true);
    }
    else
    {
        m_memoryUsed=cells.size()*sizeof(typename ChunkType::CellType);
        setEmpty(false);
    }
}

template<typename _Chunk>
void ChunkHandle<_Chunk>::release()
{
    if(m_chunk)
    {
#ifdef DEBUG_ALLOCATION
        allocated--;
        LOG(INFO)<<"ChunkHandle ("<<m_regionHash<<" ,"<<m_hash<<") freeing ("<<allocated<<")\n";
#endif
    }
    m_chunk.reset(nullptr); 
    m_memoryUsed=0;  
//    setState(ChunkState::Unknown);
}

template<typename _Chunk>
void ChunkHandle<_Chunk>::read(IGridDescriptors *descriptors, const std::string &fileName, size_t lod)
{
    glm::ivec3 chunkIndex=descriptors->getChunkIndex(m_hash);
    glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;

#ifdef DEBUG_ALLOCATION
    allocated++;
    LOG(INFO)<<"ChunkHandle ("<<m_regionHash<<" ,"<<m_hash<<") allocating by read\n";
#endif
    m_chunk=std::make_unique<ChunkType>(m_hash, 0, chunkIndex, offset, lod);

    auto &cells=m_chunk->getCells();
    std::ifstream file;

    file.open(fileName, std::ofstream::in|std::ofstream::binary);
    file.read((char *)cells.data(), cells.size()*sizeof(typename ChunkType::CellType));
    file.close();

    m_memoryUsed=cells.size()*sizeof(typename ChunkType::CellType);
//    setState(ChunkState::Memory);
}

template<typename _Chunk>
void ChunkHandle<_Chunk>::write(IGridDescriptors *descriptors, const std::string &fileName, size_t lod)
{
    auto &cells=m_chunk->getCells();
    std::ofstream file;

    file.open(fileName, std::ofstream::out|std::ofstream::trunc|std::ofstream::binary);
    file.write((char *)cells.data(), cells.size()*sizeof(typename ChunkType::CellType));
    file.close();

    m_cachedOnDisk=true;
}

} //namespace voxigen

#endif //_voxigen_chunkHandler_h_