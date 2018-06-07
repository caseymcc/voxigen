#ifndef _voxigen_chunkHandle_h_
#define _voxigen_chunkHandle_h_

#include "voxigen/chunk.h"
#include <memory>
#include <glog/logging.h>

namespace voxigen
{

class Generator;

#ifdef DEBUG_ALLOCATION
std::atomic<int> allocated=0;
#endif

template<typename _Chunk>
class ChunkHandle
{
public:
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    enum Status
    {
        Unknown,
        Cached,
        Loading,
        Generating,
        Memory
    };

    ChunkHandle(RegionHash regionHash, const glm::ivec3 &regionIndex, ChunkHash chunkHash, const glm::ivec3 &chunkIndex):m_regionHash(regionHash), m_regionIndex(regionIndex), m_hash(chunkHash), m_chunkIndex(chunkIndex), m_status(Unknown), m_cachedOnDisk(false), m_empty(false), m_memoryUsed(0){}

    void generate(IGridDescriptors *descriptors, Generator *generator);
    void read(IGridDescriptors *descriptors, const std::string &fileName);
    void write(IGridDescriptors *descriptors, const std::string &fileName);

    void release();
    glm::ivec3 size() { return glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value); }

    Status status() { return m_status; }
    void setStatus(Status status) { m_status=status; }
    bool cachedOnDisk() { return m_cachedOnDisk; }
    void setCachedOnDisk(bool cached) { m_cachedOnDisk=cached; }
    bool empty() { return m_empty; }
    void setEmpty(bool empty=true) { m_empty=empty; if(empty) { m_memoryUsed=0; m_status=Memory; } }

    ChunkHash hash() { return m_hash; }
    glm::ivec3 chunkIndex() { return m_chunkIndex; }

    RegionHash regionHash() { return m_regionHash; }
    glm::ivec3 regionIndex() { return m_regionIndex; }
    glm::ivec3 regionOffset() { return m_regionOffset; }
    void setRegionOffset(const glm::ivec3 &offset) { m_regionOffset=offset; }

    ChunkType *chunk() { return m_chunk.get(); }

    size_t memoryUsed() { return m_memoryUsed; }

private:
    Status m_status;
    RegionHash m_regionHash;
    glm::ivec3 m_regionIndex;
    ChunkHash m_hash;
    glm::ivec3 m_chunkIndex;
    UniqueChunk m_chunk;

    glm::ivec3 m_regionOffset;

    size_t m_memoryUsed;
    bool m_cachedOnDisk;
    bool m_empty;
};

template<typename _Chunk>
void ChunkHandle<_Chunk>::generate(IGridDescriptors *descriptors, Generator *generator)
{
    glm::ivec3 chunkIndex=descriptors->getChunkIndex(m_hash);
    glm::vec3 startPos=descriptors->getRegionOffset(m_regionHash);
    glm::vec3 chunkOffset=descriptors->getChunkOffset(m_hash);

    startPos+=chunkOffset;
    m_chunk=std::make_unique<ChunkType>(m_hash, 0, chunkIndex, chunkOffset);

    if(!m_chunk)
        return;

    ChunkType::Cells &cells=m_chunk->getCells();

    unsigned int validCells=generator->generateChunk(startPos, glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value), cells.data(), cells.size()*sizeof(ChunkType::CellType));

    m_chunk->setValidCellCount(validCells);
    m_status=Memory;

    if(validCells<=0)
    {
        m_chunk.reset(nullptr);//drop chunks we are empty
        setEmpty(true);
    }
    else
    {
#ifdef DEBUG_ALLOCATION
        allocated++;
        LOG(INFO)<<"ChunkHandle ("<<m_regionHash<<" ,"<<m_hash<<") allocated by generate\n";
#endif

        m_memoryUsed=cells.size()*sizeof(ChunkType::CellType);
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
    m_status=Unknown;
}

template<typename _Chunk>
void ChunkHandle<_Chunk>::read(IGridDescriptors *descriptors, const std::string &fileName)
{
    glm::ivec3 chunkIndex=descriptors->getChunkIndex(m_hash);
    glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;

    m_chunk=std::make_unique<ChunkType>(m_hash, 0, chunkIndex, offset);

    auto &cells=m_chunk->getCells();
    std::ifstream file;

    file.open(fileName, std::ofstream::in|std::ofstream::binary);
    file.read((char *)cells.data(), cells.size()*sizeof(typename ChunkType::CellType));
    file.close();

#ifdef DEBUG_ALLOCATION
    allocated++;
    LOG(INFO)<<"ChunkHandle ("<<m_regionHash<<" ,"<<m_hash<<") allocated by read\n";
#endif

    m_memoryUsed=cells.size()*sizeof(typename ChunkType::CellType);
    m_status=Memory;
}

template<typename _Chunk>
void ChunkHandle<_Chunk>::write(IGridDescriptors *descriptors, const std::string &fileName)
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