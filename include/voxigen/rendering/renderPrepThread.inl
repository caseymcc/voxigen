#include "voxigen/volume/chunkFunctions.h"
#include "voxigen/search.h"

namespace voxigen
{


template<typename _DataType, typename _Object>
RenderPrepThread<_DataType, _Object>::RenderPrepThread(size_t requestSize):
    m_requests(requestSize),
    m_meshes(50)
{
}

template<typename _DataType, typename _Object>
RenderPrepThread<_DataType, _Object>::~RenderPrepThread()
{

}

template<typename _DataType, typename _Object>
bool RenderPrepThread<_DataType, _Object>::requestPositionUpdate(const glm::ivec3 &region, const glm::ivec3 &chunk)
{
    Request *request=m_requests.get();

    if(!request)
        return false;

#ifdef DEBUG_MESH
    Log::debug("MainThread - RenderPrepThread::requestPositionUpdate get request (%x)", request);
#endif//DEBUG_MESH

    request->type=prep::Type::UpdatePos;
    request->priority=prep::Priority::UpdatePos;
    request->getRegion()=region;
    request->getChunk()=chunk;

    m_requestCache.push_back(request);
    return true;
}

template<typename _DataType, typename _Object>
bool RenderPrepThread<_DataType, _Object>::requestMesh(_Object *object, TextureAtlas *textureAtlas)
{
    Request *request=m_requests.get();

    if(!request)
        return false;

#ifdef DEBUG_MESH
    Log::debug("MainThread - RenderPrepThread::requestMesh get request (%x)", request);
#endif//DEBUG_MESH

#ifdef DEBUG_MESH
    auto chunkHandle=object->getChunkHandle();

    assert(!chunkHandle->empty());
    assert(object->getAction()==RenderAction::Idle);

    glm::ivec3 regionIndex=object->getRegionIndex();
    glm::ivec3 chunkIndex=object->getChunkIndex();

    Log::debug("MainThread - RenderPrepThread %x (%d, %d, %d) (%d, %d, %d) requestMesh (%x) action %d", object, 
        regionIndex.x, regionIndex.y, regionIndex.z,
        chunkIndex.x, chunkIndex.y, chunkIndex.z,
        request, object->getAction());
#endif//DEBUG_MESH
    request->type=prep::Type::Mesh;
    request->priority=prep::Priority::Mesh;
    Request::ObjectMesh &objectMesh=request->getObjectMesh();
    
//    object->incrementMesh();
    objectMesh.object=object;
    objectMesh.mesh=nullptr;
    objectMesh.textureAtlas=textureAtlas;

    m_requestCache.push_back(request);
    return true;
}

//template<typename _DataType, typename _Object>
//bool RenderPrepThread<_DataType, _Object>::returnMesh(voxigen::ChunkTextureMesh *mesh)
//{
//    Request *request=m_requests.get();
//
//    if(!request)
//        return false;
//
//    request->type=prep::Type::ReturnMesh;
//    request->priority=prep::Priority::ReturnMesh;
//    Request::ObjectMesh &objectMesh=request->getObjectMesh();
//
//    objectMesh.object=nullptr;
//    objectMesh.mesh=mesh;
//    objectMesh.textureAtlas=nullptr;
//
//    m_requestCache.push_back(request);
//    return true;
//}

template<typename _DataType, typename _Object>
void RenderPrepThread<_DataType, _Object>::returnMeshRequest(Request *request)
{
    //change mesh request to a return
    request->type=prep::Type::ReturnMesh;
    request->priority=prep::Priority::ReturnMesh;
//    Request::ObjectMesh &objectMesh=request->getObjectMesh();
//
//    objectMesh.object=nullptr;
//    objectMesh.mesh=mesh;
//    objectMesh.textureAtlas=nullptr;

    m_requestCache.push_back(request);
}

template<typename _DataType, typename _Object>
bool RenderPrepThread<_DataType, _Object>::requestCancelMesh(_Object *object)
{
    Request *request=m_requests.get();

    if(!request)
        return false;

#ifdef DEBUG_MESH
    Log::debug("MainThread - RenderPrepThread::requestCancelMesh get request (%x)", request);
#endif//DEBUG_MESH

    request->type=prep::Type::CancelMesh;
    request->priority=prep::Priority::CancelMesh;

    request->getRegion()=object->getRegionIndex();
    request->getChunk()=object->getChunkIndex();

#ifdef DEBUG_MESH
    Log::debug("MainThread - RenderPrepThread %x (%d, %d, %d) (%d, %d, %d) requestCancelMesh (%x)", object,
        request->getRegion().x, request->getRegion().y, request->getRegion().z,
        request->getChunk().x, request->getChunk().y, request->getChunk().z,
        request);
#endif//DEBUG_MESH
//    Request::ObjectMesh &objectMesh=request->data.objectMesh;
//
//    objectMesh.object=object;
//    objectMesh.mesh=nullptr;
//    objectMesh.textureAtlas=nullptr;

    m_requestCache.push_back(request);
    return true;
}

//template<typename _DataType, typename _Object>
//void RenderPrepThread<_DataType, _Object>::returnRequest(Request *request)
//{
//    m_requests.release(request);
//}

template<typename _DataType, typename _Object>
void RenderPrepThread<_DataType, _Object>::updateQueues(Requests &completedQueue)
{
    bool update=false;
    Requests completedRequest;

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
            completedRequest.insert(completedRequest.end(), m_completedQueue.begin(), m_completedQueue.end());
            m_completedQueue.clear();
        }
    }

    for(Request *request:completedRequest)
    {
        if(request->type==prep::Type::Mesh)
            completedQueue.push_back(request);
        else
        {
#ifdef DEBUG_MESH
            Log::debug("MainThread - RenderPrepThread release request (%x)", request);

            if(m_requests.exists(request))
            {
                Log::debug("MainThread - RenderPrepThread duplicate (%x)", request);
            }
#endif//DEBUG_MESH
            m_requests.release(request);
        }
    }
    if(update)
    {
//        m_requestAvail=1;
        m_event.notify_all();
    }
}

