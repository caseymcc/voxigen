#ifndef _voxigen_regularGrid_h_
#define _voxigen_regularGrid_h_

//#include "voxigen/biome.h"
#include "voxigen/chunk.h"
#include "voxigen/region.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/dataStore.h"
#include "voxigen/entity.h"
#include "voxigen/classFactory.h"
#include "voxigen/updateQueue.h"
#include "voxigen/processQueue.h"

//#include "octomap/OcTree.h"

#include <unordered_map>
#include <memory>
#include <limits>
#include <thread>

namespace voxigen
{

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ>
using UniqueChunkMap=std::unordered_map<ChunkHash, UniqueChunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ>>;

//inheritable class the carries typedef
template<typename _Grid, bool grid=false>
class RegularGridTypes
{
public:
    typedef _Grid GridType;
    typedef GridDescriptors<GridType> DescriptorType;

    typedef typename GridType::ChunkType ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
};

template<typename _Cell, size_t _ChunkSizeX=64, size_t _ChunkSizeY=64, size_t _ChunkSizeZ=64, size_t _RegionSizeX=16, size_t _RegionSizeY=16, size_t _RegionSizeZ=16, bool _Thread=true>
class RegularGrid
{
public:
    RegularGrid();
    ~RegularGrid();

    typedef RegularGrid< _Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ> GridType;
    typedef std::integral_constant<size_t, _ChunkSizeX*_RegionSizeX> regionCellSizeX;
    typedef std::integral_constant<size_t, _ChunkSizeY*_RegionSizeY> regionCellSizeY;
    typedef std::integral_constant<size_t, _ChunkSizeZ*_RegionSizeZ> regionCellSizeZ;

    typedef GridDescriptors<GridType> DescriptorType;
    typedef _Cell CellType;

    typedef Chunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef Region<ChunkType, _RegionSizeX, _RegionSizeY, _RegionSizeZ> RegionType;
    typedef RegionHandle<RegionType> RegionHandleType;
    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;

    typedef ProcessQueue<GridType> ProcessQueueType;
    typedef std::function<bool(SharedChunkHandle)> ChunkUpdateCallback;
//    typedef std::unordered_map<RegionHash, SharedRegion> SharedRegionMap;

//    typedef UniqueChunkMap<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> UniqueChunkMap;
//    typedef std::shared_ptr<ChunkType> SharedChunk;
//    typedef std::unordered_map<ChunkHash, SharedChunk> SharedChunkMap;

////registers default generators
//    typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<ChunkType>> EquiRectWorldGenerator;

    void create(const std::string &directory, const std::string &name, const glm::ivec3 &size, const std::string &generatorName);
    bool load(const std::string &directory);
    bool save();
    bool saveTo(const std::string &directory);
    
    void setChunkUpdateCallback(ChunkUpdateCallback callback) {chunkUpdateCallback=callback; }
    bool defaultChunkUpdateCallback() { return false; }

    SharedRegionHandle getRegion(const glm::ivec3 &index);
    SharedRegionHandle getRegion(RegionHash hash);
    void loadRegion(SharedRegionHandle handle, size_t lod, bool force=false);
    void cancelLoadRegion(SharedRegionHandle handle);
//    RegionHash getRegionHash(const glm::vec3 &gridPosition);

    void updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk);

    glm::ivec3 size();
    glm::ivec3 regionCellSize();

    SharedChunkHandle getChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);
    SharedChunkHandle getChunk(RegionHash regionHash, ChunkHash chunkHash);
    SharedChunkHandle getChunk(Key &key);
    void loadChunk(SharedChunkHandle chunkHandle, size_t lod, bool force=false);
    void cancelLoadChunk(SharedChunkHandle chunkHandle);
    void releaseChunk(SharedChunkHandle chunkHandle);
    void getUpdated(std::vector<RegionHash> &updatedRegions, std::vector<Key> &updatedChunks);

    RegionHash getRegionHash(const glm::ivec3 &index);
    glm::ivec3 getRegionIndex(const glm::vec3 &position);
    glm::ivec3 getRegionIndex(RegionHash hash);

    glm::ivec3 getChunkIndex(const glm::vec3 &position);
    glm::ivec3 getChunkIndex(ChunkHash hash);
    ChunkHash getChunkHash(const glm::ivec3 &chunkIndex)const;
    ChunkHash getChunkHash(RegionHash regionHash, const glm::vec3 &gridPosition);
    ChunkHash getChunkHashFromRegionPos(const glm::vec3 &regionPosition);
    ChunkHash getChunkHash(const glm::vec3 &gridPosition)const;

    Key getHashes(const glm::vec3 &gridPosition);
    Key getHashes(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);

