#include "voxigen/gltext.h"

namespace voxigen
{

template<typename _Region, typename _Chunk>
SimpleChunkRenderer<_Region, _Chunk>::SimpleChunkRenderer():
m_state(Init),
m_action(RenderAction::Idle),
m_chunkOffset(0.0f, 0.0f, 0.0f), 
refCount(0),
m_lodUpdated(false),
m_vertexArrayGen(false),
//#ifndef NDEBUG
m_outlineGen(false),
m_outlineBuilt(false),
m_infoText(nullptr)
//#endif //NDEBUG
{}

template<typename _Region, typename _Chunk>
SimpleChunkRenderer<_Region, _Chunk>::~SimpleChunkRenderer()
{
    if(m_infoText)
        gltDeleteText(m_infoText);
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::updateInfo()
{
    std::ostringstream info;
    const glm::ivec3 &regionIndex=m_chunkHandle->regionIndex();
    const glm::ivec3 &chunkIndex=m_chunkHandle->chunkIndex();

    if(m_meshBuffer.valid)
        info<<"Renderer: "<<getActionName(m_action)<<", mesh I:"<<m_meshBuffer.indices<<"\n";
    else
        info<<"Renderer: "<<getActionName(m_action)<<", no mesh\n";
    info<<"Chunk: "<<getHandleActionName(m_chunkHandle->action())<<", "<<getHandleStateName(m_chunkHandle->state())<<((m_chunkHandle->empty())?", empty":"")<<"\n";
    info<<"Region: "<<regionIndex.x<<","<<regionIndex.y<<","<<regionIndex.z<<" Chunk: "<<chunkIndex.x<<","<<chunkIndex.y<<","<<chunkIndex.z;
    gltSetText(m_infoText, info.str().c_str());
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::setChunk(SharedChunkHandle chunk)
{
    m_chunkHandle=chunk;

    updateInfo();

    if(m_state!=Init)
    {
#ifdef OCCLUSSION_QUERY
        m_state=Occluded;
#else//OCCLUSSION_QUERY
        m_state=Dirty;
#endif//OCCLUSSION_QUERY
    }
    m_delayedFrames=0;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::setHandle(SharedChunkHandle chunk)
{
    setChunk(chunk);
}


template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::setEmpty()
{
    m_state=Empty;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::build()//unsigned int instanceData)
{
    if(m_vertexArrayGen)
        return;

    glGenVertexArrays(1, &m_vertexArray);
    m_vertexArrayGen=true;

    m_infoText=gltCreateText();
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::buildOutline(unsigned int instanceData)
{
    if(m_outlineGen)
        return;

    m_outlineGen=true;
    glGenVertexArrays(1, &m_outlineVertexArray);
    glGenBuffers(1, &m_outlineOffsetVBO);

    glBindVertexArray(m_outlineVertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceData);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineOffsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

template<typename _Region, typename _Chunk>
MeshBuffer SimpleChunkRenderer<_Region, _Chunk>::clearMesh()
{
    MeshBuffer previousMesh;

    //release old mesh
    if(m_meshBuffer.valid)
    {
        previousMesh=m_meshBuffer;
        m_meshBuffer.valid=false;
    }
    return previousMesh;
}

template<typename _Region, typename _Chunk>
MeshBuffer SimpleChunkRenderer<_Region, _Chunk>::setMesh(MeshBuffer &mesh)
{
    MeshBuffer previousMesh;

    //release old mesh
    if(m_meshBuffer.valid)
    {
        previousMesh=m_meshBuffer;
    }

    m_meshBuffer=mesh;

    if(mesh.valid)
    {
        //need to update VAO
        glBindVertexArray(m_vertexArray);

        glBindBuffer(GL_ARRAY_BUFFER, m_meshBuffer.vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshBuffer.indexBuffer);

        glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
        glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(ChunkTextureMesh::Vertex), (GLvoid*)(offsetof(ChunkTextureMesh::Vertex, x)));
        glEnableVertexAttribArray(1); // Attrib '1' is the vertex texCoord.
        glVertexAttribIPointer(1, 2, GL_SHORT, sizeof(ChunkTextureMesh::Vertex), (GLvoid*)(offsetof(ChunkTextureMesh::Vertex, tx)));
        glEnableVertexAttribArray(2); // Attrib '2' is the vertex data.
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(ChunkTextureMesh::Vertex), (GLvoid*)(offsetof(ChunkTextureMesh::Vertex, data)));

        glBindVertexArray(0);
    }

    return previousMesh;
}

template<typename _Region, typename _Chunk>
bool SimpleChunkRenderer<_Region, _Chunk>::update()
{
    bool copyStarted=false;

    if(m_state!=Dirty)
        return copyStarted;

    if(m_chunkHandle->empty())
    {
        m_state=Empty;
        return copyStarted;
    }
    return false;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::updated()
{
    m_state=Copy;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::updateOutline()
{
//#ifndef NDEBUG
    //chunk is not going to be valid till loaded, so going to hack together the offset from
    //the hash info
    glm::vec4 position=glm::vec4(m_chunkHandle->regionOffset(), 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, m_outlineOffsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, glm::value_ptr(position), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_outlineBuilt=true;
//#endif //NDEBUG
    return;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::invalidate()
{
    m_state=Invalid;
//#ifndef NDEBUG
    m_outlineBuilt=false;
//#endif //NDEBUG
    m_chunkHandle.reset();
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::releaseChunkMemory()
{
//    m_Region->getGrid()->releaseChunk(m_chunkHandle);
}

template<typename _Region, typename _Chunk>
bool SimpleChunkRenderer<_Region, _Chunk>::incrementCopy()
{
    if(m_delayedFrames==0)
    {
        //wait for sync notification
        GLenum result=glClientWaitSync(m_vertexBufferSync, 0, 0);

        if((result==GL_ALREADY_SIGNALED)||(result==GL_CONDITION_SATISFIED))
        {
            m_delayedFrames=1;
            glDeleteSync(m_vertexBufferSync);
        }
        else if(result==GL_WAIT_FAILED)
            assert(false);
    }
    else
    {
        //now delay a few frames, sync only says copy was started
        m_delayedFrames++;
        if(m_delayedFrames>3)
        {
            m_state=Built;
            return true;
        }
    }

    return false;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::draw(opengl_util::Program *program, size_t offsetId, const glm::ivec3 &offset)
{
//    if(m_state==Built)
    if(m_meshBuffer.valid)
    {
        glm::vec3 renderOffset=glm::vec3(offset+getGridOffset());

        program->uniform(offsetId)=renderOffset;

        if(!m_meshBuffer.ready)
        {
            GLenum result=glClientWaitSync(m_meshBuffer.sync, 0, 0);

            if((result==GL_ALREADY_SIGNALED)||(result==GL_CONDITION_SATISFIED))
            {
                m_meshBuffer.ready=true;
                glDeleteSync(m_meshBuffer.sync);
                m_meshBuffer.sync=nullptr;
            }
            else
                return;
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_meshBuffer.vertexBuffer);

        glBindVertexArray(m_vertexArray);

        // Draw the mesh
        glDrawElements(GL_TRIANGLES, m_meshBuffer.indices, m_meshBuffer.indexType, 0);
        assert(glGetError()==GL_NO_ERROR);
    }


}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::drawInfo(const glm::mat4x4 &projectionViewMat, const glm::ivec3 &offset)
{
    updateInfo();

    float scale(0.05f);
    float lineHeight=gltGetLineHeight(1.0f);
    glm::vec3 chunkPos=glm::vec3(m_chunkHandle->chunkIndex()*m_chunkHandle->size())+glm::vec3(0.5f, 0.0f, 0.5f)+glm::vec3(offset);
//    glm::mat4 mat=glm::translate(glm::mat4(1.0f), chunkPos);
    glm::mat4 mat=glm::scale(glm::mat4(1.0f), glm::vec3(scale, -scale, scale));

    mat=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3*lineHeight*scale, 0.0f))*mat;
    mat=glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f))*mat;
    mat=glm::translate(glm::mat4(1.0f), chunkPos)*mat;

    mat=projectionViewMat*mat;

    gltDrawText(m_infoText, (GLfloat*)glm::value_ptr(mat));
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::drawOutline(opengl_util::Program *program, size_t offsetId, const glm::ivec3 &offset, size_t colorId)
{
    

    glm::vec3 color(1.0f, 1.0f, 1.0f);

    if(!m_outlineBuilt)
        updateOutline();

    if(m_state==Built)
        return;
    if(m_state==Empty)
        return;

    HandleAction action=m_chunkHandle->action();

    if(action==HandleAction::Idle)
    {
        HandleState state=m_chunkHandle->state();

        if(state==HandleState::Memory)
        {
            if(m_chunkHandle->empty())
                color=glm::vec3(0.0f, 0.0f, 1.0f);
            else if(m_meshBuffer.ready)
                color=glm::vec3(0.0f, 1.0f, 0.0f);
            else
                color=glm::vec3(0.0f, 1.0f, 1.0f);
        }
        else if(state==HandleState::Unknown)
            color=glm::vec3(0.0f, 0.0f, 0.0f);
    }
    else if(action==HandleAction::Reading)
        color=glm::vec3(0.5f, 0.0f, 1.0f);
    else if(action==HandleAction::Generating)
        color=glm::vec3(1.0f, 0.5f, 0.0f);

    program->uniform(offsetId)=glm::vec3(offset);// +getGridOffset());
    program->uniform(colorId)=color;

    glBindVertexArray(m_outlineVertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1);
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::startOcculsionQuery()
{
//    if(m_state != Occluded) //we sill have a query id if we are occluded
//        glGenQueries(1, &m_queryId);
    m_state=Query;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::drawOcculsionQuery()
{
    glBeginQuery(GL_SAMPLES_PASSED, m_queryId);

    glBindVertexArray(m_outlineVertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1);

    glEndQuery(GL_SAMPLES_PASSED);

    m_state=QueryWait;
}

template<typename _Region, typename _Chunk>
bool SimpleChunkRenderer<_Region, _Chunk>::checkOcculsionQuery(unsigned int &samples)
{
    unsigned int hasResult;

    glGetQueryObjectuiv(m_queryId, GL_QUERY_RESULT_AVAILABLE, &hasResult);

    if(hasResult>0)
    {
        glGetQueryObjectuiv(m_queryId, GL_QUERY_RESULT, &samples);

        if(samples>0)
        {
//            glDeleteQueries(1, &m_queryId);
            m_state=Dirty;
        }
        else
            m_state=Occluded;
        return true;
    }
    return false;
}

//#ifndef NDEBUG


template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::setLod(size_t lod)
{
    m_lod=lod;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::clear()
{
    m_chunkHandle.reset();
    m_outlineBuilt=false;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::calculateMemoryUsed()
{
    m_memoryUsed=0;

    if(m_chunkHandle)
        m_memoryUsed+=m_chunkHandle->memoryUsed();
}
//#endif //NDEBUG


namespace prep
{
template<typename _Grid>
ChunkTextureMesh RequestMesh<_Grid, SimpleChunkRenderer<typename _Grid::RegionType, typename _Grid::ChunkType>>::scratchMesh;

/////////////////////////////////////////////////////////////////////////
//This is all happening inside the renderPrep thread so a opengl context is 
//currently valid and the renderer is "locked" for changes, you should not
//change any of the renders state
/////////////////////////////////////////////////////////////////////////
template<typename _Grid>
void RequestMesh<_Grid, SimpleChunkRenderer<typename _Grid::RegionType, typename _Grid::ChunkType>>::process()
{
    if(scratchMesh.getVerticies().capacity()==0)
    {
        size_t vertexes=Chunk::sizeX::value*Chunk::sizeY::value*Chunk::sizeY::value*6*4; //6 faces 4 vertexes
        size_t indexes=Chunk::sizeX::value*Chunk::sizeY::value*Chunk::sizeY::value*6*2*3; //6 faces 2 triangles per face 3 indexes per triangle 

        scratchMesh.reserve(vertexes, indexes);
    }

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

    glGenBuffers(1, &mesh.vertexBuffer);
    glGenBuffers(1, &mesh.indexBuffer);
#ifdef LOG_PROCESS_QUEUE
    LOG(INFO)<<"RenderPrepThread - ChunkRenderer "<<renderer<<"("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<
        ") building mesh"<<" ("<<mesh.vertexBuffer<<", "<<mesh.indexBuffer<<")";
#endif//LOG_PROCESS_QUEUE

    scratchMesh.clear();
    scratchMesh.setTextureAtlas(textureAtlas);
    buildCubicMesh(scratchMesh, chunkHandle->chunk());

    auto &verticies=scratchMesh.getVerticies();
    std::vector<int> &indices=scratchMesh.getIndices();

    mesh.ready=false;

    if(!indices.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, verticies.size()*sizeof(ChunkMeshVertex), verticies.data(), GL_STATIC_DRAW);
//        assert(glGetError()==GL_NO_ERROR);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
//        assert(glGetError()==GL_NO_ERROR);

        mesh.sync=glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    mesh.valid=true;
    mesh.indices=indices.size();
}

}//namespace prep
}//namespace voxigen