#ifndef _voxigen_renderPrepThread_h_
#define _voxigen_renderPrepThread_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/volume/regularGrid.h"
#include "voxigen/search.h"
#include "voxigen/texturing/textureAtlas.h"
//#include "voxigen/nativeGL.h"
#include "voxigen/meshes/chunkTextureMesh.h"
//#include "voxigen/meshBuffer.h"

#include <generic/objectHeap.h>

#include <string>
#include <deque>

namespace voxigen
{

namespace prep
{

enum Type
{
    UpdatePos,
    Mesh,
    ReturnMesh,
    CancelMesh
};

struct Position
{
    glm::ivec3 region;
    glm::ivec3 chunk;
};

template<typename _Object>
struct ObjectMesh
{
    _Object *object;
    voxigen::ChunkTextureMesh *mesh;
    TextureAtlas *textureAtlas;
};

namespace Priority
{
const size_t UpdatePos=10;
const size_t Mesh=25;
const size_t ReturnMesh=15;
const size_t CancelMesh=20;
}
template<typename _Object>
struct Request
{
    typedef ObjectMesh<_Object> ObjectMesh;
    typedef ChunkTextureMesh Mesh;

    Request() {}
    Request(Type type, size_t priority):type(type), priority(priority){}
//    virtual ~Request(){}
//
//    virtual void process()=0;

    Type type;
    size_t priority;

    union Data
    {
        Position position;
        ObjectMesh objectMesh;
    } data;

    ObjectMesh &getObjectMesh() { return data.objectMesh; }
    _Object *getObject() { return data.objectMesh.object; }
    Mesh *getMesh() { return data.objectMesh.mesh; }

    glm::ivec3 &getRegion() { return data.position.region; }
    glm::ivec3 &getChunk() { return data.position.chunk; }
};

}//namespace prep

////////////////////////////////////////////////////////////////////////////////////////
//RenderPrepThread
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _DataType, typename _Object>
class RenderPrepThread
{
public:
    typedef prep::Request<_Object> Request;
    typedef std::vector<Request *> Requests;
    typedef std::deque<Request *> RequestQueue;

    typedef _DataType DataType;
    
//    typedef std::function<void(DataType *)> Initialize;
//    typedef std::function<void(DataType *)> Terminate;

    typedef voxigen::ChunkTextureMesh Mesh;

    RenderPrepThread(size_t requestSize=50);
    ~RenderPrepThread();

    bool requestPositionUpdate(const glm::ivec3 &region, const glm::ivec3 &chunk);
    bool requestMesh(_Object *object, TextureAtlas *textureAtlas);
    bool returnMesh(voxigen::ChunkTextureMesh *mesh);
    bool requestCancelMesh(_Object *object);
    void returnRequest(Request *request);

    void updateQueues(Requests &completedQueue);// ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed);

//    void start(NativeGL *nativeGL);
//    void start(DataType *dataType, Initialize init, Terminate term);
    void start();
    void stop();

    //Do not call directly, the start function will start and call the processThread
    void processThread();

private:
    bool positionUpdate(Request *request, glm::ivec3 &region, glm::ivec3 &chunk);
    bool buildMesh(Request *request, voxigen::ChunkTextureMesh *scratchMesh);
    bool returnMesh(Request *request);
    bool cancelMesh(Request *request, Requests &requests, Requests &canceled);

//    Initialize initialize;
//    Terminate terminate;
//    DataType *m_data;
//    NativeGL *m_nativeGL;
//    unsigned int m_outlineInstanceVertices;

    std::thread m_thread;
    std::condition_variable m_event;

    glm::ivec3 m_currentRegion;
    glm::ivec3 m_currentChunk;

///////////////////////////////////////////////////////
//all this data should be accessed by only one thread, allows cache all request before pushing to thread
    Requests m_requestCache;
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//all this data should be thread safe and only accessed under mutex lock
    std::atomic<int> m_requestAvail;
    std::mutex m_queueMutex;

    bool m_run;
    bool m_updateSearch;
    SearchSettings m_settings;

    Requests m_requestQueue;
    Requests m_completedQueue;
    generic::ObjectHeap<Request> m_requests;
    generic::ObjectHeap<voxigen::ChunkTextureMesh> m_meshes;
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//all this data is owned by the thread an should never be accessed by anything else
///////////////////////////////////////////////////////
};

}//namespace voxigen

#include "renderPrepThread.inl"

#endif //_voxigen_renderPrepThread_h_