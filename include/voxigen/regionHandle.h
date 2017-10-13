#ifndef _voxigen_regionHandle_h_
#define _voxigen_regionHandle_h_

#include "voxigen/chunkHandle.h"
#include "voxigen/dataHandler.h"
#include "voxigen/dataStore.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/simpleFilesystem.h"

#include <thread>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <queue>
//#include <boost/filesystem.hpp>
#include <sstream>
#include <iomanip>

//#include <rapidjson/prettywriter.h>
//#include <rapidjson/filewritestream.h>
//#include <rapidjson/filereadstream.h>
//#include <rapidjson/document.h>

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
    void saveConfigTo(std::string configFile);
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

    //rapidjson::Document m_configDocument;
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

    if(!fs::is_directory(directory))
    {
        if(fs::exists(directory))
            return false;

        fs::create_directory(directory);
    }

    m_configFile=m_directory+"/regionConfig.json";

    if(!fs::exists(m_configFile))
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
    JsonUnserializer serializer;

    serializer.open(m_configFile.c_str());

    serializer.openObject();
    if(serializer.key("version"))
        m_version=serializer.getUInt();

    if(serializer.key("chunks"))
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
                        SharedChunkHandle chunkHandle(newHandle(hash));

                        chunkHandle->cachedOnDisk=true;

                        if(serializer.key("empty"))
                            chunkHandle->empty=serializer.getBool();
                        else
                            chunkHandle->empty=true;

                        m_dataHandles.insert(SharedDataHandleMap::value_type(hash, chunkHandle));
                    }

                    serializer.closeObject();
                }
            } while(serializer.advance());
            serializer.closeArray();
        }
    }
    serializer.closeObject();
}

template<typename _Region>
void RegionHandle<_Region>::saveConfig()
{
    saveConfigTo(m_configFile);
}

template<typename _Region>
void RegionHandle<_Region>::saveConfigTo(std::string configFile)
{
    JsonSerializer serializer;

    serializer.open(m_configFile.c_str());

    serializer.startObject();

    serializer.addKey("version");
    serializer.addInt(m_version);

    serializer.addKey("chunks");
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

template<typename _Region>
void RegionHandle<_Region>::addConfig(SharedChunkHandle handle)
{
    if(handle->empty)
    {
        std::string tempConfig=m_configFile+".tmp";
        saveConfigTo(tempConfig);
        fs::copy_file(tempConfig, m_configFile, true);
    }
}

template<typename _Region>
void RegionHandle<_Region>::loadDataStore()
{
    std::vector<std::string> directories=fs::get_directories(m_directory);

    for(auto &entry:directories)
    {
        std::istringstream fileNameStream(entry);
        ChunkHash chunkHash;

        fileNameStream.ignore(6);
        fileNameStream>>std::hex>>chunkHash;

        SharedChunkHandle handle=std::make_shared<ChunkHandleType>(hash, chunkHash);

        handle->cachedOnDisk=true;
        handle->empty=false;

        m_dataHandles.insert(SharedDataHandleMap::value_type(chunkHash, handle));
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