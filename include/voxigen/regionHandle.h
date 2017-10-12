#ifndef _voxigen_regionHandle_h_
#define _voxigen_regionHandle_h_

#include "voxigen/chunkHandle.h"
#include "voxigen/dataHandler.h"
#include "voxigen/dataStore.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"

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

template<typename _Region, typename _Chunk>
class DataStore;

template<typename _Region>
class RegionHandle:public DataHandler<ChunkHash, ChunkHandle<typename _Region::ChunkType>, typename _Region::ChunkType>
{
public:
    typedef _Region RegionType;
    typedef typename RegionType::ChunkType ChunkType;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
    
    enum Status
    {
        Unknown,
        Loaded
    };

    RegionHandle(RegionHash regionHash, GridDescriptors *descriptors, GeneratorQueue<ChunkType> *generatorQueue, DataStore<RegionType, ChunkType> *dataStore, UpdateQueue *updateQueue);

    SharedChunkHandle getChunk(ChunkHash chunkHash);
    void loadChunk(SharedChunkHandle chunkHandle, size_t lod);

    Status getStatus() { return m_status; }

    bool load(const std::string &name);
    void release() {}

    void addConfig(SharedChunkHandle handle);

    RegionHash hash;
    bool cachedOnDisk;
    bool empty;

protected:
    virtual DataHandle *newHandle(HashType hash);

private:
    void loadConfig(); 
    void saveConfig();
    void loadDataStore();
    void verifyDirectory();

    GridDescriptors *m_descriptors;
    DataStore<_Region, typename _Region::ChunkType> *m_dataStore;
    GeneratorQueue<ChunkType> *m_generatorQueue;
    UpdateQueue *m_updateQueue;

    Status m_status;
    unsigned int m_version;
    std::string m_directory;
    std::string m_configFile;

    rapidjson::Document m_configDocument;
};

template<typename _Region>
RegionHandle<_Region>::RegionHandle(RegionHash regionHash, GridDescriptors *descriptors, GeneratorQueue<ChunkType> *generatorQueue, DataStore<RegionType, ChunkType> *dataStore, UpdateQueue *updateQueue):
m_status(Unknown),
m_version(0),
hash(regionHash),
m_descriptors(descriptors),
m_generatorQueue(generatorQueue),
m_dataStore(dataStore),
m_updateQueue(updateQueue),
cachedOnDisk(false),
empty(false)
{
}

template<typename _Region>
bool RegionHandle<_Region>::load(const std::string &directory)
{
    m_directory=directory;
    fs::path path(directory);

    if(!fs::is_directory(path))
    {
        if(fs::exists(path))
            return false;

        fs::create_directory(path);
    }

    m_configFile=m_directory+"/regionConfig.json";
    fs::path configPath(m_configFile);

    if(!fs::exists(configPath))
        saveConfig();
    else
        loadConfig();

    loadDataStore();
    verifyDirectory();

    m_status=Loaded;

    return true;
}

template<typename _Region>
typename RegionHandle<_Region>::DataHandle *RegionHandle<_Region>::newHandle(HashType chunkHash)
{
    return new ChunkHandleType(hash, chunkHash);
}

template<typename _Region>
void RegionHandle<_Region>::loadConfig()
{
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
        const rapidjson::Value &chunkValue=chunks[i];

        ChunkHash hash=chunkValue["id"].GetUint();
        SharedChunkHandle chunkHandle(newHandle(hash));

        chunkHandle->cachedOnDisk=true;
        chunkHandle->empty=chunkValue["empty"].GetBool();

        m_dataHandles.insert(SharedDataHandleMap::value_type(hash, chunkHandle));
    }

    fclose(filePtr);
}

template<typename _Region>
void RegionHandle<_Region>::saveConfig()
{
    FILE *filePtr=fopen(m_configFile.c_str(), "wb");
    char writeBuffer[65536];

    rapidjson::FileWriteStream fileStream(filePtr, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fileStream);

    writer.StartObject();

    writer.Key("version");
    writer.Uint(m_version);

    writer.Key("chunks");
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

template<typename _Region>
void RegionHandle<_Region>::addConfig(SharedChunkHandle handle)
{
    //TODO - fix
    //lazy programming for the moment, see remarks in ioThread below
    if(handle->empty)
    {
        rapidjson::Document::AllocatorType &allocator=m_configDocument.GetAllocator();
        rapidjson::Value &chunks=m_configDocument["chunks"];
        assert(chunks.IsArray());

        rapidjson::Value chunk(rapidjson::kObjectType);

        chunk.AddMember("id", handle->hash, allocator);
        chunk.AddMember("empty", handle->empty, allocator);

        chunks.PushBack(chunk, allocator);

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

template<typename _Region>
void RegionHandle<_Region>::loadDataStore()
{
    fs::path path(m_directory);

    for(auto &entry:fs::directory_iterator(path))
    {
        if(fs::is_directory(entry.path()))
        {
            if(entry.path().extension().string()!=".chk")
                continue;

            std::istringstream fileNameStream(entry.path().stem().string());
            ChunkHash chunkHash;

            fileNameStream.ignore(6);
            fileNameStream>>std::hex>>chunkHash;

            SharedChunkHandle handle=std::make_shared<ChunkHandleType>(hash, chunkHash);

            handle->cachedOnDisk=true;
            handle->empty=false;

            m_dataHandles.insert(SharedDataHandleMap::value_type(chunkHash, handle));
        }
    }
}

template<typename _Region>
void RegionHandle<_Region>::verifyDirectory()
{

}

template<typename _Region>
typename RegionHandle<_Region>::SharedChunkHandle RegionHandle<_Region>::getChunk(ChunkHash chunkHash)
{
    SharedChunkHandle chunkHandle=getDataHandle(chunkHash);

    glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHash);
    chunkHandle->regionOffset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;


//    if(chunkHandle->status!=ChunkHandleType::Memory)
//    {
//        if(chunkHandle->empty) //empty is already loaded
//        {
//            chunkHandle->status=ChunkHandleType::Memory;
//            m_updateQueue->add(Key(hash, chunkHash));
//        }
//        else
//        {
//            //we dont have it in memory so we need to load or generate it
//            if(!chunkHandle->cachedOnDisk) 
//                m_generatorQueue->add(chunkHandle);
//            else
//                m_dataStore->read(chunkHandle);
//        }
//    }

    return chunkHandle;
}

template<typename _Region>
void RegionHandle<_Region>::loadChunk(SharedChunkHandle chunkHandle, size_t lod)
{
    if(chunkHandle->status!=ChunkHandleType::Memory)
    {
        if(chunkHandle->empty) //empty is already loaded
        {
            chunkHandle->status=ChunkHandleType::Memory;
            m_updateQueue->add(Key(chunkHandle->regionHash, chunkHandle->hash));
        }
        else
        {
            //we dont have it in memory so we need to load or generate it
            if(!chunkHandle->cachedOnDisk) 
                m_generatorQueue->add(chunkHandle);
            else
                m_dataStore->read(chunkHandle);
        }
    }
}

} //namespace voxigen

#endif //_voxigen_regionHandle_h_