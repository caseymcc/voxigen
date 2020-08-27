#ifndef _voxigen_regionHandle_h_
#define _voxigen_regionHandle_h_

#include "voxigen/volume/chunkHandle.h"
#include "voxigen/volume/dataHandler.h"
#include "voxigen/volume/dataStore.h"
#include "voxigen/volume/gridDescriptors.h"
#include "voxigen/generators/generator.h"
#include "voxigen/fileio/simpleFilesystem.h"

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
#include "voxigen/volume/cell.h"

namespace voxigen
{

template<typename   >
class DataStore;

template<typename _Region>
class RegionHandle:public DataHandler<RegionHash, ChunkHandle<typename _Region::ChunkType>, typename _Region::ChunkType>
{
public:
    typedef _Region RegionType;
    typedef typename RegionType::ChunkType ChunkType;

    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
    
    //DataHandler typdefs
    typedef RegionHash HashType;
    typedef typename DataHandler<RegionHash, ChunkHandle<ChunkType>, ChunkType>::DataHandle DataHandle;

    typedef std::shared_ptr<DataHandle> SharedDataHandle;
    typedef std::unordered_map<RegionHash, SharedDataHandle> SharedDataHandleMap;

    typedef CellHeight Cell;
    typedef std::vector<Cell> Cells;
//    enum Status
//    {
//        Unknown,
//        Loaded
//    };

//    RegionHandle(RegionHash regionHash, GridDescriptors<_Grid> *descriptors, GeneratorQueue<_Grid> *generatorQueue, DataStore<_Grid> *dataStore, UpdateQueue *updateQueue);
    RegionHandle(RegionHash regionHash, IGridDescriptors *descriptors);

    const glm::ivec3 &getRegionIndex() { return m_index; }

    void generate(IGridDescriptors *descriptors, Generator *generator, size_t lod);
    void release();

    SharedChunkHandle getChunk(RegionHash chunkHash);
//    void loadChunk(SharedChunkHandle chunkHandle, size_t lod, bool force=false);
//    void cancelLoadChunk(SharedChunkHandle chunkHandle);

//    Status getStatus() { return m_status; }
    HandleState state() { return m_state; }
    void setState(HandleState state) { m_state=state; }

    HandleAction action() { return m_action; }
    void setAction(HandleAction action) { m_action=action; }

    bool load(const std::string &name);

    void addConfig(ChunkHandleType *handle);

    RegionHash hash() { return m_hash; }
    bool cachedOnDisk() { return m_cachedOnDisk; }
    void setCachedOnDisk(bool cached) { m_cachedOnDisk=cached; }
    bool empty() { return m_empty; }
    void setEmpty(bool empty) { m_empty=empty; }

    const size_t &claims() const { return m_dataClaims; }
    void addClaim() { m_dataClaims++; }
    void releaseClaim() { m_dataClaims--; }

    const Cells &getHeightMap() const { return m_heightMap; }
    size_t getHeighMapLod() const { return m_heightMapLod; }

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

    IGridDescriptors *m_descriptors;
//    DataStore<_Grid> *m_dataStore;
//    GeneratorQueue<_Grid> *m_generatorQueue;
    UpdateQueue *m_updateQueue;

    RegionHash m_hash;
    glm::ivec3 m_index;

    bool m_cachedOnDisk;
    bool m_empty;

//    Status m_status;
    HandleState m_state;
    HandleAction m_action;
    unsigned int m_version;
    std::string m_directory;
    std::string m_configFile;

    Cells m_heightMap;
    size_t m_heightMapLod;

    size_t m_dataClaims;

    //rapidjson::Document m_configDocument;
};

} //namespace voxigen

#include "voxigen/volume/regionHandle.inl"

#endif //_voxigen_regionHandle_h_