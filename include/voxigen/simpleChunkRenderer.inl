#include "voxigen/chunkVolume.h"

namespace voxigen
{

template<typename _Parent, typename _Chunk>
SimpleChunkRenderer<_Parent, _Chunk>::SimpleChunkRenderer():
m_state(Init), 
m_chunkOffset(0.0f, 0.0f, 0.0f), 
refCount(0)
//#ifndef NDEBUG
,m_outlineBuilt(false)
//#endif //NDEBUG
{}

template<typename _Parent, typename _Chunk>
SimpleChunkRenderer<_Parent, _Chunk>::~SimpleChunkRenderer()
{}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::setParent(RenderType *parent)
{
    m_parent=parent;
}

//template<typename _Parent, typename _Chunk>
//void SimpleChunkRenderer<_Parent, _Chunk>::setSegmentHash(SegmentHash hash)
//{
//    m_segmentHash=hash;
//}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::setChunk(SharedChunkHandle chunk)
{
    m_chunkHandle=chunk;

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

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::setEmpty()
{
    m_state=Empty;
}

//template<typename _Parent, typename _Chunk>
//void SimpleChunkRenderer<_Parent, _Chunk>::setChunkOffset(glm::vec3 chunkOffset)
//{
//    m_chunkOffset=chunkOffset;
//    
////    if(m_state!=Init)
////        m_state=Dirty;
//}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::build(unsigned int instanceData)
{
    if(m_state != Init)
        return;

//    glGenVertexArrays(1, &m_vertexArray);
//
//    glGenBuffers(1, &m_offsetVBO);
//    update();
//
//    glBindVertexArray(m_vertexArray);
//
//    glEnableVertexAttribArray(0);
//    glBindBuffer(GL_ARRAY_BUFFER, instanceData);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);
//
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));
//
//    glEnableVertexAttribArray(2);
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));
//
//    glEnableVertexAttribArray(3);
//    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*_Chunk::sizeX::value*_Chunk::sizeY::value*_Chunk::sizeZ::value, nullptr, GL_STATIC_DRAW);
//    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
//    glVertexAttribDivisor(3, 1);
//    
//    glBindVertexArray(0);

    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);
    glGenBuffers(1, &m_indexBuffer);
    assert(glGetError()==GL_NO_ERROR);

    glBindVertexArray(m_vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

//    glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
//    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(CubicVertex), (GLvoid*)(offsetof(CubicVertex, encodedPosition)));
//    glEnableVertexAttribArray(1); // Attrib '1' is the vertex normals.
//    glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(CubicVertex), (GLvoid*)(offsetof(CubicVertex, data)));

    glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(ChunkMeshVertex), (GLvoid*)(offsetof(ChunkMeshVertex, x)));
    glEnableVertexAttribArray(1); // Attrib '1' is the vertex normals.
    glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(ChunkMeshVertex), (GLvoid*)(offsetof(ChunkMeshVertex, data)));
    
    glBindVertexArray(0);

//    m_indexType=sizeof(typename MeshType::IndexType)==2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    m_indexType=GL_UNSIGNED_INT;

    glGenQueries(1, &m_queryId);

    if(m_chunkHandle)
#ifdef OCCLUSSION_QUERY
        m_state=Occluded; //assumed, as it will force a check
#else//OCCLUSSION_QUERY
        m_state=Dirty;
