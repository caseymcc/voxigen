#include "voxigen/meshbuilders/heightmapMeshBuilder.h"
#include "voxigen/rendering/simpleShapes.h"

namespace voxigen
{

template<typename _RegionHandle>
std::string RegionRenderer<_RegionHandle>::vertShader=
"#version 330 core\n"
"layout (location = 0) in uvec3 packedPosition;\n"
"layout (location = 1) in ivec2 vTexCoords;\n"
"layout (location = 2) in uint data;\n"
"\n"
"out vec3 position;\n"
"out vec2 texCoords;\n"
"flat out uint type;\n"
"\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"   vec3 decodedPosition=packedPosition;\n"
"   decodedPosition=decodedPosition;\n"
"   position=regionOffset+decodedPosition;\n"
"   texCoords=vec2(vTexCoords.x, vTexCoords.y);\n"
"   type=data;\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"

"}\n"
"";

template<typename _RegionHandle>
std::string RegionRenderer<_RegionHandle>::fragmentShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
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
"   vec3 normal = cross(dFdy(position), dFdx(position));\n"
"   normal=normalize(normal);\n"
"\n"
"   // ambient\n"
"   float ambientStrength=0.5;\n"
"   vec3 ambient=ambientStrength * lightColor;\n"
"   \n"
"   // diffuse \n"
"   vec3 lightDir=normalize(lightPos-position); \n"
"   float diff=max(dot(normal, lightDir), 0.0); \n"
"   vec3 diffuse=diff * lightColor; \n"
"   color=texelFetch(textureSampler, ivec2(texCoords), 0);\n"
"}\n"
"";

template<typename _RegionHandle>
std::string RegionRenderer<_RegionHandle>::vertOutlineShader=
"#version 330 core\n"
"layout (location = 0) in vec3 inputVertex;\n"
"layout (location = 1) in vec3 inputNormal;\n"
"layout (location = 2) in vec2 inputTexCoord;\n"
"\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"out vec3 position;\n"
"\n"
"void main()\n"
"{\n"
"   position=inputVertex;\n"
"   gl_Position=projectionView*vec4(position+regionOffset, 1.0);\n"
"}\n"
"";

template<typename _RegionHandle>
std::string RegionRenderer<_RegionHandle>::fragmentOutlineShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform vec3 statusColor;\n"
"uniform float lineWidth=0.2;\n"
"\n"
"void main()\n"
"{\n"
"   vec3 distance=min(position, vec3(1024.0, 1024.0, 256.0)-position);\n"
"   float ambientStrength=0.5; \n"
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
"   color=vec4(statusColor, 1.0f);\n"
"}\n"
"";

template<typename _RegionHandle>
bool RegionRenderer<_RegionHandle>::m_outlineInstanceGen=false;

template<typename _RegionHandle>
opengl_util::Program RegionRenderer<_RegionHandle>::m_program;

template<typename _RegionHandle>
size_t RegionRenderer<_RegionHandle>::m_projectionViewId;

template<typename _RegionHandle>
size_t RegionRenderer<_RegionHandle>::m_offsetId;

template<typename _RegionHandle>
opengl_util::Program RegionRenderer<_RegionHandle>::m_outlineProgram;

template<typename _RegionHandle>
size_t RegionRenderer<_RegionHandle>::m_outlineProjectionViewId;

template<typename _RegionHandle>
size_t RegionRenderer<_RegionHandle>::m_outlineOffsetId;

template<typename _RegionHandle>
size_t RegionRenderer<_RegionHandle>::m_outlineColorId;

template<typename _RegionHandle>
unsigned int RegionRenderer<_RegionHandle>::m_outlineInstanceVertices=0;

template<typename _RegionHandle>
glm::mat4 RegionRenderer<_RegionHandle>::m_infoMat;


template<typename _RegionHandle>
RegionRenderer<_RegionHandle>::RegionRenderer():
m_vertexArrayGen(false),
m_outlineGen(false),
refCount(0)
{

}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::updateInfo()
{
    std::ostringstream info;
    const glm::ivec3 &regionIndex=m_region->getRegionIndex();

    if(m_meshBuffer.valid)
        info<<"Renderer: "<<getActionName(m_action)<<", mesh I:"<<m_meshBuffer.indices<<"\n";
    else
        info<<"Renderer: "<<getActionName(m_action)<<", no mesh\n";
    info<<"Region: "<<getHandleActionName(m_region->action())<<", "<<getHandleStateName(m_region->state())<<((m_region->empty())?", empty":"")<<"\n";
    info<<regionIndex.x<<","<<regionIndex.y<<","<<regionIndex.z;
    gltSetText(m_infoText, info.str().c_str());
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::setHandle(SharedRegionHandle handle)
{
    m_region=handle;
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::build()
{
    if(!m_vertexArrayGen)
    {
        gl::glGenVertexArrays(1, &m_vertexArray);
        m_vertexArrayGen=true;

        m_infoText=gltCreateText();
    }

    if(!m_outlineInstanceGen)
    {
        const std::vector<float> &outlineVertices=SimpleCube<Region::sizeX::value*Chunk::sizeX::value,
            Region::sizeY::value*Chunk::sizeY::value,
            Region::sizeZ::value*Chunk::sizeZ::value>::vertCoords;

        gl::glGenBuffers(1, &m_outlineInstanceVertices);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_outlineInstanceVertices);
        gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(float)*outlineVertices.size(), outlineVertices.data(), gl::GL_STATIC_DRAW);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

        float scale(0.05f);
        float lineHeight=gltGetLineHeight(1.0f);

        m_infoMat=glm::scale(glm::mat4(1.0f), glm::vec3(scale, -scale, scale));

        m_infoMat=glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3*lineHeight*scale, 0.0f))*m_infoMat;
        m_infoMat=glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f))*m_infoMat;
        m_infoMat=glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f))*m_infoMat;

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
    gl::glVertexAttribPointer(1, 3, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    gl::glEnableVertexAttribArray(2);
    gl::glVertexAttribPointer(2, 2, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    gl::glEnableVertexAttribArray(3);
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_outlineOffsetVBO);
    gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, nullptr, gl::GL_STATIC_DRAW);
    gl::glVertexAttribPointer(3, 4, gl::GL_FLOAT, gl::GL_FALSE, 0, (void*)0);
    gl::glVertexAttribDivisor(3, 1);

    gl::glBindVertexArray(0);
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::draw(const glm::ivec3 &offset)
{
    if(m_meshBuffer.valid)
    {
        m_program.uniform(m_offsetId)=glm::vec3(offset);

        if(!m_meshBuffer.ready)
        {
            gl::GLenum result=glClientWaitSync((gl::GLsync)m_meshBuffer.sync, gl::SyncObjectMask::GL_NONE_BIT, 0);

            if((result==gl::GL_ALREADY_SIGNALED)||(result==gl::GL_CONDITION_SATISFIED))
            {
                m_meshBuffer.ready=true;
                gl::glDeleteSync((gl::GLsync)m_meshBuffer.sync);
                m_meshBuffer.sync=nullptr;
            }
            else
                return;
        }

        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_meshBuffer.vertexBuffer);
        gl::glBindVertexArray(m_vertexArray);

        // Draw the mesh
        gl::glDrawElements(gl::GL_TRIANGLES, m_meshBuffer.indices, (gl::GLenum)m_meshBuffer.indexType, 0);
        assert(gl::glGetError()==gl::GL_NO_ERROR);
    }
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::drawInfo(const glm::mat4x4 &projectionViewMat, const glm::ivec3 &offset)
{
    updateInfo();

    glm::vec3 position=glm::vec3(0.0f, 0.5f, 0.5f)+glm::vec3(offset);
    glm::mat4 mat=glm::translate(glm::mat4(1.0f), position)*m_infoMat;

    mat=projectionViewMat*mat;
    gltDrawText(m_infoText, (gl::GLfloat*)glm::value_ptr(mat));
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::drawOutline(const glm::ivec3 &offset)
{
    glm::vec3 color(1.0f, 1.0f, 1.0f);

    m_outlineProgram.uniform(m_outlineOffsetId)=glm::vec3(offset);// +getGridOffset());
    m_outlineProgram.uniform(m_outlineColorId)=color;

    gl::glBindVertexArray(m_outlineVertexArray);
    gl::glDrawArraysInstanced(gl::GL_TRIANGLES, 0, 36, 1);
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::clear()
{}

template<typename _RegionHandle>
MeshBuffer RegionRenderer<_RegionHandle>::setMesh(MeshBuffer &mesh)
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
        gl::glVertexAttribIPointer(0, 3, gl::GL_UNSIGNED_SHORT, sizeof(TexturedMesh::Vertex), (gl::GLvoid*)(offsetof(TexturedMesh::Vertex, x)));
        gl::glEnableVertexAttribArray(1); // Attrib '1' is the vertex texCoord.
        gl::glVertexAttribIPointer(1, 2, gl::GL_SHORT, sizeof(TexturedMesh::Vertex), (gl::GLvoid*)(offsetof(TexturedMesh::Vertex, tx)));
        gl::glEnableVertexAttribArray(2); // Attrib '2' is the vertex data.
        gl::glVertexAttribIPointer(2, 1, gl::GL_UNSIGNED_SHORT, sizeof(TexturedMesh::Vertex), (gl::GLvoid*)(offsetof(TexturedMesh::Vertex, data)));

        gl::glBindVertexArray(0);
    }

    return previousMesh;
}

