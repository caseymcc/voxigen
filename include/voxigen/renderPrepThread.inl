#include "voxigen/chunkFunctions.h"
#include "voxigen/search.h"

#include <glog/logging.h>

namespace voxigen
{

void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
{
#ifndef NDEBUG
    //    if(severity != DEBUG_SEVERITY_NOTIFICATION)
    if(type==GL_DEBUG_TYPE_ERROR)
        LOG(INFO)<<"Opengl - error: "<<message;
//    else
//        LOG(INFO)<<"Opengl : "<<message;
#endif
}

template<typename _Grid, typename _ChunkRenderer>
RenderPrepThread<_Grid, _ChunkRenderer>::RenderPrepThread()
{
}

template<typename _Grid, typename _ChunkRenderer>
RenderPrepThread<_Grid, _ChunkRenderer>::~RenderPrepThread()
{

}



//template<typename _Grid, typename _ChunkRenderer>
//typename RenderPrepThread<_Grid, _ChunkRenderer>::ChunkRendererType *RenderPrepThread<_Grid, _ChunkRenderer>::getFreeRenderer()
//{
//    if(m_freeChunkRenderers.empty())
//    {
//        size_t minRendererCount=1024;
//        
////        for(size_t i=0; i<m_chunkIndices.size(); ++i)
////            minRendererCount+=m_chunkIndices[i].size();
////        minRendererCount=(minRendererCount*3)/2;
////
////        if(minRendererCount<m_chunkRenderers.size())
////            minRendererCount+m_chunkRenderers.size()+1;
//
//        if(m_chunkRenderers.size()<minRendererCount)
//        {
//            size_t buildIndex=m_chunkRenderers.size();
//            m_chunkRenderers.resize(minRendererCount);
//
//            //need to setup buffers for new chunks
//            for(size_t i=buildIndex; i<m_chunkRenderers.size(); ++i)
//            {
//                ChunkRendererType *chunkRenderer=new ChunkRendererType();
//
////                chunkRenderer->setTextureAtlas(m_textureAtlas);
////                m_chunkRenderers[i].reset(chunkRenderer);
////
////                chunkRenderer->setParent(this);
//
//                m_freeChunkRenderers.push_back(chunkRenderer);
//            }
//        }
//
//        if(m_freeChunkRenderers.empty())
//            return nullptr;
//    }
//
//    ChunkRendererType *renderer=m_freeChunkRenderers.back();
//
//    m_freeChunkRenderers.pop_back();
//    return renderer;
//}

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::requestSearchUpdate()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);

    m_updateSearch=true;
}

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::requestSearchUpdate(const SearchSettings &settings)
{
    std::unique_lock<std::mutex> lock(m_queueMutex);

    m_settings=settings;
    m_updateSearch=true;
}

//template<typename _Grid, typename _ChunkRenderer>
//void RenderPrepThread<_Grid, _ChunkRenderer>::requestRenderer(SharedChunkHandle &chunkHandle)
//{
//    RequestAdd *request=new RequestAdd(chunkHandle);
//
//    m_requestCache.push_back(request);
////    m_addCache.push_back(chunkHandle);
//}
//
//template<typename _Grid, typename _ChunkRenderer>
//void RenderPrepThread<_Grid, _ChunkRenderer>::requestRemove(ChunkRendererType *chunkRenderer)
//{
//    RequestRemove *request=new RequestRemove(chunkRenderer);
//
//    m_requestCache.push_back(request);
////    m_removeCache.push_back(chunkRenderer);
//}


template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::requestMesh(ChunkRendererType *chunkRenderer, TextureAtlas *textureAtlas)
{
#ifdef DEBUG_MESH
    SharedChunkHandle chunkHandle=chunkRenderer->getChunkHandle();
    
    assert(!chunkHandle->empty());
    assert(chunkHandle->action() == ChunkAction::Idle);

    LOG(INFO)<<"Mainthread - ChunkRenderer "<<chunkRenderer<<"("<<chunkRenderer->getRegionHash()<<", "<<chunkRenderer->getChunkHash()<<") requestMesh";
#endif//DEBUG_MESH
    RequestMesh *request=new RequestMesh(chunkRenderer, textureAtlas);

    m_requestCache.push_back(request);
    //    m_updateCache.push_back(chunkRenderer);
}

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::requestReleaseMesh(MeshBuffer &mesh)
{
    RequestReleaseMesh *request=new RequestReleaseMesh(mesh);

    m_requestCache.push_back(request);
}

