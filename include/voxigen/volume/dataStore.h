#ifndef _voxigen_dataStore_h_
#define _voxigen_dataStore_h_

#include "voxigen/volume/chunkHandle.h"
#include "voxigen/volume/regionHandle.h"
#include "voxigen/volume/gridDescriptors.h"
#include "voxigen/generators/generator.h"
#include "voxigen/fileio/jsonSerializer.h"
#include "voxigen/fileio/simpleFilesystem.h"
//#include "voxigen/processQueue.h"
#include "voxigen/fileio/log.h"

#include <thread>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <queue>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace voxigen
{


template<typename _Chunk>
struct IORequest
{
    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandleType;

    enum Type
    {
        Read,
        Write
    };

    IORequest(Type type, unsigned int priority):type(type), priority(priority) {}
    bool operator<(const IORequest &rhs) const { return priority<rhs.priority; }

    Type type;
    unsigned int priority;

};

template<typename _Chunk>
struct IOReadRequest:public IORequest<_Chunk>
{
    typedef typename IORequest<_Chunk>::ChunkType ChunkType;
    typedef typename IORequest<_Chunk>::ChunkHandleType ChunkHandleType;
    typedef typename IORequest<_Chunk>::SharedChunkHandleType SharedChunkHandleType;
    typedef std::weak_ptr<ChunkHandleType> WeakChunkHandleType;

    IOReadRequest(SharedChunkHandleType chunkHandle):IORequest<_Chunk>(IORequest<_Chunk>::Type::Read, 500), chunkHandle(chunkHandle) {}
    IOReadRequest(unsigned int priority, SharedChunkHandleType chunkHandle):IORequest<_Chunk>(IORequest<_Chunk>::Type::Read, priority), chunkHandle(chunkHandle) {}

    WeakChunkHandleType chunkHandle;
};

template<typename _Chunk>
struct IOWriteRequest:public IORequest<_Chunk>
{
    typedef typename IORequest<_Chunk>::ChunkType ChunkType;
    typedef typename IORequest<_Chunk>::ChunkHandleType ChunkHandleType;
    typedef typename IORequest<_Chunk>::SharedChunkHandleType SharedChunkHandleType;

    IOWriteRequest(SharedChunkHandleType chunkHandle):IORequest<_Chunk>(IORequest<_Chunk>::Type::Write, 1000), chunkHandle(chunkHandle) {}
    IOWriteRequest(unsigned int priority, SharedChunkHandleType chunkHandle):IORequest<_Chunk>(IORequest<_Chunk>::Type::Write, priority), chunkHandle(chunkHandle) {}

    SharedChunkHandleType chunkHandle;
};

template<typename _Grid>
class DataStore:public DataHandler<RegionHash, RegionHandle<typename _Grid::RegionType>, typename _Grid::RegionType>
{
public:
//DataHandler typdefs
    typedef DataHandler<RegionHash, RegionHandle<typename _Grid::RegionType>, typename _Grid::RegionType> DataHandlerType;
    typedef typename DataHandlerType::HashType HashType;
    typedef typename DataHandlerType::DataHandle DataHandle;
    typedef typename DataHandlerType::SharedDataHandle SharedDataHandle;
    typedef typename DataHandlerType::SharedDataHandleMap SharedDataHandleMap;

    typedef typename _Grid::RegionType RegionType;
    typedef RegionHandle<RegionType> RegionHandleType;
    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;

    typedef typename _Grid::ChunkType ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef IORequest<ChunkType> IORequestType;
    typedef IOReadRequest<ChunkType> IOReadRequestType;
    typedef IOWriteRequest<ChunkType> IOWriteRequestType;
    typedef std::shared_ptr<IORequestType> SharedIORequest;

    DataStore(GridDescriptors<_Grid> *descriptors);

    void initialize();
    void terminate();
//    size_t handlesInUse();

    bool load(const std::string &name);

    void ioThread();
    void generatorThread();

    SharedRegionHandle getRegion(RegionHash regionHash);
    bool loadRegion(RegionHandleType *handle, size_t lod, bool force);
    bool cancelLoadRegion(RegionHandleType *handle);

    SharedChunkHandle getChunk(RegionHash regionHash, ChunkHash chunkHash);

    bool loadChunk(ChunkHandleType *handle, size_t lod, bool force=false);
    bool cancelLoadChunk(ChunkHandleType *handle);
//    void removeHandle(ChunkHandleType *chunkHandle);

    void addUpdated(Key hash);
    std::vector<Key> getUpdated();

    bool generateRegion(RegionHandleType *handle, size_t lod);

    bool generate(ChunkHandleType *chunkHandle, size_t lod);
    bool read(ChunkHandleType *chunkHandle, size_t lod);
    bool write(ChunkHandleType *chunkHandle);
    bool empty(ChunkHandleType *chunkHandle);
    bool cancel(ChunkHandleType *chunkHandle);

    void readChunk(ChunkHandleType *handle);
    void writeChunk(ChunkHandleType *handle);

protected:
    virtual DataHandle *newHandle(HashType hash);

private:
    void readChunk(IORequestType *request);
    void writeChunk(IORequestType *request);
    void addConfig(SharedDataHandle handle);
    void addConfig(ChunkHandleType *handle);

    void loadConfig();
    void saveConfig();
    void saveConfigTo(std::string configFile);
    void loadDataStore();
    void verifyDirectory();

    GridDescriptors<_Grid> *m_descriptors;
//    GeneratorQueue<_Grid> *m_generatorQueue;
//    ProcessQueue<_Grid> *m_processQueue;

//World files
    std::string m_directory;
    std::string m_configFile;
    std::string m_cacheDirectory;
    unsigned int m_version;

//IO thread
//    std::thread m_ioThread;
//    std::mutex m_ioMutex;
//    std::priority_queue<SharedIORequest> m_ioQueue;
//    std::condition_variable m_ioEvent;
//    bool m_ioThreadRun;
//    rapidjson::Document m_configDocument;

//Updated chunks
//    UpdateQueue *m_updateQueue;
};

} //namespace voxigen

#include "voxigen/volume/dataStore.inl"

#endif //_voxigen_dataStore_h_