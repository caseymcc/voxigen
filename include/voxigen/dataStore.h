#ifndef _voxigen_dataStore_h_
#define _voxigen_dataStore_h_

#include "voxigen/chunkHandle.h"
#include "voxigen/segmentHandle.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"

#include <thread>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <queue>
#include <sstream>
#include <iomanip>

#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>

#include <boost/filesystem.hpp>
namespace fs=boost::filesystem;

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

namespace fs=boost::filesystem;

template<typename _Segment, typename _Chunk>
class DataStore:public DataHandler<SegmentHash, SegmentHandle<_Segment>, _Segment>
{
public:
    typedef _Segment SegmentType;
    typedef SegmentHandle<_Segment> SegmentHandleType;
    typedef std::shared_ptr<SegmentHandleType> SharedSegmentHandle;
//    typedef std::weak_ptr<SegmentHandleType> WeakSegmentHandle;
//    typedef std::unordered_map<SegmentHash, WeakSegmentHandle> WeakSegmentHandleMap;
//    typedef std::unordered_map<SegmentHash, SharedSegmentHandle> SegmentHandleMap;

    typedef _Chunk ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef IORequest<_Chunk> IORequestType;
    typedef IOReadRequest<_Chunk> IOReadRequestType;
    typedef IOWriteRequest<_Chunk> IOWriteRequestType;
    typedef std::shared_ptr<IORequestType> SharedIORequest;

    DataStore(GridDescriptors *descriptors, GeneratorQueue<ChunkType> *generatorQueue, UpdateQueue *updateQueue);

    void initialize();
    void terminate();
//    size_t handlesInUse();

    bool load(const std::string &name);

    void ioThread();
    void generatorThread();

    SharedSegmentHandle getSegment(SegmentHash segmentHash);
    SharedChunkHandle getChunk(SegmentHash segmentHash, ChunkHash chunkHash);
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
    void loadDataStore();
    void verifyDirectory();

    GridDescriptors *m_descriptors;
    GeneratorQueue<ChunkType> *m_generatorQueue;

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
    rapidjson::Document m_configDocument;

//Updated chunks
    UpdateQueue *m_updateQueue;
};

