#include "voxigen/rendering/voxigen_gltext.h"

namespace voxigen
{

template<typename _Region, typename _Chunk>
std::string SimpleChunkRenderer<_Region, _Chunk>::vertShader=
"#version 330 core\n"
"//layout (location = 0) in vec3 blockvertex;\n"
"//layout (location = 1) in vec3 blockNormal;\n"
"//layout (location = 2) in vec2 blockTexCoord;\n"
"//layout (location = 3) in vec4 blockOffset;\n"
"layout (location = 0) in uvec3 packedPosition;\n"
"layout (location = 1) in ivec3 packedNormal;\n"
"layout (location = 2) in ivec2 vTexCoords;\n"
"layout (location = 3) in uint data;\n"
"\n"
"out vec3 position;\n"
"out vec3 normal;\n"
"out vec2 texCoords;\n"
"flat out uint type;\n"
"\n"
"//layout (std140) uniform pos\n"
"//{\n"
"//   vec4 cameraPos;\n"
"//   vec4 lightPos;\n"
"//   vec4 lightColor;\n"
"//}\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"//   gl_Position=vec4(blockOffset.xyz+blockvertex, 1.0);\n"
"//   position=regionOffset+blockOffset.xyz+blockvertex;\n"
"   vec3 decodedPosition=packedPosition;\n"
"   decodedPosition=decodedPosition;\n"
"   position=regionOffset+decodedPosition;\n"
"//   normal=blockNormal;\n"
"   normal=packedNormal;\n"
"   normal=normal;\n"
"   texCoords=vec2(vTexCoords.x, vTexCoords.y);\n"
"//   texCoords=vec3(0.0, 0.0, data);\n"
"   type=data;\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"

"}\n"
"";

template<typename _Region, typename _Chunk>
std::string SimpleChunkRenderer<_Region, _Chunk>::fragmentShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec2 texCoords;\n"
"flat in uint type;\n"
"out vec4 color;\n"
"\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"\n"
"uniform sampler2D textureSampler;\n"
"\n"
"void main()\n"
"{\n"
"//   vec3 normal = cross(dFdy(position), dFdx(position));\n"
"//   normal=normalize(normal);\n"
"\n"
"//   float value=texCoords.z/10.0f;\n"
"//   color=vec3(texCoords.x, 0.0, texCoords.y);\n"
"   // ambient\n"
"   float ambientStrength=0.5;\n"
"   vec3 ambient=ambientStrength * lightColor;\n"
"   \n"
"   // diffuse \n"
"   vec3 lightDir=normalize(lightPos-position); \n"
"   float diff=max(dot(normal, lightDir), 0.0); \n"
"   vec3 diffuse=diff * lightColor; \n"
"//   color=vec4(texCoords.x/1024.0f, texCoords.y/1024.0f, 0.0f, 1.0f);\n"
"   color=texelFetch(textureSampler, ivec2(texCoords), 0);\n"
"//   color=vec4((ambient+diffuse)*color.rgb, color.a);\n"
"//   color=vec4((ambient+diffuse)*vec3(value, value, value), 1.0f);\n"
"//   color=vec4(abs(normal), 1.0f);\n"
"//   color=vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
"//   if(type==1u)\n"
"//       color=vec4(0.2f, 0.2f, 1.0f, 0.8f);\n"
"//   else if(type>=2u && type<=3u)\n"
"//       color=vec4(0.08f, 0.53f, 0.08f, 1.0f);\n"
"//   else if(type>3u)\n"
"//   {\n"
"//       float level=1.0f-(float(type-3u)/10.f);"
"//       color=vec4(level, level, level, 1.0f);\n"
"//   }\n"
"//   color=vec4(color.rgb*(ambient+diffuse), color.a);\n"
"//   color=vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n"
"";

template<typename _Region, typename _Chunk>
std::string SimpleChunkRenderer<_Region, _Chunk>::vertOutlineShader=
"#version 330 core\n"
"layout (location = 0) in vec3 inputVertex;\n"
"layout (location = 1) in vec3 inputNormal;\n"
"layout (location = 2) in vec2 inputTexCoord;\n"
"layout (location = 3) in vec4 inputOffset;\n"
"\n"
"out vec3 position;\n"
"out vec3 normal;\n"
"out vec3 texCoords;\n"
"out vec3 cubePos;\n"
"\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"//   position=inputOffset.xyz+inputVertex;\n"
"   cubePos=inputVertex;\n"
"   position=regionOffset+inputOffset.xyz+inputVertex;\n"
"   normal=inputNormal;\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"
"}\n"
"";

template<typename _Region, typename _Chunk>
std::string SimpleChunkRenderer<_Region, _Chunk>::fragmentOutlineShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec3 texCoords;\n"
"in vec3 vertexColor;\n"
"in vec3 cubePos;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform vec3 lightPos;\n"
"uniform vec3 statusColor;\n"
"uniform float lineWidth=0.1;\n"
"\n"
"void main()\n"
"{\n"
"//   float value=1.0f;"
"//   vec3 lightColor=vec3(1.0f, 1.0f, 1.0f);\n"
"   vec3 distance=min(cubePos, vec3(64.0, 64.0, 16.0)-cubePos);\n"
"   float ambientStrength=0.5; \n"
"//   vec3 ambient=ambientStrength * lightColor;\n"
"   \n"
"   int count=0;\n"
"   if(distance.x < lineWidth)\n"
"       count++;\n"
"   if(distance.y < lineWidth)\n"
"       count++;\n"
"   if(distance.z < lineWidth)\n"
"       count++;\n"
"   if(count<2)\n"
"       discard;\n"
"   // diffuse \n"
"//   vec3 lightDir=normalize(lightPos-position); \n"
"//   float diff=max(dot(normal, lightDir), 0.0); \n"
"//   vec3 diffuse=diff*lightColor; \n"
"//   color=vec4(statusColor*(ambientStrength+diff), 1.0f);\n"
"   color=vec4(statusColor, 1.0f);\n"
"}\n"
"";

// template<typename _Region, typename _Chunk>
// std::string SimpleChunkRenderer<_Region, _Chunk>::wireFramVertShader=
// "#version 330 core\n"
// "//layout (location = 0) in vec3 blockvertex;\n"
// "//layout (location = 1) in vec3 blockNormal;\n"
// "//layout (location = 2) in vec2 blockTexCoord;\n"
// "//layout (location = 3) in vec4 blockOffset;\n"
// "layout (location = 0) in uvec3 packedPosition;\n"
// "layout (location = 1) in ivec3 packedNormal;\n"
// "layout (location = 2) in ivec2 vTexCoords;\n"
// "layout (location = 3) in uint data;\n"
// "\n"
// "out vec3 position;\n"
// "out vec3 normal;\n"
// "out vec2 texCoords;\n"
// "out vec3 baryCentric;\n"
// "flat out uint type;\n"
// "\n"
// "uniform mat4 projectionView;\n"
// "uniform vec3 regionOffset;\n"
// "\n"
// "void main()\n"
// "{\n"
// "   vec3 decodedPosition=packedPosition;\n"
// "   decodedPosition=decodedPosition;\n"
// "   position=regionOffset+decodedPosition;\n"
// "   normal=packedNormal;\n"
// "   normal=normal;\n"
// "   texCoords=vec2(vTexCoords.x, vTexCoords.y);\n"
// "   vec2 minTexCoords(min()"
// "   baryCentric=vec3(, vTexCoords.x, vTexCoords.y)"
// "   type=data;\n"
// "   gl_Position=projectionView*vec4(position, 1.0);\n"
// 
// "}\n"
// "";


template<typename _Region, typename _Chunk>
bool SimpleChunkRenderer<_Region, _Chunk>::m_outlineInstanceGen=false;

template<typename _Region, typename _Chunk>
opengl_util::Program SimpleChunkRenderer<_Region, _Chunk>::m_program;

template<typename _Region, typename _Chunk>
size_t SimpleChunkRenderer<_Region, _Chunk>::m_projectionViewId;

template<typename _Region, typename _Chunk>
size_t SimpleChunkRenderer<_Region, _Chunk>::m_offsetId;

template<typename _Region, typename _Chunk>
opengl_util::Program SimpleChunkRenderer<_Region, _Chunk>::m_outlineProgram;

template<typename _Region, typename _Chunk>
size_t SimpleChunkRenderer<_Region, _Chunk>::m_outlineProjectionViewId;

template<typename _Region, typename _Chunk>
size_t SimpleChunkRenderer<_Region, _Chunk>::m_outlineOffsetId;

template<typename _Region, typename _Chunk>
size_t SimpleChunkRenderer<_Region, _Chunk>::m_outlineColorId;

template<typename _Region, typename _Chunk>
unsigned int SimpleChunkRenderer<_Region, _Chunk>::m_outlineInstanceVertices=0;


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
void SimpleChunkRenderer<_Region, _Chunk>::updateInfo(std::string &value)
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

