#include "voxigen/chunkFunctions.h"
#include "voxigen/search.h"

#include <glog/logging.h>

#include <glbinding/gl/gl.h>

namespace voxigen
{
using namespace gl;

inline void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
{
#ifndef NDEBUG
    if(type==GL_DEBUG_TYPE_ERROR)
        LOG(INFO)<<"Opengl - error: "<<message;
//    else
//        LOG(INFO)<<"Opengl : "<<message;
#endif
}


inline RenderPrepThread::RenderPrepThread()
{
}


inline RenderPrepThread::~RenderPrepThread()
{

}

inline void RenderPrepThread::requestSearchUpdate()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);

    m_updateSearch=true;
}


inline void RenderPrepThread::requestSearchUpdate(const SearchSettings &settings)
{
    std::unique_lock<std::mutex> lock(m_queueMutex);

    m_settings=settings;
    m_updateSearch=true;
}

template<typename _Grid, typename _Renderer>
void RenderPrepThread::requestMesh(_Renderer *renderer, TextureAtlas *textureAtlas)
{
#ifdef DEBUG_MESH
    auto chunkHandle=chunkRenderer->getChunkHandle();
    
    assert(!chunkHandle->empty());
    assert(chunkHandle->action() == HandleAction::Idle);

    LOG(INFO)<<"Mainthread - ChunkRenderer "<<chunkRenderer<<"("<<chunkRenderer->getRegionHash()<<", "<<chunkRenderer->getChunkHash()<<") requestMesh";
#endif//DEBUG_MESH
    prep::RequestMesh<_Grid, _Renderer> *request=new prep::RequestMesh<_Grid, _Renderer>(renderer, textureAtlas);

    m_requestCache.push_back(request);
    //    m_updateCache.push_back(chunkRenderer);
}


inline void RenderPrepThread::requestReleaseMesh(MeshBuffer &mesh)
{
    prep::RequestReleaseMesh *request=new prep::RequestReleaseMesh(mesh);

    m_requestCache.push_back(request);
}

inline void RenderPrepThread::updateQueues(Requests &completedQueue)//ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed)
{
    bool update=false;

    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        if(!m_requestCache.empty())
        {
            m_requestQueue.insert(m_requestQueue.end(), m_requestCache.begin(), m_requestCache.end());
            m_requestCache.clear();
            update=true;

        }

        if(!m_completedQueue.empty())
        {
            completedQueue.insert(completedQueue.end(), m_completedQueue.begin(), m_completedQueue.end());
            m_completedQueue.clear();
        }
    }

    if(update)
        m_event.notify_all();
}

inline void RenderPrepThread::start(NativeGL *nativeGL)
{
    m_nativeGL=nativeGL;
    m_run=true;
    m_thread=std::thread(std::bind(&RenderPrepThread::processThread, this));
}


inline void RenderPrepThread::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=false;
    }

    m_event.notify_all();
    m_thread.join();
}


inline void RenderPrepThread::processThread()
{
    m_nativeGL->makeCurrent();

#ifndef NDEBUG
    glDebugMessageCallback(debugMessage, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
#endif

    bool run=true;
    bool updateSearch=true;

    bool searchSet=false;
    SearchSettings settings;

    size_t index=0;
    size_t processBeforeUpdate=15;

    RequestQueue requestQueue;
    Requests completedQueue;

    while(run)
    {
        if(requestQueue.empty() || index>processBeforeUpdate)
        {
            index=0;

            std::unique_lock<std::mutex> lock(m_queueMutex);

            //update running
            run=m_run;
            if(m_updateSearch)
            {
                searchSet=true;
                updateSearch=m_updateSearch;
                settings=m_settings;
                m_updateSearch=false;
            }

            //check if any new request have been added.
            if(!m_requestQueue.empty())
            {
                requestQueue.insert(requestQueue.end(), m_requestQueue.begin(), m_requestQueue.end());
                m_requestQueue.clear();
            }

            //while we have the lock update anything that is complete
            if(!completedQueue.empty())
            {
                m_completedQueue.insert(m_completedQueue.end(), completedQueue.begin(), completedQueue.end());
                completedQueue.clear();
            }

            if(run && requestQueue.empty())
                m_event.wait(lock);
            
            continue;
        }

        index++;

        prep::Request *request=requestQueue.front();

        requestQueue.pop_front();
        request->process();
        completedQueue.push_back(request);
    }
}

namespace prep
{

inline void RequestReleaseMesh::process()
{
    //likely better to store these and re-use
    glDeleteBuffers(1, &mesh.vertexBuffer);
    glDeleteBuffers(1, &mesh.indexBuffer);
}

}//namespace prep

}//namespace voxigen