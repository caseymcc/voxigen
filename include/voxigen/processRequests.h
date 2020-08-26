#ifndef _voxigen_processRequests_h_
#define _voxigen_processRequests_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/volume/gridFunctions.h"

#include "voxigen/meshes/chunkTextureMesh.h"

#include <glm/glm.hpp>

#include <functional>

namespace voxigen
{

namespace process
{

enum Type
{
    UpdatePos,
    GenerateRegion,
    CancelGenerateRegion,
    Generate,
    CancelGenerate,
    Read,
    CancelRead,
    Write,
    CancelWrite,
    Mesh,
    CancelMesh,
    MeshReturn
};

namespace Priority
{
const size_t CancelRead=10;
const size_t CancelWrite=10;
const size_t CancelGenerate=10;
const size_t CancelMesh=10;
const size_t MeshReturn=10;

const size_t UpdatePos=15;

const size_t Read=25;
const size_t Generate=25;
const size_t Mesh=25;

const size_t Write=50;
}

namespace Result
{
const size_t Success=0;
const size_t Canceled=1;
}

struct Position
{
    glm::ivec3 region;
    glm::ivec3 chunk;
};

//struct ObjectMesh
//{
//    void *object;
//    voxigen::ChunkTextureMesh *mesh;
//    TextureAtlas *textureAtlas;
//};

struct Region
{
    void *handle;
    size_t lod;
};

struct Chunk
{
    void *handle;
    size_t lod;
};

struct BuildMesh
{
    void *renderer;
//    ChunkTextureMesh *mesh;
    void *mesh;
//    std::any mesh;
};

struct Request
{
    Request() {}
    Request(Type type, size_t priority):type(type), priority(priority){}

    Type type;
    size_t priority;
    Position position;
    size_t result;

    union Data
    {
        Region region;
        Chunk chunk;
        BuildMesh buildMesh;
    } data;

    glm::ivec3 &getRegion() { return position.region; }
    glm::ivec3 &getChunk() { return position.chunk; }

    float distance(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex) const
    {
        return details::distance(regionIndex, chunkIndex, position.region, position.chunk, regionSize, chunkSize);
    }

    //TODO: currently a hack to make it work
    static glm::ivec3 regionSize;
    static glm::ivec3 chunkSize;
};

typedef std::function<bool(Request *)> Callback;

struct Compare
{
    bool operator()(const process::Request *request1, const process::Request *request2) const
    {
        if(request1->priority!=request2->priority)
            return (request1->priority>request2->priority);
        else
        {
            //really only need to calculate this per player update
            float handle1Distance=request1->distance(currentRegion, currentChunk);
            float handle2Distance=request2->distance(currentRegion, currentChunk);

#ifdef LOG_PROCESS_QUEUE
            request1->distanceValue=handle1Distance;
            request2->distanceValue=handle2Distance;
#endif
            return (handle1Distance>handle2Distance);
        }
    }

//    static IGridDescriptors *descriptor;
    static glm::ivec3 currentRegion;
    static glm::ivec3 currentChunk;
};

}//namespace process

typedef std::vector<process::Request *> RequestQueue;

}//namespace voxigen

#endif //_voxigen_processRequests_h_