#ifndef _voxigen_dataStore_h_
#define _voxigen_dataStore_h_

#include "voxigen/chunkHandle.h"
#include "voxigen/regionHandle.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/jsonSerializer.h"
#include "voxigen/simpleFileSystem.h"

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
    typedef std::weak_ptr<ChunkHandleType> WeakChunkHandleType;

    IOReadRequest(SharedChunkHandleType chunkHandle):IORequest<_Chunk>(Type::Read, 500), chunkHandle(chunkHandle) {}
    IOReadRequest(unsigned int priority, SharedChunkHandleType chunkHandle):IORequest<_Chunk>(Type::Read, priority), chunkHandle(chunkHandle) {}

    WeakChunkHandleType chunkHandle;
};

template<typename _Chunk>
struct IOWriteRequest:public IORequest<_Chunk>
{
    IOWriteRequest(SharedChunkHandleType chunkHandle):IORequest<_Chunk>(Type::Write, 1000), chunkHandle(chunkHandle) {}
    IOWriteRequest(unsigned int priority, SharedChunkHandleType chunkHandle):IORequest<_Chunk>(Type::Write, priority), chunkHandle(chunkHandle) {}

    SharedChunkHandleType chunkHandle;
};

template<typename _Grid>
class DataStore:public DataHandler<RegionHash, RegionHandle<_Grid>, typename _Grid::RegionType>
{
public:
    typedef typename _Grid::RegionType RegionType;
    typedef RegionHandle<_Grid> RegionHandleType;
    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;
//    typedef std::weak_ptr<RegionHandleType> WeakRegionHandle;
//    typedef std::unordered_map<RegionHash, WeakRegionHandle> WeakRegionHandleMap;
//    typedef std::unordered_map<RegionHash, SharedRegionHandle> RegionHandleMap;

    typedef typename _Grid::ChunkType ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef IORequest<ChunkType> IORequestType;
    typedef IOReadRequest<ChunkType> IOReadRequestType;
    typedef IOWriteRequest<ChunkType> IOWriteRequestType;
    typedef std::shared_ptr<IORequestType> SharedIORequest;

    DataStore(GridDescriptors<_Grid> *descriptors, GeneratorQueue<_Grid> *generatorQueue, UpdateQueue *updateQueue);

    void initialize();
    void terminate();
//    size_t handlesInUse();

    bool load(const std::string &name);

    void ioThread();
    void generatorThread();

    SharedRegionHandle getRegion(RegionHash regionHash);
    SharedChunkHandle getChunk(RegionHash regionHash, ChunkHash chunkHash);

    void loadChunk(SharedChunkHandle handle, size_t lod);
//    void removeHandle(ChunkHandleType *chunkHandle);

    void addUpdated(Key hash);
    std::vector<Key> getUpdated();

    void read(SharedChunkHandle chunkHandle);
    void write(SharedChunkHandle chunkHandle);

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
DataStore<_Grid>::DataStore(GridDescriptors<_Grid> *descriptors, GeneratorQueue<_Grid> *generatorQueue, UpdateQueue *updateQueue):
m_descriptors(descriptors),
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
    return new RegionHandleType(hash, m_descriptors, m_generatorQueue, this, m_updateQueue);
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

                        regionHandle->cachedOnDisk=true;

                        if(serializer.key("empty"))
                            regionHandle->empty=serializer.getBool();
                        else
                            regionHandle->empty=true;