    value=info.str();
//    gltSetText(m_infoText, info.str().c_str());
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::updateInfoText()
{
    std::string info;

    updateInfo(info);
    gltSetText(m_infoText, info.c_str());
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::buildPrograms()
{
    std::string error;

    if(!m_program.attachLoadAndCompileShaders(vertShader, fragmentShader, error))
    {
        assert(false);
        return;
    }

    m_projectionViewId=m_program.getUniformId("projectionView");
    m_offsetId=m_program.getUniformId("regionOffset");

    if(!m_outlineProgram.attachLoadAndCompileShaders(vertOutlineShader, fragmentOutlineShader, error))
    {
        assert(false);
        return;
    }

    m_outlineProjectionViewId=m_outlineProgram.getUniformId("projectionView");
    m_outlineOffsetId=m_outlineProgram.getUniformId("regionOffset");
    m_outlineColorId=m_outlineProgram.getUniformId("statusColor");
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::useProgram()
{
    m_program.use();
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::updateProgramProjection(const glm::mat4 &projection)
{
    m_program.uniform(m_projectionViewId)=projection;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::useOutlineProgram()
{
    m_outlineProgram.use();
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::updateOutlineProgramProjection(const glm::mat4 &projection)
{
    m_outlineProgram.uniform(m_outlineProjectionViewId)=projection;
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::setChunk(SharedChunkHandle chunk)
{
    m_chunkHandle=chunk;

    updateInfoText();

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
    if(!m_vertexArrayGen)
    {
        gl::glGenVertexArrays(1, &m_vertexArray);
        m_vertexArrayGen=true;

        m_infoText=gltCreateText();
    }

    if(!m_outlineInstanceGen)
    {
        const std::vector<float> &outlineVertices=SimpleCube<ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value>::vertCoords;

        gl::glGenBuffers(1, &m_outlineInstanceVertices);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_outlineInstanceVertices);
        gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), gl::GL_STATIC_DRAW);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

        //        float scale(0.05f);
        //        float lineHeight=gltGetLineHeight(1.0f);
        //
        //        m_infoMat=glm::scale(glm::mat4(1.0f), glm::vec3(scale, -scale, scale));
        //
        //        m_infoMat=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3*lineHeight*scale, 0.0f))*m_infoMat;
        //        m_infoMat=glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f))*m_infoMat;
        //        m_infoMat=glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f))*m_infoMat;

        m_outlineInstanceGen=true;
    }

    if(m_outlineGen)
        return;

    m_outlineGen=true;
    gl::glGenVertexArrays(1, &m_outlineVertexArray);
    gl::glGenBuffers(1, &m_outlineOffsetVBO);
    gl::glBindVertexArray(m_outlineVertexArray);

    gl::glEnableVertexAttribArray(0);
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_outlineInstanceVertices);
    gl::glVertexAttribPointer(0, 3, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float)*8, (void*)0);

    gl::glEnableVertexAttribArray(1);
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
        gl::glBindVertexArray(m_vertexArray);

        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_meshBuffer.vertexBuffer);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, m_meshBuffer.indexBuffer);

