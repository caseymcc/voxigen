#ifndef _voxigen_simpleRegionRenderer_h_
#define _voxigen_simpleRegionRenderer_h_

#include "voxigen/rendererSettings.h"
#include "voxigen/voxigen_export.h"
#include "voxigen/rendering/renderAction.h"

#include "voxigen/meshes/texturedMesh.h"

#include "voxigen/rendering/voxigen_gltext.h"

namespace voxigen
{


template<typename _RegionHandle>
class RegionRenderer
{
public:
    typedef _RegionHandle RegionHandle;
    typedef std::shared_ptr<RegionHandle> SharedRegionHandle;
    typedef typename _RegionHandle::RegionType Region;
    typedef typename Region::ChunkType Chunk;

    RegionRenderer();

    static glm::ivec3 getSize() { return details::regionCellSize<Region, Chunk>(); }

    RenderAction getAction() { return m_action; }
    void setAction(RenderAction action) { m_action=action; }

    void setHandle(SharedRegionHandle handle);
    SharedRegionHandle getHandle() const{ return m_region; }

    const glm::ivec3 &getRegionIndex() const { return m_region->getRegionIndex(); }
    glm::ivec3 getRegionCellSize() const { return details::regionCellSize<Region, Chunk>(); }

    void draw(const glm::ivec3 &offset);
    void drawInfo(const glm::mat4x4 &projectionViewMat, const glm::ivec3 &offset);
    void drawOutline(const glm::ivec3 &offset);

    void build();
    
    void clear();

    MeshBuffer setMesh(MeshBuffer &mesh);
    MeshBuffer clearMesh();

    static opengl_util::Program m_program;
    static std::string vertShader;
    static std::string fragmentShader;

    static size_t m_projectionViewId;
    static size_t m_offsetId;

    static opengl_util::Program m_outlineProgram;
    static std::string vertOutlineShader;
    static std::string fragmentOutlineShader;

    static size_t m_outlineProjectionViewId;
    static size_t m_outlineOffsetId;
    static size_t m_outlineColorId;
    static unsigned int m_outlineInstanceVertices;

    static void buildPrograms();
    static void useProgram();
    static void updateProgramProjection(const glm::mat4 &projection);
    static void useOutlineProgram();
    static void updateOutlineProgramProjection(const glm::mat4 &projection);

    unsigned int refCount;
private:
    void updateInfo();

    SharedRegionHandle m_region;
    RenderAction m_action;

    MeshBuffer m_meshBuffer;

    bool m_vertexArrayGen;
    unsigned int m_vertexArray;

    static bool m_outlineInstanceGen;
    bool m_outlineGen;
    unsigned int m_outlineVertexArray;
    unsigned int m_outlineOffsetVBO;

    static glm::mat4 m_infoMat;
    GLTtext *m_infoText;
};

/////////////////////////////////////////////////////////////////////////////////////////////
//Prep thread mesh code for SimpleChunkRenderer
/////////////////////////////////////////////////////////////////////////////////////////////
//namespace prep
//{
//template<typename _Grid>
//struct RequestMesh<_Grid, RegionRenderer<typename _Grid::RegionHandleType>>:public Request
//{
//    typedef typename _Grid::RegionHandleType RegionHandle;
//    typedef typename _Grid::SharedRegionHandle SharedRegionHandle;
//    typedef typename _Grid::RegionType Region;
//    typedef typename _Grid::ChunkType Chunk;
//
//    typedef RegionRenderer<RegionHandle> Renderer;
//
//    RequestMesh(Renderer const *renderer, TextureAtlas const *textureAtlas):Request(Mesh), renderer(renderer), textureAtlas(textureAtlas) {}
//
//    void process() override;
//
//    static TexturedMesh scratchMesh;
//    Renderer const *renderer;
//    TextureAtlas const *textureAtlas;
//    MeshBuffer mesh;
//};
//
//}//namespace prep

}//namespace voxigen

#include "simpleRegionRenderer.inl"

#endif //_voxigen_simpleRenderer_h_