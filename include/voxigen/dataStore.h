#ifndef _voxigen_dataStore_h_
#define _voxigen_dataStore_h_

#include "voxigen/chunkHandle.h"
#include "voxigen/regionHandle.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/jsonSerializer.h"
#include "voxigen/simpleFilesystem.h"
#include "voxigen/processQueue.h"
#include "voxigen/log.h"

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

    DataStore(GridDescriptors<_Grid> *descriptors, ProcessQueue<_Grid> *processQueue, GeneratorQueue<_Grid> *generatorQueue, UpdateQueue *updateQueue);

    void initialize();
    void terminate();
//    size_t handlesInUse();

    bool load(const std::string &name);

    void ioThread();
    void generatorThread();

    SharedRegionHandle getRegion(RegionHash regionHash);
    void loadRegion(SharedRegionHandle handle, size_t lod, bool force);
    void cancelLoadRegion(SharedRegionHandle handle);

    SharedChunkHandle getChunk(RegionHash regionHash, ChunkHash chunkHash);

    void loadChunk(SharedChunkHandle handle, size_t lod, bool force=false);
    void cancelLoadChunk(SharedChunkHandle handle);
//    void removeHandle(ChunkHandleType *chunkHandle);

    void addUpdated(Key hash);
    std::vector<Key> getUpdated();

    void generateRegion(SharedRegionHandle handle, size_t lod);

    void generate(SharedChunkHandle chunkHandle, size_t lod);
    void read(SharedChunkHandle chunkHandle, size_t lod);
    void write(SharedChunkHandle chunkHandle);
    void empty(SharedChunkHandle chunkHandle);
    void cancel(SharedChunkHandle chunkHandle);

    void readChunk(SharedChunkHandle handle);
    void writeChunk(SharedChunkHandle handle);

protected:
    virtual DataHandle *newHandle(HashType hash);

private:
    void readChunk(IORequestType *request);
    void writeChunk(IORequestType *request);
    void addConfig(SharedDataHandle handle);
    void addConfig(SharedChunkHandle handle);

    void loadConfig();
    void saveConfig();
    void saveConfigTo(std::string configFile);
    void loadDataStore();
    void verifyDirectory();

    GridDescriptors<_Grid> *m_descriptors;
    GeneratorQueue<_Grid> *m_generatorQueue;
    ProcessQueue<_Grid> *m_processQueue;

//World files
    std::string m_directory;
    std::string m_configFile;
    std::string m_cacheDirectory;
    unsigned int m_version;

//IO thread
    std::thread m_ioThread;
    std::mutex m_ioMutex;
    std::priority_queue<SharedIORequest> m_ioQueue;
    std::condition_variable m_ioEvent;
    bool m_ioThreadRun;
//    rapidjson::Document m_configDocument;

//Updated chunks
    UpdateQueue *m_updateQueue;
};

template<typename _Grid>
DataStore<_Grid>::DataStore(GridDescriptors<_Grid> *descriptors, ProcessQueue<_Grid> *processQueue, GeneratorQueue<_Grid> *generatorQueue, UpdateQueue *updateQueue):
m_descriptors(descriptors),
m_processQueue(processQueue),
m_generatorQueue(generatorQueue),
m_updateQueue(updateQueue)
{
    m_version=0;
}

template<typename _Grid>
void DataStore<_Grid>::initialize()
{
    m_ioThreadRun=true;
    m_ioThread=std::thread(std::bind(&DataStore<_Grid>::ioThread, this));
}

template<typename _Grid>
void DataStore<_Grid>::terminate()
{
    //thread flags are not atomic so we need the mutexes to coordinate the setting, 
    //otherwise would have to loop re-notifiying thread until it stopped
    {
        std::unique_lock<std::mutex> lock(m_ioMutex);
        m_ioThreadRun=false;
    }

    m_ioEvent.notify_all();
    m_ioThread.join();
}

template<typename _Grid>
typename DataStore<_Grid>::DataHandle *DataStore<_Grid>::newHandle(HashType hash)
{
//    return new RegionHandleType(hash, m_descriptors, m_generatorQueue, this, m_updateQueue);
    return new RegionHandleType(hash, m_descriptors, m_updateQueue);
}

template<typename _Grid>
bool DataStore<_Grid>::load(const std::string &directory)
{
    m_directory=directory;

    if(!fs::is_directory(directory))
    {
        if(fs::exists(directory))
            return false;

        fs::create_directory(directory);
    }

    m_configFile=m_directory+"/cacheConfig.json";

    if(!fs::exists(m_configFile))
        saveConfig();
    else
        loadConfig();

    loadDataStore();
    verifyDirectory();
    return true;
}

