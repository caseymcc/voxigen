#ifndef _voxigen_regionHandle_h_
#define _voxigen_regionHandle_h_

#include "voxigen/chunkHandle.h"
#include "voxigen/dataHandler.h"
#include "voxigen/dataStore.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/generator.h"
#include "voxigen/simpleFilesystem.h"

#include <generic/jsonSerializer.h>

#ifdef USE_OCTOMAP
#include "octomap/OcTree.h"
#endif //USE_OCTOMAP

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

template<typename _Grid>
class DataStore;

template<typename _Grid>
class RegionHandle:public DataHandler<RegionHash, ChunkHandle<typename _Grid::ChunkType>, typename _Grid::ChunkType>
{
public:
    typedef typename _Grid::RegionType RegionType;
    typedef typename RegionType::ChunkType ChunkType;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
    
    //DataHandler typdefs
    typedef RegionHash HashType;
    typedef typename DataHandler<RegionHash, ChunkHandle<typename _Grid::ChunkType>, typename _Grid::ChunkType>::DataHandle DataHandle;

    typedef std::shared_ptr<DataHandle> SharedDataHandle;
    typedef std::unordered_map<RegionHash, SharedDataHandle> SharedDataHandleMap;

    enum Status
    {
        Unknown,
        Loaded
    };

    RegionHandle(RegionHash regionHash, GridDescriptors<_Grid> *descriptors, GeneratorQueue<_Grid> *generatorQueue, DataStore<_Grid> *dataStore, UpdateQueue *updateQueue);

    SharedChunkHandle getChunk(RegionHash chunkHash);
    void loadChunk(SharedChunkHandle chunkHandle, size_t lod, bool force=false);
    void cancelLoadChunk(SharedChunkHandle chunkHandle);

    Status getStatus() { return m_status; }

    bool load(const std::string &name);
    void release() {}

    void addConfig(SharedChunkHandle handle);

    RegionHash hash() { return m_hash; }
    bool cachedOnDisk() { return m_cachedOnDisk; }
    void setCachedOnDisk(bool cached) { m_cachedOnDisk=cached; }
    bool empty() { return m_empty; }
    void setEmpty(bool empty) { m_empty=empty; }

#ifdef USE_OCTOMAP
    octomap::OcTree<SharedChunkHandle> m_chunkTree;
#endif //USE_OCTOMAP

protected:
    DataHandle *newHandle(HashType hash) override;

private:
    void loadConfig(); 
    void saveConfig();
    void saveConfigTo(std::string configFile);
    void loadDataStore();
    void verifyDirectory();

    GridDescriptors<_Grid> *m_descriptors;
    DataStore<_Grid> *m_dataStore;
    GeneratorQueue<_Grid> *m_generatorQueue;
    UpdateQueue *m_updateQueue;

    RegionHash m_hash;
    bool m_cachedOnDisk;
    bool m_empty;

    Status m_status;
    unsigned int m_version;
    std::string m_directory;
    std::string m_configFile;

    //rapidjson::Document m_configDocument;
};

template<typename _Grid>
RegionHandle<_Grid>::RegionHandle(RegionHash regionHash, GridDescriptors<_Grid> *descriptors, GeneratorQueue<_Grid> *generatorQueue, DataStore<_Grid> *dataStore, UpdateQueue *updateQueue):
m_status(Unknown),
m_version(0),
m_hash(regionHash),
m_descriptors(descriptors),
m_generatorQueue(generatorQueue),
m_dataStore(dataStore),
m_updateQueue(updateQueue),
m_cachedOnDisk(false),
m_empty(false)
{
}

template<typename _Grid>
bool RegionHandle<_Grid>::load(const std::string &directory)
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

template<typename _Grid>
typename RegionHandle<_Grid>::DataHandle *RegionHandle<_Grid>::newHandle(HashType chunkHash)
{
    glm::ivec3 regionIndex=m_descriptors->getRegionIndex(m_hash);
    glm::ivec3 chunkIndex=m_descriptors->getChunkIndex(chunkHash);

    return new ChunkHandleType(m_hash, regionIndex, chunkHash, chunkIndex);
}

