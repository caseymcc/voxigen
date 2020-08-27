#ifndef _voxigen_regularGrid_h_
#define _voxigen_regularGrid_h_

//#include "voxigen/biome.h"
#include "voxigen/volume/chunk.h"
#include "voxigen/volume/region.h"
#include "voxigen/volume/gridDescriptors.h"
#include "voxigen/generators/generator.h"
#include "voxigen/volume/dataStore.h"
#include "voxigen/entity.h"
#include "voxigen/classFactory.h"
#include "voxigen/updateQueue.h"
#include "voxigen/processRequests.h"
//#include "voxigen/processQueue.h"
#include "voxigen/processingThread.h"

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
//template<typename _Grid, bool grid=false>
//class RegularGridTypes
//{
//public:
//    typedef _Grid GridType;
//    typedef GridDescriptors<GridType> DescriptorType;
//
//    typedef typename GridType::ChunkType ChunkType;
//    typedef ChunkHandle<ChunkType> ChunkHandleType;
//    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
//};

template<typename _Cell, size_t _ChunkSizeX=64, size_t _ChunkSizeY=64, size_t _ChunkSizeZ=64, size_t _RegionSizeX=16, size_t _RegionSizeY=16, size_t _RegionSizeZ=16, bool _Thread=true>
class RegularGrid
{
public:
    RegularGrid();
    ~RegularGrid();

    typedef RegularGrid< _Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ> Type;
    typedef RegularGrid< _Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ> GridType;
    typedef std::integral_constant<size_t, _ChunkSizeX*_RegionSizeX> regionCellSizeX;
    typedef std::integral_constant<size_t, _ChunkSizeY*_RegionSizeY> regionCellSizeY;
    typedef std::integral_constant<size_t, _ChunkSizeZ*_RegionSizeZ> regionCellSizeZ;

    typedef GridDescriptors<GridType> DescriptorType;
    typedef GridDescriptors<GridType> Descriptor;
    typedef _Cell CellType;

    typedef Chunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> ChunkType;
    typedef Chunk<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> Chunk;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    typedef Region<ChunkType, _RegionSizeX, _RegionSizeY, _RegionSizeZ> RegionType;
    typedef Region<ChunkType, _RegionSizeX, _RegionSizeY, _RegionSizeZ> Region;
    typedef RegionHandle<RegionType> RegionHandleType;
    typedef std::shared_ptr<RegionHandleType> SharedRegionHandle;

    typedef process::Request ProcessRequest;
//    typedef ProcessQueue<GridType> ProcessQueueType;
//    typedef typename ProcessQueueType::ProcessRequest ProcessRequest;
//    typedef std::function<bool(SharedChunkHandle)> ChunkUpdateCallback;
//    typedef std::unordered_map<RegionHash, SharedRegion> SharedRegionMap;

//    typedef UniqueChunkMap<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ> UniqueChunkMap;
//    typedef std::shared_ptr<ChunkType> SharedChunk;
//    typedef std::unordered_map<ChunkHash, SharedChunk> SharedChunkMap;

////registers default generators
//    typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<ChunkType>> EquiRectWorldGenerator;

    void create(const std::string &directory, const std::string &name, const glm::ivec3 &size, const std::string &generatorName, LoadProgress &progress);
    bool load(const std::string &directory, LoadProgress &progress);
    bool save();
    bool saveTo(const std::string &directory);
    
//    void setChunkUpdateCallback(ChunkUpdateCallback callback) {chunkUpdateCallback=callback; }
//    bool defaultChunkUpdateCallback() { return false; }

    SharedRegionHandle getRegion(const glm::ivec3 &index);
    SharedRegionHandle getRegion(RegionHash hash);
    bool loadRegion(RegionHandleType *handle, size_t lod, bool force=false);
    bool cancelLoadRegion(RegionHandleType *handle);

    const glm::ivec3 &getRegionCount() { return m_descriptors.getRegionCount(); }
//    RegionHash getRegionHash(const glm::vec3 &gridPosition);

//    bool updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk);

    glm::ivec3 size() const;
    static glm::ivec3 regionCellSize();

    SharedChunkHandle getChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);
    SharedChunkHandle getChunk(RegionHash regionHash, ChunkHash chunkHash);
    SharedChunkHandle getChunk(Key &key);
    bool loadChunk(ChunkHandleType *chunkHandle, size_t lod, bool force=false);
    bool cancelLoadChunk(ChunkHandleType *chunkHandle);
    void releaseChunk(ChunkHandleType *chunkHandle);
    void getUpdated(std::vector<RegionHash> &updatedRegions, std::vector<Key> &updatedChunks, RequestQueue &requests);

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

    int getBaseHeight(const glm::vec2 &pos) { return m_generator->getBaseHeight(pos); }

    Generator &getGenerator() { return *m_generator.get(); }

#ifdef USE_OCTOMAP
    octomap::OcTree<SharedRegionHandle> m_regionTree;
#endif //USE_OCTOMAP

//Sizes
    glm::ivec3 getChunkSize();

    //used to set the number of request (load, read, write, genereate) that can be in flight
    size_t getChunkRequestSize();
    void setChunkRequestSize(size_t size);

    glm::vec3 gridPosToRegionPos(RegionHash regionHash, const glm::vec3 &gridPosition);

    DescriptorType &getDescriptors() { return m_descriptors; }
    const DescriptorType &getDescriptors() const { return m_descriptors; }

    glm::mat4 &getTransform() { return m_transform; }

//    void updateProcessQueue() { m_processQueue.updateQueue(); }
    void processThread();

    bool processRequest(process::Request *request);
    bool processGenerateRegion(process::Request *request);
    bool processGenerate(process::Request *request);
    bool processRead(process::Request *request);
    bool processWrite(process::Request *request);
    bool processUpdate(process::Request *request);
    bool processRelease(process::Request *request);

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
//    ChunkUpdateCallback chunkUpdateCallback;

//    GeneratorQueue<GridType> m_generatorQueue;
    std::unique_ptr<Generator> m_generator;
    DataStore<GridType> m_dataStore;
    UpdateQueue m_updateQueue;

    glm::mat4 m_transform;

    std::thread m_processThread;
    bool m_processThreadRunning;
    std::mutex m_processMutex;
    std::condition_variable m_processEvent;
//    ProcessQueueType m_processQueue;
    
    std::vector<ProcessRequest> m_completeQueue;
};

}//namespace voxigen

#include "voxigen/volume/regularGrid.inl"

#endif //_voxigen_regularGrid_h_