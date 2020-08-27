#ifndef _voxigen_generator_h_
#define _voxigen_generator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/classFactory.h"
#include "voxigen/updateQueue.h"
#include "voxigen/loadProgress.h"

#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace voxigen
{

template<typename _Chunk>
class ChunkHandle;

class Generator
{
public:
    Generator() {}
    virtual ~Generator() {}

    virtual void create(IGridDescriptors *descriptors, LoadProgress &progress)=0;
    virtual bool load(IGridDescriptors *descriptors, const std::string &directoryName, LoadProgress &progress)=0;
    virtual void save(const std::string &directoryName)=0;

    virtual void loadDescriptors(IGridDescriptors *descriptors)=0;
    virtual void saveDescriptors(IGridDescriptors *descriptors)=0;
    //    virtual void terminate()=0;

    //    virtual void generateChunk(unsigned int hash, void *buffer, size_t size)=0;
    virtual unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod)=0;
    virtual unsigned int generateRegion(const glm::vec3 &startPos, const glm::ivec3 &size, void *buffer, size_t bufferSize, size_t lod)=0;

    //used to get the general height at a location, may not be exact
    //exepected limited use
    //pos grid coordinates
    virtual int getBaseHeight(const glm::vec2 &pos)=0;
};

typedef std::shared_ptr<Generator> SharedGenerator;

template<typename _Generator, typename _FileIO>
class GeneratorTemplate:public RegisterClass<GeneratorTemplate<_Generator, _FileIO>, Generator>
{
public:
    GeneratorTemplate():m_generator(new _Generator()) {}
    virtual ~GeneratorTemplate() {}

    static const char *typeName() { return _Generator::typeName(); }

    void create(IGridDescriptors *descriptors, LoadProgress &progress) override { m_generator->create(descriptors, progress); }
    bool load(IGridDescriptors *descriptors, const std::string &directoryName, LoadProgress &progress) override { return m_generator->load<_FileIO>(descriptors, directoryName, progress); }
    void save(const std::string &directoryName) override { m_generator->save<_FileIO>(directoryName); }

    void loadDescriptors(IGridDescriptors *descriptors) override { m_generator->loadDescriptors(descriptors); }
    void saveDescriptors(IGridDescriptors *descriptors) override { m_generator->saveDescriptors(descriptors); }
    //    void terminate() { m_generator->terminate(); }

    //    void generateChunk(unsigned int hash, void *buffer, size_t size) { m_generator->generateChunk(hash, buffer, size); };
    unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod) override { return m_generator->generateChunk(startPos, chunkSize, buffer, bufferSize, lod); };
    unsigned int generateRegion(const glm::vec3 &startPos, const glm::ivec3 &size, void *buffer, size_t bufferSize, size_t lod) override { return m_generator->generateRegion(startPos, size, buffer, bufferSize, lod); };

    int getBaseHeight(const glm::vec2 &pos) override { return m_generator->getBaseHeight(pos); };

    _Generator *get() { return m_generator.get(); }
private:
    std::unique_ptr<_Generator> m_generator;
};

}//namespace voxigen

#include "voxigen/volume/chunkHandle.h"

//namespace voxigen
//{
//
//template<typename _Grid>
//class GeneratorQueue
//{
//public:
//    typedef typename _Grid::ChunkType ChunkType;
//    typedef ChunkHandle<ChunkType> ChunkHandleType;
//    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
//
//    GeneratorQueue(GridDescriptors<_Grid> *descriptors, UpdateQueue *updateQueue):m_descriptors(descriptors), m_generator(nullptr), m_updateQueue(updateQueue){}
//
//    void setGenerator(Generator *generator) { m_generator=generator; }
//
//    void initialize();
//    void terminate();
//
//    void add(SharedChunkHandle chunkHandle);
//
//    void addUpdated(Key hash);
//    std::vector<Key> getUpdated();
//
//    void generatorThread();
//
//private:
//    Generator *m_generator;
//    GridDescriptors<_Grid> *m_descriptors;
//
//    //generator thread/queue
//    std::mutex m_generatorMutex;
//    std::thread m_generatorThread;
//    std::queue<SharedChunkHandle> m_generatorQueue;
//    std::condition_variable m_generatorEvent;
//    bool m_generatorThreadRun;
//
//    //Status updates
//    UpdateQueue *m_updateQueue;
//};
//
//template<typename _Grid>
//void GeneratorQueue<_Grid>::initialize()
//{
//    m_generatorThreadRun=true;
//    m_generatorThread=std::thread(std::bind(&GeneratorQueue<_Grid>::generatorThread, this));
//}
//
//template<typename _Grid>
//void GeneratorQueue<_Grid>::terminate()
//{
//    //thread flags are not atomic so we need the mutexes to coordinate the setting, 
//    //otherwise would have to loop re-notifiying thread until it stopped
//    {
//        std::unique_lock<std::mutex> lock(m_generatorMutex);
//        m_generatorThreadRun=false;
//    }
//
//    m_generatorEvent.notify_all();
//    m_generatorThread.join();
//
//}
//
//template<typename _Grid>
//void GeneratorQueue<_Grid>::generatorThread()
//{
//    std::unique_lock<std::mutex> lock(m_generatorMutex);
//
//    while(m_generatorThreadRun)
//    {
//        if(m_generatorQueue.empty())
//        {
//            m_generatorEvent.wait(lock);
//            continue;
//        }
//
//        SharedChunkHandle chunkHandle=m_generatorQueue.front();
//
//        if(!chunkHandle)
//            return;
//
//        m_generatorQueue.pop();
//
//        //no generator
//        if(m_generator==nullptr)
//            continue;
//
//        lock.unlock();//drop lock while working
//
//        chunkHandle->generate(m_descriptors, m_generator);
////        glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHandle->hash);
////        glm::vec3 startPos=m_descriptors->regionOffset(chunkHandle->regionHash);
////        glm::vec3 chunkOffset=m_descriptors->chunkOffset(chunkHandle->hash);
////
////        startPos+=chunkOffset;
////        chunkHandle->chunk=std::make_unique<ChunkType>(chunkHandle->hash, 0, chunkIndex, chunkOffset);
////
////        ChunkType::Cells &cells=chunkHandle->chunk->getCells();
////
////        unsigned int validCells=m_generator->generateChunk(startPos, glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value), cells.data(), cells.size()*sizeof(ChunkType::CellType));
////
////        chunkHandle->chunk->setValidCellCount(validCells);
////        chunkHandle->status=ChunkHandleType::Memory;
////        chunkHandle->m_memoryUsed=validCells*sizeof(_Grid::CellType)
////
////        if(validCells<=0)
////            chunkHandle->empty=true;
////        else
////            chunkHandle->empty=false;
//
//        m_updateQueue->add(Key(chunkHandle->regionHash(), chunkHandle->hash()));
//        chunkHandle.reset();//release pointer while not holding lock as there is a chance this will call removeHandle
//                            //which will lock m_chunkMutex and safer to only have one lock at a time
//        lock.lock();
//    }
//}
//
//template<typename _Grid>
//void GeneratorQueue<_Grid>::add(SharedChunkHandle chunkHandle)
//{
//    std::unique_lock<std::mutex> lock(m_generatorMutex);
//
////    chunkHandle->setStatus(ChunkHandleType::Generating);
//    m_generatorQueue.push(chunkHandle);
//    m_generatorEvent.notify_all();
//}
//
//}//namespace voxigen

#endif //_voxigen_generator_h_