template<typename _Grid>
void DataStore<_Grid>::loadConfig()
{
    JsonUnserializer serializer;

    serializer.open(m_configFile.c_str());

    serializer.openObject();
    if(serializer.key("version"))
        m_version=serializer.getUInt();

    if(serializer.key("regions"))
    {
        if(serializer.openArray())
        {
            do
            {
                if(serializer.openObject())
                {
                    if(serializer.key("id"))
                    {
                        RegionHash hash=serializer.getUInt();
                        SharedRegionHandle regionHandle(newHandle(hash));

                        regionHandle->setCachedOnDisk(true);

                        if(serializer.key("empty"))
                            regionHandle->setEmpty(serializer.getBool());
                        else
                            regionHandle->setEmpty(true);

                        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(hash, regionHandle));
                    }

                    serializer.closeObject();
                }
            } while(serializer.advance());
            serializer.closeArray();
        }
    }
    serializer.closeObject();
}

template<typename _Grid>
void DataStore<_Grid>::saveConfig()
{
    saveConfigTo(m_configFile);
}

template<typename _Grid>
void DataStore<_Grid>::saveConfigTo(std::string configFile)
{
    JsonSerializer serializer;

    serializer.open(configFile.c_str());

    serializer.startObject();

    serializer.addKey("version");
    serializer.addInt(m_version);

    serializer.addKey("regions");
    serializer.startArray();
    for(auto &handle:this->m_dataHandles)
    {
        if(handle.second->empty())
        {
            serializer.startObject();
            serializer.addKey("id");
            serializer.addUInt(handle.second->hash());
            serializer.addKey("empty");
            serializer.addBool(handle.second->empty());
            serializer.endObject();
        }
    }
    serializer.endArray();

    serializer.endObject();
}

template<typename _Grid>
void DataStore<_Grid>::addConfig(SharedDataHandle handle)
{
    if(handle->empty)
    {
        std::string tempConfig=m_configFile+".tmp";

        saveConfigTo(tempConfig);
//        JsonUnserializer unserializer;
//        JsonSerializer serializer;
//
//        unserializer.open(m_configFile.c_str());
//        serializer.open(tempConfig.c_str());
//
//        unserializer.openObject();
//        serializer.startObject();
//
//        if(unserializer.key("version"))
//        {
//            unsigned int version=unserializer.getUInt();
//            serializer.addKey("version");
//            serializer.addUInt(version);
//        }
//
//        if(unserializer.key("regions"))
//        {
//            serializer.addKey("regions");
//            if(unserializer.openArray())
//            {
//                serializer.startArray();
//                do
//                {
//                    if(unserializer.openObject())
//                    {
//                        serializer.startObject();
//
//                        if(unserializer.key("id"))
//                        {
//                            RegionHash hash=unserializer.GetUint();
//
//                            serializer.addKey("id");
//                            serializer.addUInt(hash);
//                            
//                            bool empty=true;
//
//                            if(unserializer.key("empty"))
//                                empty=unserializer.GetBool();
//
//                            serializer.addKey("empty");
//                            serializer.addBool(empty);
//                        }
//
//                        unserializer.closeObject();
//                        serializer.endObject();
//                    }
//                } while(unserializer.advance());
//                
//                unserializer.closeArray();
//                serializer.endArray();
//            }
//        }
//        
//        unserializer.closeObject();
//        serializer.endObject();

//        fs::path configPath(m_configFile);
//        fs::path tempPath(tempConfig);
//        copy_file(tempPath, configPath, fs::copy_option::overwrite_if_exists);
        fs::copy_file(tempConfig, m_configFile, true);
    }
}

template<typename _Grid>
void DataStore<_Grid>::addConfig(SharedChunkHandle handle)
{
    //TODO - fix
    //lazy programming for the moment, see remarks in ioThread below
    if(handle->empty())
    {
        SharedRegionHandle regionHandle=getRegion(handle->regionHash());

        regionHandle->addConfig(handle);
    }
}

template<typename _Grid>
void DataStore<_Grid>::loadDataStore()
{
    std::vector<std::string> directories=fs::get_directories(m_directory);

    for(auto &entry:directories)
    {
        std::istringstream fileNameStream(entry);
        RegionHash hash;

        fileNameStream>>std::hex>>hash;

        SharedRegionHandle handle(newHandle(hash));

        handle->setCachedOnDisk(true);
        handle->setEmpty(false);

        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(hash, handle));
    }
}