//RenderPrepThread<_DataType, _Object>::start(NativeGL *nativeGL)
template<typename _DataType, typename _Object>
void RenderPrepThread<_DataType, _Object>::start()//DataType *dataType, Initialize init, Terminate term)
{
//    m_nativeGL=nativeGL;
//    initialize=init;
//    terminate=term;

    m_run=true;
    m_thread=std::thread(std::bind(&RenderPrepThread::processThread, this));
}

template<typename _DataType, typename _Object>
void RenderPrepThread<_DataType, _Object>::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=false;
    }

    m_event.notify_all();
    m_thread.join();
}

template<typename _DataType, typename _Object>
void RenderPrepThread<_DataType, _Object>::processThread()
{
//    initialize(m_data);

    bool run=true;
    bool sortQueue=false;
    bool completed;
    Requests requestQueue;
    Requests completedQueue;
    Requests canceledRequests;
    ChunkTextureMesh scratchMesh;

    glm::ivec3 region(0, 0, 0);
    glm::ivec3 chunk(0, 0, 0);

    //reserve the maximum amount of vertexes and indexes as the mesher assumes all the space required is there
    size_t vertexes=_Object::ChunkType::sizeX::value*_Object::ChunkType::sizeY::value*_Object::ChunkType::sizeZ::value*6*4; //6 faces 4 vertexes
    size_t indexes=_Object::ChunkType::sizeX::value*_Object::ChunkType::sizeY::value*_Object::ChunkType::sizeZ::value*6*2*3; //6 faces 2 triangles per face 3 indexes per triangle 

    scratchMesh.reserve(vertexes, indexes);

    while(run)
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            //update running
            run=m_run;

            //check if any new request have been added.
            if(!m_requestQueue.empty())
            {
                requestQueue.insert(requestQueue.end(), m_requestQueue.begin(), m_requestQueue.end());
                m_requestQueue.clear();
                sortQueue=true;
            }

            //while we have the lock update anything that is complete
            if(!completedQueue.empty())
            {
                m_completedQueue.insert(m_completedQueue.end(), completedQueue.begin(), completedQueue.end());
                completedQueue.clear();
            }

            if(run && requestQueue.empty())
            {
                m_event.wait(lock);
                continue;
            }
        }

        if(sortQueue)
        {
            sortQueue=false;
            //sort the request based on priority and proximity to player
            sort(requestQueue.begin(), requestQueue.end(), [&](Request *r1, Request *r2)
            {
                
                if(r1->priority != r2->priority)
                    return (r1->priority>r2->priority);

                if(r1->type == prep::Type::Mesh)
                {
                    _Object *o1=r1->getObject();
                    _Object *o2=r2->getObject();

                    float o1Distance=o1->distanceTo(region, chunk);
                    float o2Distance=o2->distanceTo(region, chunk);

                    return (o1Distance>o2Distance);
                }
                else
                    return (r1->priority>r2->priority);
            });
        }

        if(requestQueue.empty())
            continue;

        Request *request=requestQueue.back();

        requestQueue.pop_back();

        completed=false;
        switch(request->type)
        {
        case prep::UpdatePos:
            completed=positionUpdate(request, region, chunk);
//            m_requests.release(request);
            completedQueue.push_back(request);
            sortQueue=true;
            break;
        case prep::Mesh:
            completed=buildMesh(request, &scratchMesh);
            if(completed)
                completedQueue.push_back(request);
            else
                requestQueue.push_back(request);//failed to get mesh, push back on stack
            break;
        case prep::ReturnMesh:
            completed=returnMesh(request);
            completedQueue.push_back(request);
//            m_requests.release(request);
            break;
        case prep::CancelMesh:
            if(cancelMesh(request, requestQueue, canceledRequests))
                sortQueue=true;
            if(!canceledRequests.empty())
            {
                for(Request *canceledRequest:canceledRequests)
                {
                    canceledRequest->data.objectMesh.mesh=nullptr;//make sure mesh is null
                    completedQueue.push_back(canceledRequest);
                }
            }
            canceledRequests.clear();
            completedQueue.push_back(request);
//            m_requests.release(request);
            break;
        }
    }