//template<typename _Grid, typename _ChunkRenderer>
//bool RenderPrepThread<_Grid, _ChunkRenderer>::hasUpdates()
//{
//    return (!m_addedQueue.empty())||(!m_updatedQueue.empty())||(!m_removedQueue.empty());
//}
//
//template<typename _Grid, typename _ChunkRenderer>
//void RenderPrepThread<_Grid, _ChunkRenderer>::getUpdates(const ChunkRenderers &added, const ChunkRenderers &updated, const ChunkRenderers &removed)
//{
//    std::unique_lock<std::mutex> lock(m_queueMutex);
//
//    added.insert(added.end(), m_addedQueue.begin(), m_addedQueue.end());
//    m_addedQueue.clear();
//    updated.insert(updated.end(), m_updatedQueue.begin(), m_updatedQueue.end());
//    m_updatedQueue.clear();
//    removed.insert(removed.end(), m_removedQueue.begin(), m_removedQueue.end());
//    m_removedQueue.clear();
//}

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::updateQueues(Requests &completedQueue)//ChunkRenderers &added, ChunkRenderers &updated, ChunkRenderers &removed)
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

//        if(!m_addCache.empty())
//        {
//            m_addQueue.insert(m_addQueue.end(), m_addCache.begin(), m_addCache.end());
//            m_addCache.clear();
//            update=true;
//        }
//        if(!m_updateCache.empty())  
//        {
//            m_updateQueue.insert(m_updateQueue.end(), m_updateCache.begin(), m_updateCache.end());
//            m_updateCache.clear();
//            update=true;
//        }
//        if(!m_removeCache.empty())
//        {
//            m_removeQueue.insert(m_removeQueue.end(), m_removeCache.begin(), m_removeCache.end());
//            m_removeCache.clear();
//            update=true;
//        }
//
//        if(!m_addedQueue.empty())
//        {
//            added.insert(added.end(), m_addedQueue.begin(), m_addedQueue.end());
//            m_addedQueue.clear();
//        }
//        if(!m_updatedQueue.empty())
//        {
//            updated.insert(updated.end(), m_updatedQueue.begin(), m_updatedQueue.end());
//            m_updatedQueue.clear();
//        }
//        if(!m_removedQueue.empty())
//        {
//            removed.insert(removed.end(), m_removedQueue.begin(), m_removedQueue.end());
//            m_removedQueue.clear();
//
//        }
    }

    if(update)
        m_event.notify_all();
}

//template<typename _Grid, typename _ChunkRenderer>
//void RenderPrepThread<_Grid, _ChunkRenderer>::updateOutputQueues()
//{
//    std::unique_lock<std::mutex> lock(m_queueMutex);
//
//    m_addedQueue.insert(m_addedQueue.end(), m_threadOwned_addedQueue.begin(), m_threadOwned_addedQueue.end());
//    m_threadOwned_addedQueue.clear();
//    m_updatedQueue.insert(m_updatedQueue.end(), m_threadOwned_updatedQueue.begin(), m_threadOwned_updatedQueue.end());
//    m_threadOwned_updatedQueue.clear();
//    m_removedQueue.insert(m_removedQueue.end(), m_threadOwned_removedQueue.begin(), m_threadOwned_removedQueue.end());
//    m_threadOwned_removedQueue.clear();
//}

