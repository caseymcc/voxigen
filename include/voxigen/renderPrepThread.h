#ifndef _voxigen_renderPrepThread_h_
#define _voxigen_renderPrepThread_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/regularGrid.h"
#include "voxigen/search.h"
#include "voxigen/textureAtlas.h"

#include <string>
#include <deque>

#if defined(_WIN32) || defined(_WIN64)
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

    void updateQueues(Requests &completedQueue);// ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed);

#if defined(_WIN32) || defined(_WIN64)
    void start(HDC dc, HGLRC glContext);
#else
    void start(Display *display, GLXDrawable drawable, GLXContext glContext);
#endif
    //stop only happens when there is no work todo
    void stop();
    void processThread();

private:
#if defined(_WIN32) || defined(_WIN64)
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