template<typename _RegionHandle>
MeshBuffer RegionRenderer<_RegionHandle>::clearMesh()
{
    MeshBuffer previousMesh;

//    //release old mesh
//    if(m_meshBuffer.valid)
//    {
//        previousMesh=m_meshBuffer;
//        m_meshBuffer.valid=false;
//    }
    return previousMesh;
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::buildPrograms()
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

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::useProgram()
{
    m_program.use();
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::updateProgramProjection(const glm::mat4 &projection)
{
    m_program.uniform(m_projectionViewId)=projection;
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::useOutlineProgram()
{
    m_outlineProgram.use();
}

template<typename _RegionHandle>
void RegionRenderer<_RegionHandle>::updateOutlineProgramProjection(const glm::mat4 &projection)
{
    m_outlineProgram.uniform(m_outlineProjectionViewId)=projection;
}

//namespace prep
//{
//template<typename _Grid>
//TexturedMesh RequestMesh<_Grid, RegionRenderer<typename _Grid::RegionHandleType>>::scratchMesh;
//
///////////////////////////////////////////////////////////////////////////
////This is all happening inside the renderPrep thread so a opengl context is 
////currently valid and the renderer is "locked" for changes, you should not
////change any of the renders state
///////////////////////////////////////////////////////////////////////////
//template<typename _Grid>
//void RequestMesh<_Grid, RegionRenderer<typename _Grid::RegionHandleType>>::process()
//{
//    glm::ivec3 regionSize=details::regionCellSize<Region, Chunk>();
//
//    if(scratchMesh.getVertexes().capacity()==0)
//    {
//        //allocate worst case as this is scratch memory and will be reused
//        size_t vertexCount=regionSize.x*regionSize.y*4; //4 vertexes
//        size_t indexCount=regionSize.x*regionSize.y*2*3; //6 faces 2 triangles per face 3 indexes per triangle 
//
//        scratchMesh.reserve(vertexCount, indexCount);
//    }
//
//    SharedRegionHandle regionHandle=renderer->getHandle();
//
//    if(regionHandle->empty())
//    {
//        mesh.valid=false;
//        mesh.indices=0;
//        assert(false);//requesting mesh on empty region
//        return;
//    }
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"RenderPrepThread - RegionRenderer "<<renderer<<"("<<renderer->getHandle()->getHash()<<") building mesh";
//#endif//LOG_PROCESS_QUEUE
//
//    mesh.indexType=(unsigned int)gl::GL_UNSIGNED_INT;
//
//    gl::glGenBuffers(1, &mesh.vertexBuffer);
//    gl::glGenBuffers(1, &mesh.indexBuffer);
//#ifdef LOG_PROCESS_QUEUE
//    LOG(INFO)<<"RenderPrepThread - RegionRenderer "<<renderer<<"("<<renderer->getHandle()->getHash()<<") building mesh"<<" ("<<mesh.vertexBuffer<<", "<<mesh.indexBuffer<<")";
//#endif//LOG_PROCESS_QUEUE
//
//    scratchMesh.clear();
//    scratchMesh.setTextureAtlas(textureAtlas);
//
//    glm::ivec2 cellsSize=glm::ivec2(regionSize.x, regionSize.y);
//    glm::ivec2 heightRange;
//    buildHeightmapMesh(scratchMesh, regionHandle->getHeightMap(), cellsSize, heightRange, regionHandle->getHeighMapLod());
//
//    auto &vertexes=scratchMesh.getVertexes();
//    std::vector<int> &indexes=scratchMesh.getIndexes();
//
//    mesh.ready=false;
//
//    if(!indexes.empty())
//    {
//        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, mesh.vertexBuffer);
//        gl::glBufferData(gl::GL_ARRAY_BUFFER, vertexes.size()*sizeof(TexturedMesh::Vertex), vertexes.data(), gl::GL_STATIC_DRAW);
//        //        assert(gl::glGetError()==gl::GL_NO_ERROR);
//
//        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
//        gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, indexes.size()*sizeof(uint32_t), indexes.data(), gl::GL_STATIC_DRAW);
//        //        assert(gl::glGetError()==gl::GL_NO_ERROR);
//
//        mesh.sync=gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, gl::UnusedMask::GL_NONE_BIT);
//    }
//
//    mesh.valid=true;
//    mesh.indices=indexes.size();
//}
//
//}//namespace prep
}//namespace voxigen