template<typename _Grid>
void RegionHandle<_Grid>::loadConfig()
{
    generic::JsonDeserializer deserializer;

    deserializer.open(m_configFile.c_str());

    deserializer.openObject();
    if(deserializer.key("version"))
        m_version=deserializer.getUInt();

    if(deserializer.key("chunks"))
    {
        if(deserializer.openArray())
        {
            do
            {
                if(deserializer.openObject())
                {
                    if(deserializer.key("id"))
                    {
                        RegionHash hash=deserializer.getUInt();
                        SharedChunkHandle chunkHandle(newHandle(hash));

                        chunkHandle->setCachedOnDisk(true);

                        if(deserializer.key("empty"))
                            chunkHandle->setEmpty(deserializer.getBool());
                        else
                            chunkHandle->setEmpty(true);

                        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(hash, chunkHandle));
                    }

                    deserializer.closeObject();
                }
            } while(deserializer.advance());
            deserializer.closeArray();
        }
    }
    deserializer.closeObject();
}

template<typename _Grid>
void RegionHandle<_Grid>::saveConfig()
{
    saveConfigTo(m_configFile);
}

template<typename _Grid>
void RegionHandle<_Grid>::saveConfigTo(std::string configFile)
{
    generic::JsonSerializer serializer;

    serializer.open(m_configFile.c_str());

    serializer.startObject();

    serializer.addKey("version");
    serializer.addInt(m_version);

    serializer.addKey("chunks");
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
void RegionHandle<_Grid>::addConfig(SharedChunkHandle handle)
{
    if(handle->empty())
    {
        std::string tempConfig=m_configFile+".tmp";
        saveConfigTo(tempConfig);
        fs::copy_file(tempConfig, m_configFile, true);
    }
}

template<typename _Grid>
void RegionHandle<_Grid>::loadDataStore()
{
    std::vector<std::string> directories=fs::get_directories(m_directory);

    for(auto &entry:directories)
    {
        std::istringstream fileNameStream(entry);
        ChunkHash chunkHash;

        fileNameStream.ignore(6);
        fileNameStream>>std::hex>>chunkHash;

        glm::ivec3 regionIndex=m_descriptors->getRegionIndex(m_hash);
        glm::ivec3 chunkIndex=m_descriptors->getChunkIndex(chunkHash);

        SharedChunkHandle handle=std::make_shared<ChunkHandleType>(m_hash, regionIndex, chunkHash, chunkIndex);

        handle->setCachedOnDisk(true);
        handle->setEmpty(false);

        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(chunkHash, handle));
    }
}

template<typename _Grid>
void RegionHandle<_Grid>::verifyDirectory()
{

}

template<typename _Grid>
typename RegionHandle<_Grid>::SharedChunkHandle RegionHandle<_Grid>::getChunk(ChunkHash chunkHash)
{
    SharedChunkHandle chunkHandle=this->getDataHandle(chunkHash);

    glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHash);
    chunkHandle->setRegionOffset(glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex);


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

template<typename _Grid>
void RegionHandle<_Grid>::loadChunk(SharedChunkHandle chunkHandle, size_t lod, bool force)
{
    if(force || (chunkHandle->state() != ChunkState::Memory))
    {
        //an action is in process lets not start something else as well
        if(chunkHandle->action()!=ChunkAction::Idle)
            return;

        if(chunkHandle->empty()) //empty is already loaded
        {
//            m_dataStore->empty(chunkHandle);
        }
        else
        {
            //we dont have it in memory so we need to load or generate it
            if(!chunkHandle->cachedOnDisk()) 
                m_dataStore->generate(chunkHandle, lod);
            else
                m_dataStore->read(chunkHandle, lod);
        }
    }
}

template<typename _Grid>
void RegionHandle<_Grid>::cancelLoadChunk(SharedChunkHandle chunkHandle)
{
    //if we are not doing anything ignore
    if(chunkHandle->action()==ChunkAction::Idle)
        return;
    
    m_dataStore->cancel(chunkHandle);
}

} //namespace voxigen

#endif //_voxigen_regionHandle_h_