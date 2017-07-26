#ifndef _voxigen_chunkHandler_h_
#define _voxigen_chunkHandler_h_

#include "voxigen/chunkHandle.h"
#include "voxigen/worldDescriptors.h"
#include "voxigen/worldGenerator.h"

#include <thread>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <queue>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iomanip>

#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>

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

template<typename _Chunk>
class ChunkHandler
{
public:
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunk;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
    typedef std::weak_ptr<ChunkHandleType> WeakChunkHandle;
    typedef std::unordered_map<unsigned int, WeakChunkHandle> WeakChunkHandleMap;
    typedef std::unordered_map<unsigned int, SharedChunkHandle> ChunkHandleMap;

    typedef IORequest<_Chunk> IORequestType;
    typedef IOReadRequest<_Chunk> IOReadRequestType;
    typedef IOWriteRequest<_Chunk> IOWriteRequestType;
    typedef std::shared_ptr<IORequestType> SharedIORequest;

    ChunkHandler(WorldDescriptors *descriptors);

    void initialize();
    void terminate();
    size_t handlesInUse();

    bool load(const std::string &name);

    void ioThread();
    void generatorThread();
    
    SharedChunkHandle getChunk(unsigned int hash);
    void removeHandle(ChunkHandleType *chunkHandle);

    std::vector<unsigned int> getUpdatedChunks();

private:
    void addReadEvent(SharedChunkHandle chunkHandle);
    void readChunk(IORequestType *request);
    void addWriteEvent(SharedChunkHandle chunkHandle);
    void writeChunk(IORequestType *request);

    void addConfig(SharedChunkHandle chunkHandle);

    void addToGenerateQueue(SharedChunkHandle chunkHandle);
    void addToUpdatedQueue(unsigned int hash);

    void loadConfig();
    void saveConfig();
    void loadChunkCache();
    void verifyDirectory();

    WorldDescriptors *m_descriptors;
    WorldGenerator<ChunkType> m_worldGenerator;

//World files
    std::string m_worldDirectory;
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

//generator thread/queue
    std::mutex m_generatorMutex;
    std::thread m_generatorThread;
    std::queue<SharedChunkHandle> m_generatorQueue;
    std::condition_variable m_generatorEvent;
    bool m_generatorThreadRun;

//Data store
    std::mutex m_chunkMutex;
    ChunkHandleMap m_chunkHandles;
    WeakChunkHandleMap m_weakChunkHandles;

//Status updates
    std::mutex m_chunkUpdatedMutex;
    std::vector<unsigned int> m_chunksUpdated;
};

