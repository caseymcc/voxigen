#ifndef _voxigen_renderPrepThread_h_
#define _voxigen_renderPrepThread_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/regularGrid.h"
#include "voxigen/search.h"
#include "voxigen/textureAtlas.h"

#include <string>
#include <deque>

#ifdef _WINDOWS
#include "windows.h"
#else
#include <GL/glx.h>
#endif

namespace voxigen
{

namespace prep
{

enum Type
{
    Add,
    Remove,
    Mesh,
    ReleaseMesh
};

struct Request
{
    Request(Type type):type(type) {}

    Type type;
};

template<typename _ChunkHandle, typename _ChunkRenderer>
struct RequestAdd:public Request
{
    RequestAdd(_ChunkHandle chunkHandle):Request(Add), chunkHandle(chunkHandle){}

//input
    _ChunkHandle chunkHandle;

//output
    _ChunkRenderer *renderer;
};

template<typename _ChunkRenderer>
struct RequestRemove:public Request
{
    RequestRemove(_ChunkRenderer *renderer):Request(Remove), renderer(renderer){}

//input
    _ChunkRenderer *renderer;
    
};

template<typename _ChunkRenderer>
struct RequestMesh:public Request
{
    RequestMesh(_ChunkRenderer *renderer, TextureAtlas *textureAtlas):Request(Mesh), renderer(renderer), textureAtlas(textureAtlas) {}

//input
    _ChunkRenderer *renderer;
    TextureAtlas *textureAtlas;

//output
    MeshBuffer mesh;
};

struct RequestReleaseMesh:public Request
{
    RequestReleaseMesh(MeshBuffer &mesh):Request(ReleaseMesh), mesh(mesh){}

    //input
    MeshBuffer mesh;
};

}//namespace prep

/////////////////////////////////////////////////////////////////////////////////////////
//RenderPrepThread
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Grid, typename _ChunkRenderer>
class RenderPrepThread//:public RegularGridTypes<_Grid>
{
public:
    typedef typename _Grid::GridType GridType;
    typedef typename _Grid::DescriptorType DescriptorType;
    typedef typename _Grid::ChunkType ChunkType;
    typedef typename _Grid::ChunkHandleType ChunkHandleType;
    typedef typename _Grid::SharedChunkHandle SharedChunkHandle;

    typedef std::vector<SharedChunkHandle> SharedChunkHandles;
    typedef std::deque<SharedChunkHandle> SharedChunkHandleQueue;

    typedef _ChunkRenderer ChunkRendererType;
    typedef std::vector<ChunkRendererType *> ChunkRenderers;
    typedef std::deque<ChunkRendererType *> ChunkRendererQueue;

    typedef std::vector<prep::Request *> Requests;
    typedef std::deque<prep::Request *> RequestQueue;

    typedef prep::Request Request;
//    typedef prep::RequestAdd<SharedChunkHandle, _ChunkRenderer> RequestAdd;
//    typedef prep::RequestRemove<_ChunkRenderer> RequestRemove;
    typedef prep::RequestMesh<_ChunkRenderer> RequestMesh;
    typedef prep::RequestReleaseMesh RequestReleaseMesh;

    RenderPrepThread();
    ~RenderPrepThread();

    void requestSearchUpdate();
    void requestSearchUpdate(const SearchSettings &settings);
//    void add(const ChunkRenderers &chunkRenderers);
//    void requestRenderer(SharedChunkHandle &chunkHandle);
//    void requestRemove(ChunkRendererType *chunkRenderer);
    void requestMesh(ChunkRendererType *chunkRenderer, TextureAtlas *textureAtlas);
    void requestReleaseMesh(MeshBuffer &mesh);

//    bool hasUpdates(); //this reads a non mutex'd variable so it is undefined behavior
//    void getUpdates(ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed);
//    void getAdded(const ChunkRenderers &chunkRenderers);
//    void getUpdated(const ChunkRenderers &chunkRenderers);
//    void getRemoved(const ChunkRenderers &chunkRenderers);

    void updateQueues(Requests &completedQueue);// ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed);

#ifdef _WINDOWS
    void start(HDC dc, HGLRC glContext);
#else
    void start(Display *display, GLXDrawable drawable, GLXContext glContext);
#endif
    //stop only happens when there is no work todo
    void stop();
    void processThread();

private:
//    void rendererUpdateChunks();
//    void updateChunks(ChunkRenderers &addRenderers, ChunkRenderers &updateRenderers, ChunkRenderers &removeRenderers);
//    void updateOcclusionQueries();
//    void processAdd(RequestAdd *request);
//    void processRemove(RequestRemove *request);
    void processMesh(RequestMesh *request);
    void processReleaseMesh(RequestReleaseMesh *request);

#ifdef _WINDOWS
    HDC m_dc;
    HGLRC m_glContext;
#else
    Display *m_display;
    GLXDrawable m_drawable;
    GLXContext m_glContext;

#endif
    unsigned int m_outlineInstanceVertices;

    std::thread m_thread;
    std::condition_variable m_event;

///////////////////////////////////////////////////////
//all this data should be accessed by only one thread, allows cache all request before pushing to thread
//    SharedChunkHandles m_addCache;
//    ChunkRenderers m_updateCache;
//    ChunkRenderers m_removeCache;

    Requests m_requestCache;
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//all this data should be thread safe and only accessed under mutex lock
    std::mutex m_queueMutex;

    bool m_run;
    bool m_updateSearch;
    SearchSettings m_settings;
//    SharedChunkHandles m_addQueue;
//    ChunkRenderers m_updateQueue;
//    ChunkRenderers m_removeQueue;
//
//    ChunkRenderers m_addedQueue;
//    ChunkRenderers m_updatedQueue;
//    ChunkRenderers m_removedQueue;

    Requests m_requestQueue;
    Requests m_completedQueue;
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//all this data is owned by the thread an should never be acessed by anything else
//    SharedChunkHandleQueue m_threadOwned_addQueue;
//    ChunkRendererQueue m_threadOwned_updateQueue;
//    ChunkRenderers m_threadOwned_removeQueue;
//
//    void updateOutputQueues();
//
//    ChunkRenderers m_threadOwned_addedQueue;
//    ChunkRenderers m_threadOwned_updatedQueue;
//    ChunkRenderers m_threadOwned_removedQueue;
//
//    ChunkRendererQueue m_threadOwned_requestQueue;
//    ChunkRenderers m_threadOwned_completedQueue;

//    ChunkRenderers m_chunkRenderers; //all renderers in use
//    ChunkRendererType *getFreeRenderer();
//    ChunkRenderers m_freeChunkRenderers; //renderers available for re-use

//    typedef std::unordered_map<Key::Type, ChunkRendererType *> ChunkRenderMap;
//    ChunkRenderMap m_chunkRendererMap; 

    //temporary mesh used for generating
    ChunkTextureMesh m_mesh;
///////////////////////////////////////////////////////
};

}//namespace voxigen

#include "renderPrepThread.inl"

#endif //_voxigen_renderPrepThread_h_