#include <fstream>

namespace voxigen
{

template<typename _Chunk>
void ChunkHandle<_Chunk>::generate(IGridDescriptors *descriptors, Generator *generator, size_t lod)
{
    glm::ivec3 chunkIndex=descriptors->getChunkIndex(m_hash);
    glm::vec3 startPos=descriptors->getRegionOffset(m_regionHash);
    glm::vec3 chunkOffset=descriptors->getChunkOffset(m_hash);

    startPos+=chunkOffset;

#ifdef DEBUG_ALLOCATION
    allocated++;
    Log::debug("ChunkHandle (%d, %d) allocating by generate\n", m_regionHash, m_hash);
#endif
    m_chunk=std::make_unique<ChunkType>(m_hash, 0, chunkIndex, chunkOffset, lod);

    if(!m_chunk)
        return;

    typename ChunkType::Cells &cells=m_chunk->getCells();

    unsigned int validCells=generator->generateChunk(startPos, glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value), cells.data(), cells.size()*sizeof(typename ChunkType::CellType), lod);

    m_chunk->setValidCellCount(validCells);
//    setState(HandleState::Memory);

    if(validCells<=0)
    {
#ifdef DEBUG_ALLOCATION
        allocated++;
        Log::debug("ChunkHandle (%d, %d) deleting by generate\n", m_regionHash, m_hash);
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
        Log::debug("ChunkHandle (%d, %d) freeing (%d)\n", m_regionHash, m_hash, allocated);
#endif
    }
    m_chunk.reset(nullptr); 
    m_memoryUsed=0;  
//    setState(HandleState::Unknown);
}

template<typename _Chunk>
void ChunkHandle<_Chunk>::read(IGridDescriptors *descriptors, const std::string &fileName, size_t lod)
{
    glm::ivec3 chunkIndex=descriptors->getChunkIndex(m_hash);
    glm::vec3 offset=glm::vec3(glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex);

#ifdef DEBUG_ALLOCATION
    allocated++;
    Log::debug("ChunkHandle (%d, %d) allocating by read\n", m_regionHash, m_hash);
#endif
    m_chunk=std::make_unique<ChunkType>(m_hash, 0, chunkIndex, offset, lod);

    auto &cells=m_chunk->getCells();
    std::ifstream file;

    file.open(fileName, std::ofstream::in|std::ofstream::binary);
    file.read((char *)cells.data(), cells.size()*sizeof(typename ChunkType::CellType));
    file.close();

    m_memoryUsed=cells.size()*sizeof(typename ChunkType::CellType);
//    setState(HandleState::Memory);
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