        gl::glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
        gl::glVertexAttribIPointer(0, 3, gl::GL_UNSIGNED_BYTE, sizeof(ChunkTextureMesh::Vertex), (gl::GLvoid*)(offsetof(ChunkTextureMesh::Vertex, x)));
        gl::glEnableVertexAttribArray(1); // Attrib '1' is the normal.
        gl::glVertexAttribIPointer(1, 3, gl::GL_BYTE, sizeof(ChunkTextureMesh::Vertex), (gl::GLvoid*)(offsetof(ChunkTextureMesh::Vertex, nx)));
        gl::glEnableVertexAttribArray(2); // Attrib '2' is the vertex texCoord.
        gl::glVertexAttribIPointer(2, 2, gl::GL_SHORT, sizeof(ChunkTextureMesh::Vertex), (gl::GLvoid*)(offsetof(ChunkTextureMesh::Vertex, tx)));
        gl::glEnableVertexAttribArray(3); // Attrib '3' is the vertex data.
        gl::glVertexAttribIPointer(3, 1, gl::GL_UNSIGNED_INT, sizeof(ChunkTextureMesh::Vertex), (gl::GLvoid*)(offsetof(ChunkTextureMesh::Vertex, data)));

        gl::glBindVertexArray(0);
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

    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_outlineOffsetVBO);
    gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, glm::value_ptr(position), gl::GL_STATIC_DRAW);
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

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
    if(m_chunkHandle)
        m_chunkHandle->removeInUse();

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
        gl::GLenum result=gl::glClientWaitSync(m_vertexBufferSync, gl::SyncObjectMask::GL_NONE_BIT, 0);

        if((result==gl::GL_ALREADY_SIGNALED)||(result==gl::GL_CONDITION_SATISFIED))
        {
            m_delayedFrames=1;
            gl::glDeleteSync(m_vertexBufferSync);
        }
        else if(result==gl::GL_WAIT_FAILED)
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
void SimpleChunkRenderer<_Region, _Chunk>::draw(const glm::ivec3 &offset)
{
//    if(m_state==Built)
    if(m_meshBuffer.valid)
    {
        glm::vec3 renderOffset=glm::vec3(offset+getGridOffset());

        m_program.uniform(m_offsetId)=renderOffset;

        if(!m_meshBuffer.ready)
            return;

//        if(!m_meshBuffer.ready)
//        {
//            gl::GLenum result=gl::glClientWaitSync((gl::GLsync)m_meshBuffer.sync, gl::SyncObjectMask::GL_NONE_BIT, 0);
//
//            if((result==gl::GL_ALREADY_SIGNALED)||(result==gl::GL_CONDITION_SATISFIED))
//            {
//                m_meshBuffer.ready=true;
//                gl::glDeleteSync((gl::GLsync)m_meshBuffer.sync);
//                m_meshBuffer.sync=nullptr;
//            }
//            else
//                return;
//        }

        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_meshBuffer.vertexBuffer);

        gl::glBindVertexArray(m_vertexArray);

        // Draw the mesh
        gl::glDrawElements(gl::GL_TRIANGLES, m_meshBuffer.indices, (gl::GLenum)m_meshBuffer.indexType, 0);
        assert(gl::glGetError()==gl::GL_NO_ERROR);
    }


}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::drawInfo(const glm::mat4x4 &projectionViewMat, const glm::ivec3 &offset)
{
    updateInfoText();

    float scale(0.05f);
    float lineHeight=gltGetLineHeight(1.0f);
    glm::vec3 chunkPos=glm::vec3(m_chunkHandle->chunkIndex()*m_chunkHandle->size())+glm::vec3(0.5f, 0.0f, 0.5f)+glm::vec3(offset);
//    glm::mat4 mat=glm::translate(glm::mat4(1.0f), chunkPos);
    glm::mat4 mat=glm::scale(glm::mat4(1.0f), glm::vec3(scale, -scale, scale));

    mat=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3*lineHeight*scale, 0.0f))*mat;
    mat=glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f))*mat;
    mat=glm::translate(glm::mat4(1.0f), chunkPos)*mat;

    mat=projectionViewMat*mat;

    gltDrawText(m_infoText, (gl::GLfloat*)glm::value_ptr(mat));
}

