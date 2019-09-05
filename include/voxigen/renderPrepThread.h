#ifndef _voxigen_renderPrepThread_h_
#define _voxigen_renderPrepThread_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/regularGrid.h"
#include "voxigen/search.h"
#include "voxigen/textureAtlas.h"
#include "voxigen/nativeGL.h"
#include "voxigen/chunkTextureMesh.h"
#include "voxigen/meshBuffer.h"

#include <string>
#include <deque>

namespace voxigen
{

namespace prep
{

enum Type
{
    Mesh,
    ReleaseMesh
};

struct Request
{
    Request(Type type):type(type) {}
    virtual ~Request(){}

    virtual void process()=0;

    Type type;
};

template<typename _Grid, typename _Renderer>
struct RequestMesh:public Request
{
    RequestMesh(_Renderer *renderer, TextureAtlas *textureAtlas):Request(Mesh), renderer(renderer), textureAtlas(textureAtlas) {}
    virtual ~RequestMesh() {}

    void process() override { assert(false); }

//input
    _Renderer *renderer;
    TextureAtlas *textureAtlas;

//output
    MeshBuffer mesh;
};

struct RequestReleaseMesh:public Request
{
    RequestReleaseMesh(MeshBuffer &mesh):Request(ReleaseMesh), mesh(mesh){}
    virtual ~RequestReleaseMesh() {}

    void process() override;

    //input
    MeshBuffer mesh;
};

}//namespace prep

/////////////////////////////////////////////////////////////////////////////////////////
//RenderPrepThread
/////////////////////////////////////////////////////////////////////////////////////////
class RenderPrepThread
{
public:
    typedef prep::Request Request;
    typedef std::vector<Request *> Requests;
    typedef std::deque<Request *> RequestQueue;

    RenderPrepThread();
    ~RenderPrepThread();

    void requestSearchUpdate();
    void requestSearchUpdate(const SearchSettings &settings);

    template<typename _Grid, typename _Renderer>
    void requestMesh(_Renderer *chunkRenderer, TextureAtlas *textureAtlas);
    
    void requestReleaseMesh(MeshBuffer &mesh);
    void releaseMesh(MeshBuffer &mesh) { requestReleaseMesh(mesh); }

    void updateQueues(Requests &completedQueue);// ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed);

    void start(NativeGL *nativeGL);

    //stop only happens when there is no work todo
    void stop();
    void processThread();

private:
    NativeGL *m_nativeGL;
    unsigned int m_outlineInstanceVertices;

    std::thread m_thread;
    std::condition_variable m_event;

///////////////////////////////////////////////////////
//all this data should be accessed by only one thread, allows cache all request before pushing to thread
    Requests m_requestCache;
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//all this data should be thread safe and only accessed under mutex lock
    std::mutex m_queueMutex;

    bool m_run;
    bool m_updateSearch;
    SearchSettings m_settings;

    Requests m_requestQueue;
    Requests m_completedQueue;
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//all this data is owned by the thread an should never be accessed by anything else
///////////////////////////////////////////////////////
};

}//namespace voxigen

#include "renderPrepThread.inl"

#endif //_voxigen_renderPrepThread_h_