#endif//OCCLUSSION_QUERY
    else
        m_state=Invalid;

}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::buildOutline(unsigned int instanceData)
{
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

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::buildMesh()
{
    if(m_state!=Dirty)
        return;

    if(m_chunkHandle->status()!=ChunkHandleType::Memory) //not loaded yet need to wait
    {
        return;
    }

    if(m_chunkHandle->empty())
    {
        m_state=Empty;
        return;
    }

    buildCubicMesh(m_mesh, m_chunkHandle->chunk());

    //we have mesh lets drop the data
//    m_chunkHandle->release();

    calculateMemoryUsed();
}

template<typename _Parent, typename _Chunk>
bool SimpleChunkRenderer<_Parent, _Chunk>::update()
{
    bool copyStarted=false;

    if(m_state!=Dirty)
        return copyStarted;

//    if(m_chunkHandle->status()!=ChunkHandleType::Memory) //not loaded yet need to wait
//    {
//        return copyStarted;
//    }

    if(m_chunkHandle->empty())
    {
        m_state=Empty;
        return copyStarted;
    }

//    ChunkType *chunk=m_chunkHandle->chunk.get();
//
//    auto &cells=chunk->getCells();
////    glm::ivec3 &chunkSize=m_parent->getWorld()->getDescriptors().chunkSize;
////    std::vector<glm::vec4> translations(chunkSize.x*chunkSize.y*chunkSize.z);
//    std::vector<glm::vec4> translations(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value);
////    glm::ivec3 position=m_chunk->getPosition();
//    glm::vec3 position=chunk->getGridOffset();//+m_chunkOffset;
//    glm::ivec3 pos=position;
//
//    int index=0;
//    int validCells=0;
//    
//    for(int z=0; z<ChunkType::sizeZ::value; ++z)
//    {
//        pos.y=position.y;
//        for(int y=0; y<ChunkType::sizeY::value; ++y)
//        {
//            pos.x=position.x;
//            for(int x=0; x<ChunkType::sizeX::value; ++x)
//            {
//                unsigned int type=cells[index].type;
//
//                if(type>0)
//                {
//                    translations[validCells]=glm::vec4(pos, type);
//                    validCells++;
//                }
//                pos.x+=1.0;
//                index++;
//            }
//            pos.y+=1.0;
//        }
//        pos.z+=1.0;
//    }
//
//    m_validBlocks=validCells;
//    if(validCells==0)
//    {
//        m_state=Empty;
//        return;
//    }
//
//    glBindBuffer(GL_ARRAY_BUFFER, m_offsetVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*validCells, translations.data(), GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);

//    ChunkVolume<_Chunk> chunkVolume(m_chunkHandle->chunk.get());
//
//    PolyVox::Region region(PolyVox::Vector3DInt32(0, 0, 0), PolyVox::Vector3DInt32(_Chunk::sizeX::value-1, _Chunk::sizeY::value-1, _Chunk::sizeZ::value-1));
//
//#ifdef NDEBUG
//    m_mesh=PolyVox::extractCubicMesh(&chunkVolume, region);
//#else //NDEBUG
//    m_mesh=PolyVox::extractCubicMesh(&chunkVolume, region, PolyVox::DefaultIsQuadNeeded<ChunkVolume<_Chunk>::VoxelType>(), false);
//#endif //NDEBUG
//
//    size_t vertexBufferSize=sizeof(typename MeshType::VertexType);
//    size_t vertexBufferSize2=sizeof(CubicVertex);
//    size_t indexBufferSize=sizeof(typename MeshType::IndexType);
//
//    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
//    glBufferData(GL_ARRAY_BUFFER, m_mesh.getNoOfVertices()*sizeof(typename MeshType::VertexType), m_mesh.getRawVertexData(), GL_STATIC_DRAW);
//    assert(glGetError()==GL_NO_ERROR);
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_mesh.getNoOfIndices()*sizeof(typename MeshType::IndexType), m_mesh.getRawIndexData(), GL_STATIC_DRAW);
//    assert(glGetError()==GL_NO_ERROR);

    //chunk is invalid
    if(!m_chunkHandle->chunk())
        return copyStarted;

    buildCubicMesh(m_mesh, m_chunkHandle->chunk());

    std::vector<ChunkMeshVertex> &verticies=m_mesh.getVerticies();
    std::vector<int> &indices=m_mesh.getIndices();

    if(!indices.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, verticies.size()*sizeof(ChunkMeshVertex), verticies.data(), GL_STATIC_DRAW);
        assert(glGetError()==GL_NO_ERROR);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        assert(glGetError()==GL_NO_ERROR);

        m_vertexBufferSync=glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        copyStarted=true;
    }
    else
        copyStarted=false;
    //m_validBlocks=m_mesh.getNoOfIndices();
    m_validBlocks=indices.size();


//    //we have mesh lets drop the data
//    m_chunkHandle->release();

    //m_state=Built;
