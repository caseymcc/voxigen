#ifndef _voxigen_generator_h_
#define _voxigen_generator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/classFactory.h"

#include "voxigen/chunkHandle.h"

#include <memory>
#include <thread>
#include <queue>
#include <mutex>

namespace voxigen
{

class Generator
{
public:
    Generator() {}
    virtual ~Generator() {}

    virtual void initialize(GridDescriptors *descriptors)=0;
//    virtual void terminate()=0;

//    virtual void generateChunk(unsigned int hash, void *buffer, size_t size)=0;
    virtual unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize)=0;
};
typedef std::shared_ptr<Generator> SharedGenerator;

template<typename _Generator>
class GeneratorTemplate:public RegisterClass<GeneratorTemplate<_Generator>, Generator>
{
public:
    GeneratorTemplate():m_generator(new _Generator()) {}
    virtual ~GeneratorTemplate() {}

    static char *typeName() { return _Generator::typeName(); }

    virtual void initialize(GridDescriptors *descriptors) { m_generator->initialize(descriptors); }
//    virtual void terminate() { m_generator->terminate(); }

//    virtual void generateChunk(unsigned int hash, void *buffer, size_t size) { m_generator->generateChunk(hash, buffer, size); };
    virtual unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize) { return m_generator->generateChunk(startPos, chunkSize, buffer, bufferSize); };

private:
    std::unique_ptr<_Generator> m_generator;
};

template<typename _Chunk>
class GeneratorQueue
{
public:
    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    GeneratorQueue(GridDescriptors *descriptors):m_descriptors(descriptors), m_generator(nullptr){}

    void setGenerator(Generator *generator) { m_generator=generator; }

    void initialize();
    void terminate();

    void add(SharedChunkHandle chunkHandle);

    void addUpdated(SegmentChunkHash hash);
    std::vector<SegmentChunkHash> getUpdated();

    void generatorThread();

private:
    Generator *m_generator;
    GridDescriptors *m_descriptors;

    //generator thread/queue
    std::mutex m_generatorMutex;
    std::thread m_generatorThread;
    std::queue<SharedChunkHandle> m_generatorQueue;
    std::condition_variable m_generatorEvent;
    bool m_generatorThreadRun;

    //Status updates
    std::mutex m_chunkUpdatedMutex;
    std::vector<SegmentChunkHash> m_chunksUpdated;
};

template<typename _Chunk>
void GeneratorQueue<_Chunk>::initialize()
{
    m_generatorThreadRun=true;
    m_generatorThread=std::thread(std::bind(&GeneratorQueue<_Chunk>::generatorThread, this));
}

template<typename _Chunk>
void GeneratorQueue<_Chunk>::terminate()
{
    //thread flags are not atomic so we need the mutexes to coordinate the setting, 
    //otherwise would have to loop re-notifiying thread until it stopped
    {
        std::unique_lock<std::mutex> lock(m_generatorMutex);
        m_generatorThreadRun=false;
    }

    m_generatorEvent.notify_all();
    m_generatorThread.join();

}

template<typename _Chunk>
void GeneratorQueue<_Chunk>::generatorThread()
{
    std::unique_lock<std::mutex> lock(m_generatorMutex);

    while(m_generatorThreadRun)
    {
        if(m_generatorQueue.empty())
        {
            m_generatorEvent.wait(lock);
            continue;
        }

        SharedChunkHandle chunkHandle=m_generatorQueue.front();

        if(!chunkHandle)
            return;

        m_generatorQueue.pop();

        //no generator
        if(m_generator==nullptr)
            continue;

        lock.unlock();//drop lock while working

        glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHandle->hash);
        glm::vec3 startPos=m_descriptors->segmentOffset(chunkHandle->segmentHash);
        glm::vec3 chunkOffset=m_descriptors->chunkOffset(chunkHandle->hash);

        startPos+=chunkOffset;
        chunkHandle->chunk=std::make_unique<ChunkType>(chunkHandle->hash, 0, chunkIndex, startPos);

        ChunkType::Cells &cells=chunkHandle->chunk->getCells();

        unsigned int validCells=m_generator->generateChunk(startPos, glm::ivec3(_Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value), cells.data(), cells.size()*sizeof(ChunkType::CellType));

        chunkHandle->chunk->setValidCellCount(validCells);
        chunkHandle->status=ChunkHandleType::Memory;

        if(validCells<=0)
            chunkHandle->empty=true;
        else
            chunkHandle->empty=false;

        addUpdated(SegmentChunkHash(chunkHandle->segmentHash, chunkHandle->hash));
        chunkHandle.reset();//release pointer while not holding lock as there is a chance this will call removeHandle
                            //which will lock m_chunkMutex and safer to only have one lock at a time
        lock.lock();
    }
}

template<typename _Chunk>
void GeneratorQueue<_Chunk>::add(SharedChunkHandle chunkHandle)
{
    std::unique_lock<std::mutex> lock(m_generatorMutex);

    chunkHandle->status=ChunkHandleType::Generating;
    m_generatorQueue.push(chunkHandle);
    m_generatorEvent.notify_all();
}

template<typename _Chunk>
void GeneratorQueue<_Chunk>::addUpdated(SegmentChunkHash hash)
{
    std::unique_lock<std::mutex> lock(m_chunkUpdatedMutex);

    m_chunksUpdated.push_back(hash);
}

template<typename _Chunk>
std::vector<SegmentChunkHash> GeneratorQueue<_Chunk>::getUpdated()
{
    std::unique_lock<std::mutex> lock(m_chunkUpdatedMutex);

    std::vector<SegmentChunkHash> updatedChunks(m_chunksUpdated);
    m_chunksUpdated.clear();
    lock.unlock();

    return updatedChunks;
}

}//namespace voxigen

#endif //_voxigen_generator_h_