#ifdef _WINDOWS
template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::start(HDC dc, HGLRC glContext)
{
    m_dc=dc;
    m_glContext=glContext;

    const std::vector<float> &outlineVertices=SimpleCube<ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value>::vertCoords;

//    glGenBuffers(1, &m_outlineInstanceVertices);
//    glBindBuffer(GL_ARRAY_BUFFER, m_outlineInstanceVertices);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_run=true;
    m_thread=std::thread(std::bind(&RenderPrepThread<_Grid, _ChunkRenderer>::processThread, this));
}
#else//_WINDOWS
#endif//_WINDOWS

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=false;
    }

    m_event.notify_all();
    m_thread.join();
}

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::processThread()
{
#ifdef _WINDOWS
    wglMakeCurrent(m_dc, m_glContext);
#else
    assert(false);
#endif

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

    //    ChunkRenderers addRenderers;
    //    ChunkRenderers updateRenderers;
    //    ChunkRenderers removeRenderers;
    RequestQueue requestQueue;
    Requests completedQueue;

    while(run)
    {
        //        if(m_threadOwned_updateQueue.empty() && m_threadOwned_removedQueue.empty() && !updateSearch && index<processBeforeUpdate)
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

            //            if(!m_addQueue.empty())
            //            {
            //                m_threadOwned_addQueue.insert(m_threadOwned_addQueue.end(), m_addQueue.begin(), m_addQueue.end());
            //                m_addQueue.clear();
            //            }
            //            if(!m_updateQueue.empty())
            //            {
            //                m_threadOwned_updateQueue.insert(m_threadOwned_updateQueue.end(), m_updateQueue.begin(), m_updateQueue.end());
            //                m_updateQueue.clear();
            //            }
            //            if(!m_removeQueue.empty())
            //            {
            //                m_threadOwned_removedQueue.insert(m_threadOwned_removedQueue.end(), m_removeQueue.begin(), m_removeQueue.end());
            //                m_removeQueue.clear();
            //            }

                        //while we have the lock update anything that is complete
            if(!completedQueue.empty())
            {
                m_completedQueue.insert(m_completedQueue.end(), completedQueue.begin(), completedQueue.end());
                completedQueue.clear();
            }

            //            if(!m_threadOwned_addedQueue.empty())
            //            {
            //                m_addedQueue.insert(m_addedQueue.end(), m_threadOwned_addedQueue.begin(), m_threadOwned_addedQueue.end());
            //                m_threadOwned_addedQueue.clear();
            //            }
            //            if(!m_threadOwned_updatedQueue.empty())
            //            {
            //                m_updatedQueue.insert(m_updatedQueue.end(), m_threadOwned_updatedQueue.begin(), m_threadOwned_updatedQueue.end());
            //                m_threadOwned_updatedQueue.clear();
            //            }
            //            if(!m_threadOwned_removedQueue.empty())
            //            {
            //                m_removedQueue.insert(m_removedQueue.end(), m_threadOwned_removedQueue.begin(), m_threadOwned_removedQueue.end());
            //                m_threadOwned_removedQueue.clear();
            //            }

            //            if(run && m_threadOwned_updateQueue.empty() &&m_threadOwned_removedQueue.empty() && !updateSearch)
            if(run && requestQueue.empty())
                m_event.wait(lock);
            
            continue;
        }

        index++;

        prep::Request *request=requestQueue.front();

        requestQueue.pop_front();

        switch(request->type)
        {
//        case prep::Add:
//            processAdd((RequestAdd *)request);
//            break;
//        case prep::Remove:
//            processRemove((RequestRemove *)request);
//            break;
        case prep::Mesh:
            processMesh((RequestMesh *)request);
            break;
        case prep::ReleaseMesh:
            processReleaseMesh((RequestReleaseMesh *)request);
            break;
        }

        completedQueue.push_back(request);
    }
}