//    terminate(m_data);
}

template<typename _DataType, typename _Object>
bool RenderPrepThread<_DataType, _Object>::positionUpdate(Request *request, glm::ivec3 &region, glm::ivec3 &chunk)
{
    region=request->getRegion();
    chunk=request->getChunk();

    return true;
}

template<typename _DataType, typename _Object>
bool RenderPrepThread<_DataType, _Object>::buildMesh(Request *request, voxigen::ChunkTextureMesh *scratchMesh)
{
    Mesh *mesh=m_meshes.get();

    if(!mesh)
        return false;

    Request::ObjectMesh &objectMesh=request->getObjectMesh();
    _Object *object=objectMesh.object;

#ifdef DEBUG_MESH
    glm::ivec3 regionIndex=object->getRegionIndex();
    glm::ivec3 chunkIndex=object->getChunkIndex();

    Log::debug("RenderPrepThread - RenderPrepThread %x (%d, %d, %d) (%d, %d, %d) meshing (%x)", object,
        regionIndex.x, regionIndex.y, regionIndex.z,
        chunkIndex.x, chunkIndex.y, chunkIndex.z,
        request);
#endif//DEBUG_MESH

    mesh->clear();
    mesh->setTextureAtlas(objectMesh.textureAtlas);

    scratchMesh->clear();
    scratchMesh->setTextureAtlas(objectMesh.textureAtlas);

    auto chunk=object->getHandle()->chunk();

//    if(chunk->hasNeighbors())
//        voxigen::buildCubicMesh_Neighbor(*scratchMesh, chunk, chunk->getNeighbors());
//    else
        voxigen::buildCubicMesh(*scratchMesh, chunk);

    std::vector<Mesh::Vertex> &vertexes=scratchMesh->getVertexes();
    std::vector<int> &indexes=scratchMesh->getIndexes();
    
    //copy from scratch to allocated mesh
    std::vector<Mesh::Vertex> &meshVertexes=mesh->getVertexes();
    std::vector<int> &meshIndexes=mesh->getIndexes();

    meshVertexes.resize(vertexes.size());
    memcpy(meshVertexes.data(), vertexes.data(), vertexes.size()*sizeof(Mesh::Vertex));
    meshIndexes.resize(indexes.size());
    memcpy(meshIndexes.data(), indexes.data(), indexes.size()*sizeof(int));
    
    request->data.objectMesh.mesh=mesh;
    return true;
}

template<typename _DataType, typename _Object>
bool RenderPrepThread<_DataType, _Object>::returnMesh(Request *request)
{
    Request::Mesh *mesh=request->getMesh();

    m_meshes.release(mesh);
    return true;
}

template<typename _DataType, typename _Object>
bool RenderPrepThread<_DataType, _Object>::cancelMesh(Request *request, Requests &requests, Requests &canceled)
{
    Request::ObjectMesh &objectMesh=request->getObjectMesh();
//    _Object *object=objectMesh.object;
    glm::ivec3 &region=request->getRegion();
    glm::ivec3 &chunk=request->getChunk();

    bool erased=false;

    for(Requests::iterator iter=requests.begin(); iter!=requests.end(); ++iter)
    {
        if(request->type!=prep::Mesh)
            continue;

        Request::ObjectMesh &currentObjectMesh=(*iter)->getObjectMesh();
        _Object *currentObject=currentObjectMesh.object;

        if((currentObject->getRegionIndex() == region) && (currentObject->getChunkIndex()==chunk))
        {

#ifdef DEBUG_MESH
            Log::debug("MainThread - RenderPrepThread %x (%d, %d, %d) (%d, %d, %d) cancelMesh (%x)", currentObject,
                region.x, region.y, region.z,
                chunk.x, chunk.y, chunk.z,
                request);
#endif//DEBUG_MESH

            canceled.push_back(*iter);
            requests.erase(iter);
            erased=true;
        }
    }

    return erased;
}

}//namespace voxigen