#ifdef USE_OCTOMAP
    octomap::OcTree<SharedRegionHandle> m_regionTree;
#endif //USE_OCTOMAP

//Sizes
    glm::ivec3 getChunkSize();

    glm::vec3 gridPosToRegionPos(RegionHash regionHash, const glm::vec3 &gridPosition);

    DescriptorType &getDescriptors() { return m_descriptors; }
    const DescriptorType &getDescriptors() const { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

    void updateProcessQueue() { m_processQueue.updateQueue(); }
    void processThread();

    void processGenerateRegion(ProcessRequest *request);
    void processGenerate(ProcessRequest *request);
    void processRead(ProcessRequest *request);
    void processWrite(ProcessRequest *request);
    void processUpdate(ProcessRequest *request);
    void processRelease(ProcessRequest *request);

    bool alignPosition(glm::ivec3 &regionIndex, glm::vec3 &position);

private:
    void loadRegions(std::string directory);

    void handleGenerateRegionComplete(ProcessRequest *request, std::vector<RegionHash> &updated);
    void handleGenerateComplete(ProcessRequest *request, std::vector<Key> &updatedChunks);
    void handleReadComplete(ProcessRequest *request, std::vector<Key> &updatedChunks);
    void handleUpdateComplete(ProcessRequest *request, std::vector<Key> &updatedChunks);
    void handleReleaseComplete(ProcessRequest *request);

    std::string m_directory;
    std::string m_name;

    DescriptorType m_descriptors;
//    ChunkHandler<ChunkType> m_chunkHandler;
    ChunkUpdateCallback chunkUpdateCallback;

    GeneratorQueue<GridType> m_generatorQueue;
    SharedGenerator m_generator;
    DataStore<GridType> m_dataStore;
    UpdateQueue m_updateQueue;

    glm::mat4 m_transform;

    std::thread m_processThread;
    bool m_processThreadRunning;
    std::mutex m_processMutex;
    std::condition_variable m_processEvent;
    ProcessQueueType m_processQueue;
    std::vector<SharedProcessRequest> m_completeQueue;
};

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::RegularGrid():
m_dataStore(&m_descriptors, &m_processQueue, &m_generatorQueue, &m_updateQueue),
m_generatorQueue(&m_descriptors, &m_updateQueue),
m_processQueue(&m_descriptors)
//m_chunkHandler(&m_descriptors)
{
    chunkUpdateCallback=std::bind(&RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::defaultChunkUpdateCallback, this);

    if(_Thread)
    {
        m_processThreadRunning=true;
        m_processThread=std::thread(std::bind(&RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processThread, this));
    }
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::~RegularGrid()
{
    m_dataStore.terminate();
    m_generatorQueue.terminate();

    if(_Thread)
    {
        //shutdown processing thread
        {
            std::unique_lock<std::mutex> lock(m_processMutex);

            m_processThreadRunning=false;
            m_processQueue.trigger(lock);//force update
        }
        m_processThread.join();
    }
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::create(const std::string &directory, const std::string &name, const glm::ivec3 &size, const std::string &generatorName)
{
    m_name=name;
    m_directory=directory;

    m_descriptors.create(name, 0, size);// , glm::ivec3(_RegionSizeX, _RegionSizeY, _RegionSizeZ), glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ));
    m_descriptors.init();
    m_descriptors.m_generator=generatorName;

    m_generator=createClass<Generator>(generatorName);
    m_generator->save(&m_descriptors);
    m_generatorQueue.setGenerator(m_generator.get());

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile);

    std::string regionDirectory=directory+"/regions";
    //    fs::path regionPath(regionDirectory);
    std::string regionPath(regionDirectory);

    fs::create_directory(regionPath.c_str());

    m_generator->initialize(&m_descriptors);
    m_dataStore.initialize();
    m_generatorQueue.initialize();
}

//Default processing thread, can be turned off with _Thread template variable
template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processThread()
{
    std::unique_lock<std::mutex> lock(m_processMutex);
//    std::unique_lock<std::mutex> &lock=m_processQueue.getLock();

    while(m_processThreadRunning)
    {
        Process::Type type;
        size_t lod;
//        SharedChunkHandle chunkHandle=m_processQueue.getNextProcessRequest(lock, type, lod);
        SharedChunkHandle chunkHandle;

        SharedProcessRequest sharedRequest=m_processQueue.getNextProcessRequest();
        
        if(!sharedRequest)
        {
//            m_processEvent.wait(lock);
            if(m_processQueue.empty())
            {
                //check for more items, this has a lock
                m_processQueue.updatePriorityQueue();
                
                if(m_processQueue.empty())
                    m_processQueue.wait(lock);
            }
            continue;
        }

//        lock.unlock();

//        bool addChunk=false;
        ProcessRequest *request=sharedRequest.get();

        switch(request->type)
        {
        case Process::GenerateRegion:
            processGenerateRegion(request);
            break;
        case Process::Generate:
            processGenerate(request);
            break;
        case Process::Read:
            processRead(request);
            break;
        case Process::Write:
            processWrite(request);
            break;
        case Process::Update:
            processUpdate(request);
            break;
        case Process::Release:
            processRelease(request);
            break;
        }

        m_processQueue.addCompletedRequest(sharedRequest);
        if(m_processQueue.isCompletedQueueEmpty())
        {
            m_processQueue.updateCompletedQueue();
        }
    }
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processGenerateRegion(ProcessRequest *request)
{
    typedef GenerateRegionRequest<RegionType> GenerateRegionRequest;

    GenerateRegionRequest *generateRequest=dynamic_cast<GenerateRegionRequest *>(request);
    SharedRegionHandle regionHandle=generateRequest->handle.lock();

    if(!regionHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"ProcessThread - Region ("<<regionHandle->regionHash()<<", "<<regionHandle->hash()<<") generate";
#endif//LOG_PROCESS_QUEUE
    regionHandle->generate(&m_descriptors, m_generator.get(), generateRequest->lod);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processGenerate(ProcessRequest *request)
{
    typedef GenerateRequest<RegionType, ChunkType> GenerateRequest;

    GenerateRequest *generateRequest=dynamic_cast<GenerateRequest *>(request);
    SharedChunkHandle chunkHandle=generateRequest->chunkHandle.lock();

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"ProcessThread - Chunk ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") generate";
#endif//LOG_PROCESS_QUEUE
    chunkHandle->generate(&m_descriptors, m_generator.get(), generateRequest->lod);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processRead(ProcessRequest *request)
{
    typedef ReadRequest<RegionType, ChunkType> ReadRequest;

    ReadRequest *readRequest=dynamic_cast<ReadRequest *>(request);
    SharedChunkHandle chunkHandle=readRequest->chunkHandle.lock();

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"ProcessThread - Chunk ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") read";
#endif//LOG_PROCESS_QUEUE
    m_dataStore.readChunk(chunkHandle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processWrite(ProcessRequest *request)
{
    typedef WriteRequest<RegionType, ChunkType> WriteRequest;

    WriteRequest *writeRequest=dynamic_cast<WriteRequest *>(request);
    SharedChunkHandle chunkHandle=writeRequest->chunkHandle;

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"ProcessThread - Chunk ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") write";
#endif//LOG_PROCESS_QUEUE
    m_dataStore.writeChunk(chunkHandle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processUpdate(ProcessRequest *request)
{
    typedef UpdateRequest<RegionType, ChunkType> UpdateRequest;

    UpdateRequest *updateRequest=dynamic_cast<UpdateRequest *>(request);
    SharedChunkHandle chunkHandle=updateRequest->chunkHandle.lock();

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"ProcessThread - Chunk ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") update";
#endif//LOG_PROCESS_QUEUE
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processRelease(ProcessRequest *request)
{
    typedef ReleaseRequest<RegionType, ChunkType> ReleaseRequest;

    ReleaseRequest *releaseRequest=dynamic_cast<ReleaseRequest *>(request);
    SharedChunkHandle chunkHandle=releaseRequest->chunkHandle.lock();

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"ProcessThread - Chunk ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") release";
#endif//LOG_PROCESS_QUEUE
    chunkHandle->release();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::load(const std::string &directory)
{
    m_directory=directory;

    std::string configFile=directory+"/gridConfig.json";
    
    if(!m_descriptors.load(configFile))
        return false;

    m_generator=createClass<Generator>(m_descriptors.m_generator);
    m_generatorQueue.setGenerator(m_generator.get());

    std::string regionDirectory=directory+"/regions";
    m_dataStore.load(regionDirectory);

    m_generator->initialize(&m_descriptors);
    m_dataStore.initialize();
    m_generatorQueue.initialize();

    return true;
//    std::string chunkDirectory=directory+"/chunks";
//
//    m_chunkHandler.load(chunkDirectory);
//    m_chunkHandler.initialize();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::save()
{
    std::string configFile=m_directory+"/gridConfig.json";
    m_descriptors.save(configFile);
    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::saveTo(const std::string &directory)
{
    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile);
    return true;
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
//Biome &RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getBiome(glm::ivec3 cell)
//{}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedRegionHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegion(const glm::ivec3 &index)
{
    RegionHash hash=m_descriptors.regionHash(index);

    return m_dataStore.getRegion(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::loadRegion(SharedRegionHandle handle, size_t lod, bool force)
{
    m_dataStore.loadRegion(handle, lod, force);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::cancelLoadRegion(SharedRegionHandle handle)
{
    m_dataStore.cancelLoadRegion(handle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedRegionHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegion(RegionHash hash)
{
    return m_dataStore.getRegion(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk)
{
    m_processQueue.updatePosition(region, chunk);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::size()
{
    return m_descriptors.m_size;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::regionCellSize()
{
    return glm::ivec3(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
//typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(const glm::ivec3 &cell)
//{
//    glm::ivec3 chunkIndex=cell/m_descriptors.m_chunkSize;
//
//    ChunkHash chunkHash=getChunkHash(chunkIndex);
//
//    return m_dataStore.getChunk(chunkHash);
//}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{

    RegionHash regionHash=getRegionHash(regionIndex);
    ChunkHash chunkHash=getChunkHash(chunkIndex);

    return m_dataStore.getChunk(regionHash, chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(Key &key)
{
    return m_dataStore.getChunk(key.regionHash, key.chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(RegionHash regionHash, ChunkHash chunkHash)
{
    return m_dataStore.getChunk(regionHash, chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::loadChunk(SharedChunkHandle chunkHandle, size_t lod, bool force)
{
    m_dataStore.loadChunk(chunkHandle, lod, force);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::cancelLoadChunk(SharedChunkHandle chunkHandle)
{
    m_dataStore.cancelLoadChunk(chunkHandle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::releaseChunk(SharedChunkHandle chunkHandle)
{
    //send to thread to release as it could be processing it as well
    m_processQueue.addRelease(chunkHandle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getUpdated(std::vector<RegionHash> &updatedRegions, std::vector<Key> &updatedChunks)
{
//    std::vector<Key> updatedChunks;
//
//    updatedChunks=m_updateQueue.get();
//    return updatedChunks;

//    if(m_processQueue.isCompletedQueueEmpty())
//        m_processQueue.updateCompletedQueue();

    typename ProcessQueueType::RequestQueue completedQueue;

    m_processQueue.getCompletedQueue(completedQueue);
    for(auto &request:completedQueue)
    {
//        typename ProcessQueueType::SharedChunkHandle chunkHandle=request->getChunkHandle();
//
//        if(!chunkHandle)
//            continue;
        if(request->type==Process::GenerateRegion)
        {
            handleGenerateRegionComplete(request.get(), updatedRegions);
        }
        else if(request->type==Process::Generate)
        {
            handleGenerateComplete(request.get(), updatedChunks);
        }
        else if(request->type==Process::Read)
        {
            handleReadComplete(request.get(), updatedChunks);
        }
        else if(request->type==Process::Update)
        {
            handleUpdateComplete(request.get(), updatedChunks);
        }
        else if(request->type==Process::Release)
        {
            handleReleaseComplete(request.get());
        }
    }
    completedQueue.clear();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleGenerateRegionComplete(ProcessRequest *request, std::vector<RegionHash> &updated)
{
    typedef GenerateRegionRequest<RegionType> GenerateRegionRequest;

    GenerateRegionRequest *generateRequest=dynamic_cast<GenerateRegionRequest *>(request);
    typename ProcessQueueType::SharedRegionHandle handle=generateRequest->getHandle();

    if(!handle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - RegionHandle "<<handle.get()<<" ("<<handle->hash()<<") generate complete";
#endif//LOG_PROCESS_QUEUE
    handle->setState(HandleState::Memory);
    handle->setAction(HandleAction::Idle);

    updated.push_back(handle->hash());
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleGenerateComplete(ProcessRequest *request, std::vector<Key> &updatedChunks)
{
    typedef GenerateRequest<RegionType, ChunkType> GenerateRequest;

    GenerateRequest *generateRequest=dynamic_cast<GenerateRequest *>(request);
    typename ProcessQueueType::SharedChunkHandle chunkHandle=generateRequest->getChunkHandle();
    
    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - ChunkHandle "<<chunkHandle.get()<<" ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") generate complete";
#endif//LOG_PROCESS_QUEUE
    chunkHandle->setState(HandleState::Memory);
    chunkHandle->setAction(HandleAction::Idle);

    updatedChunks.push_back(chunkHandle->key());
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleReadComplete(ProcessRequest *request, std::vector<Key> &updatedChunks)
{
    typedef ReadRequest<RegionType, ChunkType> ReadRequest;

    ReadRequest *readRequest=dynamic_cast<ReadRequest *>(request);
    typename ProcessQueueType::SharedChunkHandle chunkHandle=readRequest->getChunkHandle();

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - ChunkHandle "<<chunkHandle.get()<<" ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") read complete";
#endif//LOG_PROCESS_QUEUE

    chunkHandle->setState(HandleState::Memory);
    chunkHandle->setAction(HandleAction::Idle);
    updatedChunks.push_back(chunkHandle->key());
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleUpdateComplete(ProcessRequest *request, std::vector<Key> &updatedChunks)
{
    typedef UpdateRequest<RegionType, ChunkType> UpdateRequest;

    UpdateRequest *updateRequest=dynamic_cast<UpdateRequest *>(request);
    typename ProcessQueueType::SharedChunkHandle chunkHandle=updateRequest->getChunkHandle();

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - ChunkHandle "<<chunkHandle.get()<<" ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") update complete";
#endif//LOG_PROCESS_QUEUE

    chunkHandle->setState(HandleState::Memory);
    chunkHandle->setAction(HandleAction::Idle);
    updatedChunks.push_back(chunkHandle->key());
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleReleaseComplete(ProcessRequest *request)
{
    typedef ReleaseRequest<RegionType, ChunkType> ReleaseRequest;

    ReleaseRequest *releaseRequest=dynamic_cast<ReleaseRequest *>(request);
    typename ProcessQueueType::SharedChunkHandle chunkHandle=releaseRequest->getChunkHandle();

    if(!chunkHandle)
        return;

#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - ChunkHandle "<<chunkHandle.get()<<" ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") release complete";
#endif//LOG_PROCESS_QUEUE

    chunkHandle->setState(HandleState::Unknown);
    chunkHandle->setAction(HandleAction::Idle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
RegionHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegionHash(const glm::ivec3 &index)
{
    return m_descriptors.regionHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegionIndex(const glm::vec3 &position)
{
    return m_descriptors.regionIndex(position);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegionIndex(RegionHash hash)
{
    return m_descriptors.regionIndex(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkIndex(const glm::vec3 &position)
{
    glm::ivec3 pos=glm::ivec3(glm::floor(position));

    return pos/m_descriptors.m_chunkSize;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkIndex(ChunkHash hash)
{
    return m_descriptors.chunkIndex(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
Key RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getHashes(const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::ivec3(glm::floor(gridPosition));
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=position/regionCellSize;
    glm::ivec3 chunkIndex=(position-(regionIndex*regionCellSize))/m_descriptors.m_chunkSize;

    return Key(m_descriptors.regionHash(regionIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
Key RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getHashes(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    return Key(m_descriptors.regionHash(regionIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHash(RegionHash regionHash, const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=getRegionIndex(regionHash);

    return m_descriptors.chunkHash(position-(regionIndex*regionCellSize));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHashFromRegionPos(const glm::vec3 &regionPosition)
{
    glm::ivec3 position=glm::floor(regionPosition);

    return m_descriptors.chunkHash(position/m_descriptors.m_chunkSize);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHash(const glm::vec3 &gridPosition) const
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=position/regionCellSize;
    glm::ivec3 chunkIndex=position-(regionIndex*regionCellSize);

    return m_descriptors.chunkHash(chunkIndex);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkSize()
{
    return glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::vec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::gridPosToRegionPos(RegionHash regionHash, const glm::vec3 &gridPosition)
{
    glm::vec3 pos=gridPosition-m_descriptors.regionOffset(regionHash);

    return pos;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::alignPosition(glm::ivec3 &regionIndex, glm::vec3 &position)
{
    bool updateRegion=false;
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);

    if(position.x<0)
    {
        regionIndex.x--;
        position.x+=regionCellSize.x;
        updateRegion=true;
    }
    else if(position.x>regionCellSize.x)
    {
        regionIndex.x++;
        position.x-=regionCellSize.x;
        updateRegion=true;
    }

    if(position.y<0)
    {
        regionIndex.y--;
        position.y+=regionCellSize.y;
        updateRegion=true;
    }
    else if(position.y>regionCellSize.y)
    {
        regionIndex.y++;
        position.y-=regionCellSize.y;
        updateRegion=true;
    }

    if(position.z<0)
    {
        regionIndex.z--;
        position.z+=regionCellSize.z;
        updateRegion=true;
    }
    else if(position.z>regionCellSize.z)
    {
        regionIndex.z++;
        position.z-=regionCellSize.z;
        updateRegion=true;
    }

    return updateRegion;
}


}//namespace voxigen

#endif //_voxigen_regularGrid_h_