//    m_state=Copy;
    return copyStarted;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::updated()
{
    refCount=0;
    m_state=Copy;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::updateOutline()
{
//#ifndef NDEBUG
    //chunk is not going to be valid till loaded, so going to hack together the offset from
    //the hash info
    glm::vec4 position=glm::vec4(m_parent->getGrid()->getDescriptors().chunkOffset(m_chunkHandle->hash())/*+m_chunkOffset*/, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, m_outlineOffsetVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, glm::value_ptr(position), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_outlineBuilt=true;
//#endif //NDEBUG
    return;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::invalidate()
{
    m_state=Invalid;
//#ifndef NDEBUG
    m_outlineBuilt=false;
//#endif //NDEBUG
    m_chunkHandle.reset();
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::releaseChunkMemory()
{
    m_chunkHandle->release();
    calculateMemoryUsed();
}

template<typename _Parent, typename _Chunk>
bool SimpleChunkRenderer<_Parent, _Chunk>::incrementCopy()
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

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::draw()
{
    if(m_state==Built)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

        glBindVertexArray(m_vertexArray);
//        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, m_validBlocks);

        // Draw the mesh
        glDrawElements(GL_TRIANGLES, m_validBlocks, m_indexType, 0);
        assert(glGetError()==GL_NO_ERROR);
    }
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::startOcculsionQuery()
{
//    if(m_state != Occluded) //we sill have a query id if we are occluded
//        glGenQueries(1, &m_queryId);
    m_state=Query;
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::drawOcculsionQuery()
{
    glBeginQuery(GL_SAMPLES_PASSED, m_queryId);

    glBindVertexArray(m_outlineVertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1);

    glEndQuery(GL_SAMPLES_PASSED);

    m_state=QueryWait;
}

template<typename _Parent, typename _Chunk>
bool SimpleChunkRenderer<_Parent, _Chunk>::checkOcculsionQuery(unsigned int &samples)
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
template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::drawOutline(opengl_util::Program *program, size_t colorId)
{
    glm::vec3 color(1.0f, 1.0f, 1.0f);

    if(!m_outlineBuilt)
        updateOutline();

    if(m_state==Built)
        return;
    if(m_state==Empty)
        return;

//    if(m_state==Empty)
//    {
//        if(m_chunkHandle->memoryUsed()>0)
//            color=glm::vec3(1.0f, 1.0f, 0.0f);
//        else
//            return;
//    }
//
//    if(m_chunkHandle->memoryUsed()>0)
//        color=glm::vec3(1.0f, 0.0f, 0.0f);
//    else
//        color=glm::vec3(0.0f, 1.0f, 0.0f);

    ChunkHandleType::Status status=m_chunkHandle->status();
    
    if(status==ChunkHandleType::Unknown)
        color=glm::vec3(1.0f, 0.0f, 0.0f);
    else if(status==ChunkHandleType::Loading)
        color=glm::vec3(1.0f, 0.0f, 1.0f);
    else if(status==ChunkHandleType::Generating)
        color=glm::vec3(0.0f, 0.0f, 1.0f);
    else if(status==ChunkHandleType::Memory)
    {
//        if(refCount>0)
//            color=glm::vec3(0.0f, 1.0f, 1.0f);
//        else
//            color=glm::vec3(1.0f, 1.0f, 0.0f);
//        if(m_chunkHandle->memoryUsed>0)
//            color=glm::vec3(1.0f, 1.0f, 0.0f);
//        else
            color=glm::vec3(0.0f, 1.0f, 0.0f);
    }
    else
        color=glm::vec3(1.0f, 0.0f, 0.5f);

    program->uniform(colorId)=color;
 //   glm::vec3 position=m_parent->getGrid()->getDescriptors().chunkOffset(m_chunkHandle->hash)+m_chunkOffset;

    glBindVertexArray(m_outlineVertexArray);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1);
}

template<typename _Parent, typename _Chunk>
void SimpleChunkRenderer<_Parent, _Chunk>::calculateMemoryUsed()
{
    m_memoryUsed=0;

    if(m_chunkHandle)
        m_memoryUsed+=m_chunkHandle->memoryUsed();

    m_memoryUsed+=m_mesh.memoryUsed();
}
//#endif //NDEBUG

}//namespace voxigen