template<typename _Grid>
void DataStore<_Grid>::verifyDirectory()
{

}

template<typename _Grid>
typename DataStore<_Grid>::SharedRegionHandle DataStore<_Grid>::getRegion(RegionHash hash)
{
    SharedRegionHandle regionHandle=this->getDataHandle(hash);

//    if(regionHandle->state()!=HandleState::Memory)
//    {
//        std::ostringstream directoryName;
//
//        directoryName<<std::hex<<hash;
//        std::string directory=m_directory+"/"+directoryName.str();
//
//        regionHandle->load(directory);
//    }
    return regionHandle;
}

template<typename _Grid>
void DataStore<_Grid>::loadRegion(SharedRegionHandle handle, size_t lod, bool force)
{
    if(force||(handle->state()!=HandleState::Memory))
    {
        //an action is in process lets not start something else as well
        if(handle->action()!=HandleAction::Idle)
            return;

        if(!handle->empty())
        {
            generateRegion(handle, lod);
        }
    }
}

template<typename _Grid>
void DataStore<_Grid>::cancelLoadRegion(SharedRegionHandle handle)
{
    if(handle->action()==HandleAction::Idle)
        return;

//    cancelRegion(handle);
}

template<typename _Grid>
typename DataStore<_Grid>::SharedChunkHandle DataStore<_Grid>::getChunk(RegionHash regionHash, ChunkHash chunkHash)
{
    return getRegion(regionHash)->getChunk(chunkHash);
}

template<typename _Grid>
void DataStore<_Grid>::loadChunk(SharedChunkHandle chunkHandle, size_t lod, bool force)
{
//    return getRegion(handle->regionHash())->loadChunk(handle, lod, force);
    if(force||(chunkHandle->state()!=HandleState::Memory))
    {
        //an action is in process lets not start something else as well
        if(chunkHandle->action()!=HandleAction::Idle)
            return;

        if(!chunkHandle->empty())
        {
            //we dont have it in memory so we need to load or generate it
            if(!chunkHandle->cachedOnDisk())
                generate(chunkHandle, lod);
            else
                read(chunkHandle, lod);
        }
    }
}

template<typename _Grid>
void DataStore<_Grid>::cancelLoadChunk(SharedChunkHandle chunkHandle)
{
//    return getRegion(handle->regionHash())->cancelLoadChunk(handle);
    //if we are not doing anything ignore
    if(chunkHandle->action()==HandleAction::Idle)
        return;

    cancel(chunkHandle);
}

template<typename _Grid>
void DataStore<_Grid>::ioThread()
{
    std::unique_lock<std::mutex> lock(m_ioMutex);

    //TODO - need batch system for cache updates
    //I am so hacking this up, going to store the config as a json doc for updating
    //need batching system for this, delayed writes (get multiple changes to chunks
    //in one write)
//    {
//        FILE *filePtr=fopen(m_configFile.c_str(), "rb");
//        char readBuffer[65536];
//
//        rapidjson::FileReadStream readStream(filePtr, readBuffer, sizeof(readBuffer));
//
//        m_configDocument.ParseStream(readStream);
//        fclose(filePtr);
//    }

    std::vector<SharedIORequest> requests;

    requests.reserve(10);
    while(m_ioThreadRun)
    {
        if(m_ioQueue.empty())
        {
            m_ioEvent.wait(lock);
            continue;
        }

        SharedIORequest request;
        size_t count=0;

        while((!m_ioQueue.empty()&&count<10))
        {

            requests.push_back(m_ioQueue.top());
            m_ioQueue.pop();
            count++;
        }

        if(!requests.empty())
        {
            lock.unlock();//drop lock while working

            for(size_t i=0; i<requests.size(); ++i)
            {
                switch(requests[i]->type)
                {
                case IORequestType::Read:
                    readChunk(requests[i].get());
                    break;
                case IORequestType::Write:
                    writeChunk(requests[i].get());
                    break;
                }
            }
            requests.clear();

            lock.lock();
        }
    }
}

template<typename _Grid>
void DataStore<_Grid>::readChunk(IORequestType *request)
{
    IOReadRequestType *readRequest=(IOReadRequestType *)request;

    readChunk(readRequest->chunkHandle.lock());
}