//template<typename _Grid, typename _ChunkRenderer>
//void RenderPrepThread<_Grid, _ChunkRenderer>::processAdd(RequestAdd *request)
//{
//    ChunkRendererType *renderer=getFreeRenderer();
//
//    renderer->setChunk(request->chunkHandle);
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"RenderPrepThread - ChunkRenderer "<<renderer<<"("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") add";
//#endif//LOG_PROCESS_QUEUE
//
////cant build VAO here has to be done in main draw context
////    renderer->build();
////    renderer->buildOutline(m_outlineInstanceVertices);
//
//    request->renderer=renderer;
//}
//
//
//template<typename _Grid, typename _ChunkRenderer>
//void RenderPrepThread<_Grid, _ChunkRenderer>::processRemove(RequestRemove *request)
//{
//    ChunkRendererType *renderer=request->renderer;
//    auto chunkIter=m_chunkRendererMap.find(renderer->getKey().hash);
//
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"RenderPrepThread - ChunkRenderer "<<renderer<<" ("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") remove";
//#endif//LOG_PROCESS_QUEUE
//
//    if(chunkIter!=m_chunkRendererMap.end())
//        m_chunkRendererMap.erase(chunkIter);
//    m_freeChunkRenderers.push_back(renderer);
//}

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::processMesh(RequestMesh *request)
{
    ChunkRendererType *renderer=request->renderer;
    MeshBuffer &mesh=request->mesh;
    SharedChunkHandle chunkHandle=renderer->getChunkHandle();

    if(chunkHandle->empty())
    {
        mesh.valid=false;
        mesh.indices=0;
        assert(false);//requesting mesh on empty chunk
        return;
    }
#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"RenderPrepThread - ChunkRenderer "<<renderer<<"("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") building mesh";
#endif//LOG_PROCESS_QUEUE

    mesh.indexType=GL_UNSIGNED_INT;
//    glGenVertexArrays(1, &mesh.vertexArray);
    glGenBuffers(1, &mesh.vertexBuffer);
    glGenBuffers(1, &mesh.indexBuffer);
#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"RenderPrepThread - ChunkRenderer "<<renderer<<"("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<
        ") building mesh"<<" ("<<mesh.vertexBuffer<<", "<<mesh.indexBuffer<<")";
#endif//LOG_PROCESS_QUEUE
//    assert(glGetError()==GL_NO_ERROR);
    
//    glBindVertexArray(mesh.vertexArray);
//    
//    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
//
//    glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
//    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(ChunkTextureMesh::Vertex), (GLvoid*)(offsetof(ChunkTextureMesh::Vertex, x)));
//    glEnableVertexAttribArray(1); // Attrib '1' is the vertex texCoord.
//    glVertexAttribIPointer(1, 2, GL_SHORT, sizeof(ChunkTextureMesh::Vertex), (GLvoid*)(offsetof(ChunkTextureMesh::Vertex, tx)));
//    glEnableVertexAttribArray(2); // Attrib '2' is the vertex data.
//    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(ChunkTextureMesh::Vertex), (GLvoid*)(offsetof(ChunkTextureMesh::Vertex, data)));
//    
//    glBindVertexArray(0);

    m_mesh.clear();
    m_mesh.setTextureAtlas(request->textureAtlas);
    buildCubicMesh(m_mesh, chunkHandle->chunk());

    auto &verticies=m_mesh.getVerticies();
    std::vector<int> &indices=m_mesh.getIndices();

    mesh.ready=false;

    if(!indices.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, verticies.size()*sizeof(ChunkMeshVertex), verticies.data(), GL_STATIC_DRAW);
        assert(glGetError()==GL_NO_ERROR);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        assert(glGetError()==GL_NO_ERROR);

        mesh.sync=glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    mesh.valid=true;
    mesh.indices=indices.size();
}

template<typename _Grid, typename _ChunkRenderer>
void RenderPrepThread<_Grid, _ChunkRenderer>::processReleaseMesh(RequestReleaseMesh *request)
{
    MeshBuffer &mesh=request->mesh;

    //likely better to store these and re-use
//    glDeleteVertexArrays(1, &mesh.vertexArray);
    glDeleteBuffers(1, &mesh.vertexBuffer);
    glDeleteBuffers(1, &mesh.indexBuffer);
}

//template<typename _Grid, typename _ChunkRenderer>
//void RenderPrepThread<_Grid, _ChunkRenderer>::updateOcclusionQueries()
//{
//    if(!m_queryComplete)
//        return;
//
//    m_outstandingChunkLoads=0;
//    m_outstandingChunkPreps=0;
//
//    if(m_currentQueryRing>=m_maxChunkRing) //we have completed all query rings, start over
//        m_currentQueryRing=0;
//
//    size_t queriesStarted=0;
//
//#ifdef OLD_SEARCH
//    ChunkRenderers &chunkRenderers=m_chunkQueryOrder[m_currentQueryRing];
//
//    for(size_t i=0; i<chunkRenderers.size(); ++i)
//    {
//        ChunkRendererType *chunkRenderer=chunkRenderers[i];
//
//        if(chunkRenderer->getState()==ChunkRendererType::Occluded)
//        {
//            chunkRenderer->startOcculsionQuery();
//            queriesStarted++;
//        }
//    }
//
//    m_currentQueryRing++;
//#else//OLD_SEARCH
//    //    ChunkRenderers &chunkRenderers=m_searchMap[m_currentQueryRing];
//
//    for(ChunkRendererType *chunkRenderer:m_addedChunks)
//    {
//        if(chunkRenderer->getState()==ChunkRendererType::Occluded)
//        {
//            chunkRenderer->startOcculsionQuery();
//            queriesStarted++;
//        }
//    }
//    m_addedChunks.clear();
//#endif//OLD_SEARCH
//
//    if(queriesStarted>0)
//        m_queryComplete=false;
//
//}

}//namespace voxigen