template<typename _Chunk>
ChunkHandler<_Chunk>::ChunkHandler(WorldDescriptors *descriptors):
m_descriptors(descriptors),
m_worldGenerator(descriptors)
{
    m_version=0;
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::initialize()
{
    m_ioThreadRun=true;
    m_ioThread=std::thread(std::bind(&ChunkHandler<_Chunk>::ioThread, this));

    m_generatorThreadRun=true;
    m_generatorThread=std::thread(std::bind(&ChunkHandler<_Chunk>::generatorThread, this));
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::terminate()
{
    //thread flags are not atomic so we need the mutexes to coordinate the setting, 
    //otherwise would have to loop re-notifiying thread until it stopped
    {
        std::unique_lock<std::mutex> lock(m_ioMutex);
        m_ioThreadRun=false;
    }
    

    {
        std::unique_lock<std::mutex> lock(m_generatorMutex);
        m_generatorThreadRun=false;
    }
    
    m_ioEvent.notify_all();
    m_generatorEvent.notify_all();

    m_ioThread.join();
    m_generatorThread.join();
    
}

template<typename _Chunk>
size_t ChunkHandler<_Chunk>::handlesInUse()
{
    std::unique_lock<std::mutex> lock(m_chunkMutex);

    size_t count=0;
    for(auto iter:m_weakChunkHandles)
    {
        if(iter.second.lock())
            count++;
    }

    return count;
}

template<typename _Chunk>
bool ChunkHandler<_Chunk>::load(const std::string &directory)
{
    m_worldDirectory=directory;
    fs::path worldPath(directory);

    if(!fs::is_directory(worldPath))
    {
        if(fs::exists(worldPath))
            return false;

        fs::create_directory(worldPath);
    }

    m_configFile=m_worldDirectory+"/chunkConfig.json";
    fs::path configPath(m_configFile);

    if(!fs::exists(configPath))
        saveConfig();
    else
        loadConfig();

    loadChunkCache();
    verifyDirectory();
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::loadConfig()
{
//    m_configFile=m_worldDirectory.string()+"/chunkConfig.json";
    FILE *filePtr=fopen(m_configFile.c_str(), "rb");
    char readBuffer[65536];

    rapidjson::FileReadStream readStream(filePtr, readBuffer, sizeof(readBuffer));

    rapidjson::Document document;
    
    document.ParseStream(readStream);

    assert(document.IsObject());

    m_version=document["version"].GetUint();

    const rapidjson::Value &chunks=document["chunks"];
    assert(chunks.IsArray());

    for(rapidjson::SizeType i=0; i<chunks.Size(); ++i)
    {
        const rapidjson::Value &chunk=chunks[i];

        unsigned int hash=chunk["id"].GetUint();
        SharedChunkHandle chunkHandle=std::make_shared<ChunkHandleType>(hash);

        chunkHandle->cachedOnDisk=true;
        chunkHandle->empty=chunk["empty"].GetBool();

        m_chunkHandles.insert(ChunkHandleMap::value_type(hash, chunkHandle));
    }

    fclose(filePtr);
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::saveConfig()
{
//    m_configFile=m_worldDirectory.string()+"/chunkConfig.json";
    FILE *filePtr=fopen(m_configFile.c_str(), "wb");
    char writeBuffer[65536];

    rapidjson::FileWriteStream fileStream(filePtr, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fileStream);

    writer.StartObject();
    
    writer.Key("version");
    writer.Uint(m_version);

    writer.Key("chunks");
    writer.StartArray();
    for(auto &chunkHandle:m_chunkHandles)
    {
        if(chunkHandle.second->empty)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Uint(chunkHandle.second->hash);
            writer.Key("empty");
            writer.Bool(chunkHandle.second->empty);
            writer.EndObject();
        }
    }
    writer.EndArray();

    writer.EndObject();

    fclose(filePtr);
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::addConfig(SharedChunkHandle chunkHandle)
{
 //TODO - fix
 //lazy programming for the moment, see remarks in ioThread below
    if(chunkHandle->empty)
    {
        rapidjson::Document::AllocatorType &allocator=m_configDocument.GetAllocator();
        rapidjson::Value &chunks=m_configDocument["chunks"];
        assert(chunks.IsArray());

        rapidjson::Value chunk(rapidjson::kObjectType);

        chunk.AddMember("id", chunkHandle->hash, allocator);
        chunk.AddMember("empty", chunkHandle->empty, allocator);

        chunks.PushBack(chunk, allocator);
        //    m_configFile=m_worldDirectory.string()+"/chunkConfig.json";

        std::string tempConfig=m_configFile+".tmp";
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

template<typename _Chunk>
void ChunkHandler<_Chunk>::loadChunkCache()
{
    fs::path chunkDirectory(m_worldDirectory);

    for(auto &entry:fs::directory_iterator(chunkDirectory))
    {
        if(fs::is_regular_file(entry.path()))
        {
            if(entry.path().extension().string()!=".chk")
                continue;

            std::istringstream fileNameStream(entry.path().stem().string());
            unsigned int hash;

            fileNameStream.ignore(6);
            fileNameStream>>std::hex>>hash;

            SharedChunkHandle chunkHandle=std::make_shared<ChunkHandleType>(hash);

            chunkHandle->cachedOnDisk=true;
            chunkHandle->empty=false;

            m_chunkHandles.insert(ChunkHandleMap::value_type(hash, chunkHandle));
        }
    }
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::verifyDirectory()
{
    
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::ioThread()
{
    std::unique_lock<std::mutex> lock(m_ioMutex);

    //TODO - need batch system for cache updates
    //I am so hacking this up, going to store the config as a json doc for updating
    //need batching system for this, delayed writes (get multiple changes to chunks
    //in one write)
    {
        FILE *filePtr=fopen(m_configFile.c_str(), "rb");
        char readBuffer[65536];

        rapidjson::FileReadStream readStream(filePtr, readBuffer, sizeof(readBuffer));

        m_configDocument.ParseStream(readStream);
        fclose(filePtr);
    }

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

template<typename _Chunk>
void ChunkHandler<_Chunk>::readChunk(IORequestType *request)
{
    IOReadRequestType *readRequest=(IOReadRequestType *)request;
    SharedChunkHandle chunkHandle=readRequest->chunkHandle.lock();

    if(!chunkHandle) //chunk no longer needed, drop it
        return;

    if(!chunkHandle->empty)
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash<<".chk";
        std::string fileName=m_worldDirectory+fileNameStream.str();

        glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHandle->hash);
        glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;

        chunkHandle->chunk=std::make_unique<ChunkType>(chunkHandle->hash, 0, chunkIndex, offset);

        auto &blocks=chunkHandle->chunk->getBlocks();
        std::ifstream file;

        file.open(fileName, std::ofstream::in|std::ofstream::binary);
        //        for(auto block:blocks)
        //            block->deserialize(file);
        file.read((char *)blocks.data(), blocks.size()*sizeof(_Chunk::BlockType));
        file.close();
    }

    chunkHandle->status=ChunkHandleType::Memory;
    addToUpdatedQueue(chunkHandle->hash);
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::writeChunk(IORequestType *request)
{
    IOWriteRequestType *writeRequest=(IOWriteRequestType *)request;
    SharedChunkHandle chunkHandle=writeRequest->chunkHandle;

    if(!chunkHandle->empty)
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash<<".chk";
        std::string fileName=m_worldDirectory+fileNameStream.str();

        auto &blocks=chunkHandle->chunk->getBlocks();
        std::ofstream file;

        file.open(fileName, std::ofstream::out|std::ofstream::trunc|std::ofstream::binary);
        //        for(auto block:blocks)
        //            block->serialize(file);
        file.write((char *)blocks.data(), blocks.size()*sizeof(_Chunk::BlockType));
        file.close();
    }
    
    chunkHandle->cachedOnDisk=true;
    //need to alter this to save in batched and update config then
    addConfig(chunkHandle);

    //drop shared_ptr
    writeRequest->chunkHandle.reset();
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::generatorThread()
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

        lock.unlock();//drop lock while working

        chunkHandle->chunk=m_worldGenerator.generateChunk(chunkHandle->hash);
        chunkHandle->status=ChunkHandleType::Memory;

        if(chunkHandle->chunk->validBlockCount()<=0)
            chunkHandle->empty=true;
        else
            chunkHandle->empty=false;

        addToUpdatedQueue(chunkHandle->hash);
        addWriteEvent(chunkHandle);
        chunkHandle.reset();//release pointer while not holding lock as there is a chance this will call removeHandle
                            //which will lock m_chunkMutex and safer to only have one lock at a time
        lock.lock();
    }
}

template<typename _Chunk>
typename ChunkHandler<_Chunk>::SharedChunkHandle ChunkHandler<_Chunk>::getChunk(unsigned int hash)
{
    std::unique_lock<std::mutex> lock(m_chunkMutex);

    auto iter=m_weakChunkHandles.find(hash);

    if(iter!=m_weakChunkHandles.end())
    {
        if(!iter->second.expired())
            return iter->second.lock(); //we already have it and somebody else has it as well
    }

    SharedChunkHandle returnHandle;

    //see if we already know about the chunk
    auto handleIter=m_chunkHandles.find(hash);

    if(handleIter==m_chunkHandles.end())
    {
        //we dont know about this one, create and send for generation
        SharedChunkHandle handle;
        ChunkHandleType *chunkHandle=new ChunkHandleType(hash);

        //ceate local handle for data storage
        handle.reset(chunkHandle);
        //Create shared handle to notify handler when it is no longer in use
        returnHandle.reset(chunkHandle, std::bind(&ChunkHandler<_Chunk>::removeHandle, this, std::placeholders::_1));

        m_chunkHandles[hash]=handle;

        addToGenerateQueue(returnHandle);
    }
    else
    {
        ChunkHandleType *chunkHandle=handleIter->second.get();

        //Create shared handle to notify handler when it is no longer in use
        returnHandle.reset(chunkHandle, std::bind(&ChunkHandler<_Chunk>::removeHandle, this, std::placeholders::_1));

        if(chunkHandle->status!=ChunkHandleType::Memory)
        {
            if(chunkHandle->empty) //empty is already loaded
                chunkHandle->status=ChunkHandleType::Memory;
            else
            {
                //we dont have it in memory so we need to load it
                if(!chunkHandle->cachedOnDisk) //shouldn't happen if cache working but in for debug
                    addToGenerateQueue(returnHandle);
                else
                    addReadEvent(returnHandle);
            }
        }
    }

    m_weakChunkHandles[hash]=returnHandle;
    return returnHandle;
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::removeHandle(ChunkHandleType *chunkHandle)
{
    std::unique_lock<std::mutex> lock(m_chunkMutex);

    auto iter=m_weakChunkHandles.find(chunkHandle->hash);

    if(iter!=m_weakChunkHandles.end())
        m_weakChunkHandles.erase(iter);

    //update cache if not already saved

    //we are not releasing the handle here as we hold it in a different map
    //but we want to release the chunk as no one is using it now
    chunkHandle->chunk.reset(nullptr);
    chunkHandle->status=ChunkHandleType::Unknown;
}

template<typename _Chunk>
std::vector<unsigned int> ChunkHandler<_Chunk>::getUpdatedChunks()
{
    std::unique_lock<std::mutex> lock(m_chunkUpdatedMutex);

    std::vector<unsigned int> updatedChunks(m_chunksUpdated);
    m_chunksUpdated.clear();
    lock.unlock();

    return updatedChunks;
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::addReadEvent(SharedChunkHandle chunkHandle)
{
    {
        std::unique_lock<std::mutex> lock(m_ioMutex);

        std::shared_ptr<IOReadRequestType> request=std::make_shared<IOReadRequestType>(IORequestType::Read, chunkHandle);
        
        m_ioQueue.push(request);
    }
    m_ioEvent.notify_all();
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::addWriteEvent(SharedChunkHandle chunkHandle)
{
    {
        std::unique_lock<std::mutex> lock(m_ioMutex);

        std::shared_ptr<IOWriteRequestType> request=std::make_shared<IOWriteRequestType>(IORequestType::Write, chunkHandle);

        m_ioQueue.push(request);
    }
    m_ioEvent.notify_all();
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::addToGenerateQueue(SharedChunkHandle chunkHandle)
{
    std::unique_lock<std::mutex> lock(m_generatorMutex);

    chunkHandle->status=ChunkHandleType::Generating;
    m_generatorQueue.push(chunkHandle);
    m_generatorEvent.notify_all();
}

template<typename _Chunk>
void ChunkHandler<_Chunk>::addToUpdatedQueue(unsigned int hash)
{
    std::unique_lock<std::mutex> lock(m_chunkUpdatedMutex);

    m_chunksUpdated.push_back(hash);
}

} //namespace voxigen

#endif //_voxigen_chunkHandler_h_