template<typename _Region, typename _Chunk>
void SimpleChunkRenderer<_Region, _Chunk>::drawOutline(const glm::ivec3 &offset)
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

    m_outlineProgram.uniform(m_outlineOffsetId)=glm::vec3(offset)+glm::vec3(getGridOffset());
    m_outlineProgram.uniform(m_outlineColorId)=color;

    gl::glBindVertexArray(m_outlineVertexArray);
    gl::glDrawArraysInstanced(gl::GL_TRIANGLES, 0, 36, 1);
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
    gl::glBeginQuery(gl::GL_SAMPLES_PASSED, m_queryId);

    gl::glBindVertexArray(m_outlineVertexArray);
    gl::glDrawArraysInstanced(gl::GL_TRIANGLES, 0, 36, 1);

    gl::glEndQuery(gl::GL_SAMPLES_PASSED);

    m_state=QueryWait;
}

template<typename _Region, typename _Chunk>
bool SimpleChunkRenderer<_Region, _Chunk>::checkOcculsionQuery(unsigned int &samples)
{
    unsigned int hasResult;

    gl::glGetQueryObjectuiv(m_queryId, gl::GL_QUERY_RESULT_AVAILABLE, &hasResult);

    if(hasResult>0)
    {
        gl::glGetQueryObjectuiv(m_queryId, gl::GL_QUERY_RESULT, &samples);

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


//namespace prep
//{
//template<typename _Grid>
//ChunkTextureMesh RequestMesh<_Grid, SimpleChunkRenderer<typename _Grid::RegionType, typename _Grid::ChunkType>>::scratchMesh;
//
/////////////////////////////////////////////////////////////////////////
//This is all happening inside the renderPrep thread so a opengl context is 
//currently valid and the renderer is "locked" for changes, you should not
//change any of the renders state
/////////////////////////////////////////////////////////////////////////
//template<typename _Grid>
//void RequestMesh<_Grid, SimpleChunkRenderer<typename _Grid::RegionType, typename _Grid::ChunkType>>::process()
//{
//    if(scratchMesh.getVertexes().capacity()==0)
//    {
//        size_t vertexes=Chunk::sizeX::value*Chunk::sizeY::value*Chunk::sizeZ::value*6*4; //6 faces 4 vertexes
//        size_t indexes=Chunk::sizeX::value*Chunk::sizeY::value*Chunk::sizeZ::value*6*2*3; //6 faces 2 triangles per face 3 indexes per triangle 
//
//        scratchMesh.reserve(vertexes, indexes);
//    }
//
//    SharedChunkHandle chunkHandle=renderer->getChunkHandle();
//
//    if(chunkHandle->empty())
//    {
//        mesh.valid=false;
//        mesh.indices=0;
//        assert(false);//requesting mesh on empty chunk
//        return;
//    }
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"RenderPrepThread - ChunkRenderer "<<renderer<<"("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<") building mesh";
//#endif//LOG_PROCESS_QUEUE
//
//    mesh.indexType=(unsigned int)gl::GL_UNSIGNED_INT;
//
//    gl::glGenBuffers(1, &mesh.vertexBuffer);
//    gl::glGenBuffers(1, &mesh.indexBuffer);
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"RenderPrepThread - ChunkRenderer "<<renderer<<"("<<renderer->getRegionHash()<<", "<<renderer->getChunkHash()<<
//        ") building mesh"<<" ("<<mesh.vertexBuffer<<", "<<mesh.indexBuffer<<")";
//#endif//LOG_PROCESS_QUEUE
//
//    scratchMesh.clear();
//    scratchMesh.setTextureAtlas(textureAtlas);
//    buildCubicMesh(scratchMesh, chunkHandle->chunk());
//
//    auto &verticies=scratchMesh.getVertexes();
//    std::vector<int> &indices=scratchMesh.getIndexes();
//
//    mesh.ready=false;
//
//    if(!indices.empty())
//    {
//        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, mesh.vertexBuffer);
//        gl::glBufferData(gl::GL_ARRAY_BUFFER, verticies.size()*sizeof(ChunkTextureMesh::Vertex), verticies.data(), gl::GL_STATIC_DRAW);
////        assert(gl::glGetError()==gl::GL_NO_ERROR);
//
//        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
//        gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), gl::GL_STATIC_DRAW);
////        assert(gl::glGetError()==gl::GL_NO_ERROR);
//
//        mesh.sync=gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, gl::UnusedMask::GL_NONE_BIT);
//        gl::glFlush();
//    }
//
//    mesh.valid=true;
//    mesh.indices=indices.size();
//}
//
//}//namespace prep

}//namespace voxigen