template<typename _Grid>
void DataStore<_Grid>::readChunk(SharedChunkHandle chunkHandle)
{
//    IOReadRequestType *readRequest=(IOReadRequestType *)request;
//    SharedChunkHandle chunkHandle=readRequest->chunkHandle.lock();

    if(!chunkHandle) //chunk no longer needed, drop it
        return;

    if(!chunkHandle->empty())
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash()<<".chk";
        std::string fileName=m_directory+fileNameStream.str();

        chunkHandle->read(m_descriptors, fileName);
//        glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHandle->hash);
//        glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;
//
//        chunkHandle->chunk=std::make_unique<ChunkType>(chunkHandle->hash, 0, chunkIndex, offset);
//
//        auto &cells=chunkHandle->chunk->getCells();
//        std::ifstream file;
//
//        file.open(fileName, std::ofstream::in|std::ofstream::binary);
//        //        for(auto block:blocks)
//        //            block->deserialize(file);
//        file.read((char *)cells.data(), cells.size()*sizeof(_Grid::CellType));
//        file.close();
    }

//    chunkHandle->status=ChunkHandleType::Memory;
//    addToUpdatedQueue(chunkHandle->hash);
    m_updateQueue->add(chunkHandle->hash());
}

template<typename _Grid>
void DataStore<_Grid>::writeChunk(IORequestType *request)
{
    IOWriteRequestType *writeRequest=(IOWriteRequestType *)request;

    writeChunk(writeRequest->chunkHandle);

    //drop shared_ptr
    writeRequest->chunkHandle.reset();
}

template<typename _Grid>
void DataStore<_Grid>::writeChunk(SharedChunkHandle chunkHandle)
{
//    IOWriteRequestType *writeRequest=(IOWriteRequestType *)request;
//    SharedChunkHandle chunkHandle=writeRequest->chunkHandle;

    if(!chunkHandle->empty())
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash()<<".chk";
        std::string fileName=m_directory+fileNameStream.str();

        chunkHandle->write(m_descriptors, fileName);
//        auto &cells=chunkHandle->chunk->getCells();
//        std::ofstream file;
//
//        file.open(fileName, std::ofstream::out|std::ofstream::trunc|std::ofstream::binary);
//        //        for(auto block:blocks)
//        //            block->serialize(file);
//        file.write((char *)cells.data(), cells.size()*sizeof(_Grid::CellType));
//        file.close();
    }

//    chunkHandle->cachedOnDisk=true;
    
    //need to alter this to save in batched and update config then
    addConfig(chunkHandle);

    //drop shared_ptr
//    writeRequest->chunkHandle.reset();
}


template<typename _Grid>
void DataStore<_Grid>::generateRegion(SharedRegionHandle handle, size_t lod)
{
#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"MainThread - RegionHandle "<<handle.get()<<" ("<<chunkHandle->regionHash()<<", "<<chunkHandle->hash()<<") generating";
#endif//LOG_PROCESS_QUEUE

    handle->setAction(HandleAction::Generating);
    m_processQueue->addGenerateRegion(handle, lod);
}

template<typename _Grid>
void DataStore<_Grid>::generate(SharedChunkHandle chunkHandle, size_t lod)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %x (%d, %d) generating\n", chunkHandle.get(), chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

    chunkHandle->setAction(HandleAction::Generating);
    m_processQueue->addGenerate(chunkHandle, lod);
}

template<typename _Grid>
void DataStore<_Grid>::read(SharedChunkHandle chunkHandle, size_t lod)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread -  ChunkHandle %x (%d, %d) reading", chunkHandle.get(), chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    chunkHandle->setAction(HandleAction::Reading);
    m_processQueue->addRead(chunkHandle, lod);
}

template<typename _Grid>
void DataStore<_Grid>::write(SharedChunkHandle chunkHandle)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread -  ChunkHandle %x (%d, %d) writing", chunkHandle.get(), chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    chunkHandle->setAction(HandleAction::Writing);
    m_processQueue->addWrite(chunkHandle);
}

template<typename _Grid>
void DataStore<_Grid>::empty(SharedChunkHandle chunkHandle)
{
//    chunkHandle->setAction(HandleAction::Updating);
//    m_processQueue->addUpdate(chunkHandle);
}

template<typename _Grid>
void DataStore<_Grid>::cancel(SharedChunkHandle chunkHandle)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %x (%d, %d) canceling", chunkHandle.get(), chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

    m_processQueue->addCancel(chunkHandle);
}


} //namespace voxigen

#endif //_voxigen_dataStore_h_