                        m_dataHandles.insert(SharedDataHandleMap::value_type(hash, regionHandle));
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
    for(auto &handle:m_dataHandles)
    {
        if(handle.second->empty)
        {
            serializer.startObject();
            serializer.addKey("id");
            serializer.addUInt(handle.second->hash);
            serializer.addKey("empty");
            serializer.addBool(handle.second->empty);
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
    if(handle->empty)
    {
        SharedRegionHandle regionHandle=getRegion(handle->regionHash);

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

        handle->cachedOnDisk=true;
        handle->empty=false;

        m_dataHandles.insert(SharedDataHandleMap::value_type(hash, handle));
    }
}

template<typename _Grid>
void DataStore<_Grid>::verifyDirectory()
{

}

template<typename _Grid>
typename DataStore<_Grid>::SharedRegionHandle DataStore<_Grid>::getRegion(RegionHash hash)
{
    SharedRegionHandle regionHandle=getDataHandle(hash);

    if(regionHandle->getStatus()!=RegionHandleType::Loaded)
    {
        std::ostringstream directoryName;

        directoryName<<std::hex<<hash;
        std::string directory=m_directory+"/"+directoryName.str();

        regionHandle->load(directory);
    }
    return regionHandle;
}

template<typename _Grid>
typename DataStore<_Grid>::SharedChunkHandle DataStore<_Grid>::getChunk(RegionHash regionHash, ChunkHash chunkHash)
{
    return getRegion(regionHash)->getChunk(chunkHash);
}

template<typename _Grid>
void DataStore<_Grid>::loadChunk(SharedChunkHandle handle, size_t lod)
{
    return getRegion(handle->regionHash)->loadChunk(handle, lod);
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
    SharedChunkHandle chunkHandle=readRequest->chunkHandle.lock();

    if(!chunkHandle) //chunk no longer needed, drop it
        return;

    if(!chunkHandle->empty)
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash<<".chk";
        std::string fileName=m_directory+fileNameStream.str();

        glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHandle->hash);
        glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;

        chunkHandle->chunk=std::make_unique<ChunkType>(chunkHandle->hash, 0, chunkIndex, offset);

        auto &cells=chunkHandle->chunk->getCells();
        std::ifstream file;

        file.open(fileName, std::ofstream::in|std::ofstream::binary);
        //        for(auto block:blocks)
        //            block->deserialize(file);
        file.read((char *)cells.data(), cells.size()*sizeof(_Grid::CellType));
        file.close();
    }

    chunkHandle->status=ChunkHandleType::Memory;
//    addToUpdatedQueue(chunkHandle->hash);
    m_updateQueue->add(chunkHandle->hash);
}

template<typename _Grid>
void DataStore<_Grid>::writeChunk(IORequestType *request)
{
    IOWriteRequestType *writeRequest=(IOWriteRequestType *)request;
    SharedChunkHandle chunkHandle=writeRequest->chunkHandle;

    if(!chunkHandle->empty)
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash<<".chk";
        std::string fileName=m_directory+fileNameStream.str();

        auto &cells=chunkHandle->chunk->getCells();
        std::ofstream file;

        file.open(fileName, std::ofstream::out|std::ofstream::trunc|std::ofstream::binary);
        //        for(auto block:blocks)
        //            block->serialize(file);
        file.write((char *)cells.data(), cells.size()*sizeof(_Grid::CellType));
        file.close();
    }

    chunkHandle->cachedOnDisk=true;
    
    //need to alter this to save in batched and update config then
    addConfig(chunkHandle);

    //drop shared_ptr
    writeRequest->chunkHandle.reset();
}

template<typename _Grid>
void DataStore<_Grid>::read(SharedChunkHandle chunkHandle)
{
    {
        std::unique_lock<std::mutex> lock(m_ioMutex);

        std::shared_ptr<IOReadRequestType> request=std::make_shared<IOReadRequestType>(IORequestType::Read, chunkHandle);

        m_ioQueue.push(request);
    }
    m_ioEvent.notify_all();
}

template<typename _Grid>
void DataStore<_Grid>::write(SharedChunkHandle chunkHandle)
{
    {
        std::unique_lock<std::mutex> lock(m_ioMutex);

        std::shared_ptr<IOWriteRequestType> request=std::make_shared<IOWriteRequestType>(IORequestType::Write, chunkHandle);

        m_ioQueue.push(request);
    }
    m_ioEvent.notify_all();
}


} //namespace voxigen

#endif //_voxigen_dataStore_h_