template<typename _Segment, typename _Chunk>
DataStore<_Segment, _Chunk>::DataStore(GridDescriptors *descriptors, GeneratorQueue<ChunkType> *generatorQueue, UpdateQueue *updateQueue):
m_descriptors(descriptors),
m_generatorQueue(generatorQueue),
m_updateQueue(updateQueue)
{
    m_version=0;
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::initialize()
{
    m_ioThreadRun=true;
    m_ioThread=std::thread(std::bind(&DataStore<_Segment, _Chunk>::ioThread, this));
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::terminate()
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

template<typename _Segment, typename _Chunk>
typename DataStore<_Segment, _Chunk>::DataHandle *DataStore<_Segment, _Chunk>::newHandle(HashType hash)
{
    return new SegmentHandleType(hash, m_descriptors, m_generatorQueue, this, m_updateQueue);
}

template<typename _Segment, typename _Chunk>
bool DataStore<_Segment, _Chunk>::load(const std::string &directory)
{
    m_directory=directory;
    fs::path worldPath(directory);

    if(!fs::is_directory(worldPath))
    {
        if(fs::exists(worldPath))
            return false;

        fs::create_directory(worldPath);
    }

    m_configFile=m_directory+"/cacheConfig.json";
    fs::path configPath(m_configFile);

    if(!fs::exists(configPath))
        saveConfig();
    else
        loadConfig();

    loadDataStore();
    verifyDirectory();
    return true;
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::loadConfig()
{
    //    m_configFile=m_directory.string()+"/chunkConfig.json";
    FILE *filePtr=fopen(m_configFile.c_str(), "rb");
    char readBuffer[65536];

    rapidjson::FileReadStream readStream(filePtr, readBuffer, sizeof(readBuffer));

    rapidjson::Document document;

    document.ParseStream(readStream);

    assert(document.IsObject());

    m_version=document["version"].GetUint();

    const rapidjson::Value &segments=document["segments"];
    assert(segments.IsArray());

    for(rapidjson::SizeType i=0; i<segments.Size(); ++i)
    {
        const rapidjson::Value &segmentValue=segments[i];

        SegmentHash hash=segmentValue["id"].GetUint();
        SharedSegmentHandle segmentHandle(newHandle(hash));

        segmentHandle->cachedOnDisk=true;
        segmentHandle->empty=segmentValue["empty"].GetBool();

        m_dataHandles.insert(SharedDataHandleMap::value_type(hash, segmentHandle));
    }

    fclose(filePtr);
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::saveConfig()
{
    //    m_configFile=m_directory.string()+"/chunkConfig.json";
    FILE *filePtr=fopen(m_configFile.c_str(), "wb");
    char writeBuffer[65536];

    rapidjson::FileWriteStream fileStream(filePtr, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fileStream);

    writer.StartObject();

    writer.Key("version");
    writer.Uint(m_version);

    writer.Key("segments");
    writer.StartArray();
    for(auto &handle:m_dataHandles)
    {
        if(handle.second->empty)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Uint(handle.second->hash);
            writer.Key("empty");
            writer.Bool(handle.second->empty);
            writer.EndObject();
        }
    }
    writer.EndArray();

    writer.EndObject();

    fclose(filePtr);
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::addConfig(SharedDataHandle handle)
{
    //TODO - fix
    //lazy programming for the moment, see remarks in ioThread below
    if(handle->empty)
    {
        rapidjson::Document::AllocatorType &allocator=m_configDocument.GetAllocator();
        rapidjson::Value &segments=m_configDocument["segments"];
        assert(segments.IsArray());

        rapidjson::Value segment(rapidjson::kObjectType);

        segment.AddMember("id", handle->chunkHash, allocator);
        segment.AddMember("empty", handle->empty, allocator);

        segments.PushBack(segment, allocator);
        //    m_configFile=m_directory.string()+"/chunkConfig.json";

        std::string tempConfig=m_configFile+ror".tmp";
        FILE *filePtr=fopen(tempConfig.c_str(), "wb");
        char writeBuffer[65536];

        rapidjson::FileWriteStream fileStream(filePtr, writeBuffer, sizeof(writeBuffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fileStream);

        m_configDocument.Accept(writer);
        fclose(filePtr);

        fs::path configPath(m_configFile);
        fs::path tempPath(tempConfig);
        copy_file(tempPath, configPath, fs::copy_option::overwrite_if_exists);
    }
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::addConfig(SharedChunkHandle handle)
{
    //TODO - fix
    //lazy programming for the moment, see remarks in ioThread below
    if(handle->empty)
    {
        SharedSegmentHandle segmentHandle=getSegment(handle->segmentHash);

        segmentHandle->addConfig(handle);
    }
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::loadDataStore()
{
    fs::path chunkDirectory(m_directory);

    for(auto &entry:fs::directory_iterator(chunkDirectory))
    {
        if(fs::is_directory(entry.path()))
        {
            std::istringstream fileNameStream(entry.path().stem().string());
            SegmentHash hash;

            fileNameStream>>std::hex>>hash;

            SharedSegmentHandle handle(newHandle(hash));

            handle->cachedOnDisk=true;
            handle->empty=false;

            m_dataHandles.insert(SharedDataHandleMap::value_type(hash, handle));
        }
    }
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::verifyDirectory()
{

}

template<typename _Segment, typename _Chunk>
typename DataStore<_Segment, _Chunk>::SharedSegmentHandle DataStore<_Segment, _Chunk>::getSegment(SegmentHash hash)
{
    SharedSegmentHandle segmentHandle=getDataHandle(hash);

    if(segmentHandle->getStatus()!=SegmentHandleType::Loaded)
    {
        std::ostringstream directoryName;

        directoryName<<std::hex<<hash;
        std::string directory=m_directory+"/"+directoryName.str();

        segmentHandle->load(directory);
    }
    return segmentHandle;
}

template<typename _Segment, typename _Chunk>
typename DataStore<_Segment, _Chunk>::SharedChunkHandle DataStore<_Segment, _Chunk>::getChunk(SegmentHash segmentHash, ChunkHash chunkHash)
{
    return getSegment(segmentHash)->getChunk(chunkHash);
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::ioThread()
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

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::readChunk(IORequestType *request)
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
        file.read((char *)cells.data(), cells.size()*sizeof(_Chunk::CellType));
        file.close();
    }

    chunkHandle->status=ChunkHandleType::Memory;
//    addToUpdatedQueue(chunkHandle->hash);
    m_updateQueue->add(chunkHandle->hash);
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::writeChunk(IORequestType *request)
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
        file.write((char *)cells.data(), cells.size()*sizeof(_Chunk::CellType));
        file.close();
    }

    chunkHandle->cachedOnDisk=true;
    
    //need to alter this to save in batched and update config then
    addConfig(chunkHandle);

    //drop shared_ptr
    writeRequest->chunkHandle.reset();
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::read(SharedChunkHandle chunkHandle)
{
    {
        std::unique_lock<std::mutex> lock(m_ioMutex);

        std::shared_ptr<IOReadRequestType> request=std::make_shared<IOReadRequestType>(IORequestType::Read, chunkHandle);

        m_ioQueue.push(request);
    }
    m_ioEvent.notify_all();
}

template<typename _Segment, typename _Chunk>
void DataStore<_Segment, _Chunk>::write(SharedChunkHandle chunkHandle)
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