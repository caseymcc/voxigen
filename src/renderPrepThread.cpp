//#include "voxigen/renderPrepThread.h"
//
//#include <glbinding/gl/gl.h>
//
//namespace voxigen
//{
//
//using namespace gl;
//
//void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
//{
//#ifndef NDEBUG
//#ifdef VOXIGEN_USE_LOGGING
//    if(type==GL_DEBUG_TYPE_ERROR)
//        LOG(INFO)<<"Opengl - error: "<<message;
////    else
////        LOG(INFO)<<"Opengl : "<<message;
//#endif//VOXIGEN_USE_LOGGING
//#endif
//}
//
//
//RenderPrepThread::RenderPrepThread()
//{
//}
//
//
//RenderPrepThread::~RenderPrepThread()
//{
//
//}
//
//void RenderPrepThread::requestSearchUpdate()
//{
//    std::unique_lock<std::mutex> lock(m_queueMutex);
//
//    m_updateSearch=true;
//}
//
//
//void RenderPrepThread::requestSearchUpdate(const SearchSettings &settings)
//{
//    std::unique_lock<std::mutex> lock(m_queueMutex);
//
//    m_settings=settings;
//    m_updateSearch=true;
//}
//
//void RenderPrepThread::requestReleaseMesh(MeshBuffer &mesh)
//{
//    prep::RequestReleaseMesh *request=new prep::RequestReleaseMesh(mesh);
//
//    m_requestCache.push_back(request);
//}
//
//void RenderPrepThread::updateQueues(Requests &completedQueue)//ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed)
//{
//    bool update=false;
//
//    {
//        std::unique_lock<std::mutex> lock(m_queueMutex);
//
//        if(!m_requestCache.empty())
//        {
//            m_requestQueue.insert(m_requestQueue.end(), m_requestCache.begin(), m_requestCache.end());
//            m_requestCache.clear();
//            update=true;
//
//        }
//
//        if(!m_completedQueue.empty())
//        {
//            completedQueue.insert(completedQueue.end(), m_completedQueue.begin(), m_completedQueue.end());
//            m_completedQueue.clear();
//        }
//    }
//
//    if(update)
//        m_event.notify_all();
//}
//
////void RenderPrepThread::start(NativeGL *nativeGL)
//void RenderPrepThread::start(DataType *dataType, Initialize init, Terminate term);
//{
////    m_nativeGL=nativeGL;
//    initialize=init;
//    terminate=term;
//
//    m_run=true;
//    m_thread=std::thread(std::bind(&RenderPrepThread::processThread, this));
//}
//
//
//void RenderPrepThread::stop()
//{
//    {
//        std::unique_lock<std::mutex> lock(m_queueMutex);
//        m_run=false;
//    }
//
//    m_event.notify_all();
//    m_thread.join();
//}
//
//
//void RenderPrepThread::processThread()
//{
////    m_nativeGL->makeCurrent();
//    initialize(m_data);
//
//#ifndef NDEBUG
//    GLint majorVersion, minorVersion;
//
//    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
//    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
//
//    if((majorVersion >= 4) && (minorVersion >= 3))
//    {
//        glDebugMessageCallback(debugMessage, nullptr);
//        glEnable(GL_DEBUG_OUTPUT);
//    }
//#endif
//
//    bool run=true;
//    bool updateSearch=true;
//
//    bool searchSet=false;
//    SearchSettings settings;
//
//    size_t index=0;
//    size_t processBeforeUpdate=15;
//
//    RequestQueue requestQueue;
//    Requests completedQueue;
//
//    while(run)
//    {
//        if(requestQueue.empty() || index>processBeforeUpdate)
//        {
//            index=0;
//
//            std::unique_lock<std::mutex> lock(m_queueMutex);
//
//            //update running
//            run=m_run;
//            if(m_updateSearch)
//            {
//                searchSet=true;
//                updateSearch=m_updateSearch;
//                settings=m_settings;
//                m_updateSearch=false;
//            }
//
//            //check if any new request have been added.
//            if(!m_requestQueue.empty())
//            {
//                requestQueue.insert(requestQueue.end(), m_requestQueue.begin(), m_requestQueue.end());
//                m_requestQueue.clear();
//            }
//
//            //while we have the lock update anything that is complete
//            if(!completedQueue.empty())
//            {
//                m_completedQueue.insert(m_completedQueue.end(), completedQueue.begin(), completedQueue.end());
//                completedQueue.clear();
//            }
//
//            if(run && requestQueue.empty())
//                m_event.wait(lock);
//            
//            continue;
//        }
//
//        index++;
//
//        prep::Request *request=requestQueue.front();
//
//        requestQueue.pop_front();
//        request->process();
//        completedQueue.push_back(request);
//    }
//
//    terminate(m_data);
//}
//
//namespace prep
//{
//
//void RequestReleaseMesh::process()
//{
//    //likely better to store these and re-use
//    glDeleteBuffers(1, &mesh.vertexBuffer);
//    glDeleteBuffers(1, &mesh.indexBuffer);
//}
//
//}//namespace prep
//
//}//namespace voxigen