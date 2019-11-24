#ifndef _voxigen_simpleChunkRenderer_h_
#define _voxigen_simpleChunkRenderer_h_

#include "voxigen/rendererSettings.h"

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/chunkHandle.h"
#include "voxigen/chunkInfo.h"
#include "voxigen/cubicMeshBuilder.h"
#include "voxigen/textureAtlas.h"
#include "voxigen/chunkTextureMesh.h"
#include "voxigen/renderAction.h"
#include "voxigen/renderPrepThread.h"
#include "voxigen/voxigen_gltext.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <opengl_util/program.h>

namespace voxigen
{

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleChunkRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Region, typename _Chunk>
class SimpleChunkRenderer
{
public:
    typedef _Region RegionType;
    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    SimpleChunkRenderer();
    ~SimpleChunkRenderer();
    
    enum State
    {
        Init,
        Invalid,
        Occluded,
        Query, 
        QueryWait,
        Dirty,
        Copy,
        Built,
        Empty
    };

    static void buildPrograms();
    static void useProgram();
    static void updateProgramProjection(const glm::mat4 &projection);
    static void useOutlineProgram();
    static void updateOutlineProgramProjection(const glm::mat4 &projection);

    static glm::ivec3 getSize() { return glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value); }

    State getState() { return m_state; }
    void setDirty() { m_state=Dirty; }
    
    RenderAction getAction(){ return m_action; }
    void setAction(RenderAction action){m_action=action;}
    

    Key getKey(){return Key(m_chunkHandle->regionHash(), m_chunkHandle->hash());}

    void setChunk(SharedChunkHandle chunk);
    void setHandle(SharedChunkHandle chunk);
    SharedChunkHandle getHandle() { return m_chunkHandle; }

    void setTextureAtlas(SharedTextureAtlas textureAtlas) { m_textureAtlas=textureAtlas; }
    void setEmpty();
    const glm::vec3 &getChunkOffset() { return m_chunkOffset; }

    void build();

//    void buildMesh();
    MeshBuffer clearMesh();
    MeshBuffer setMesh(MeshBuffer &mesh);
    bool update();
    void updated();
    void updateInfo(std::string &value);

    void updateOutline();
    void invalidate();

    void releaseChunkMemory();

    bool incrementCopy();
    
    void draw(const glm::ivec3 &offset);
    void drawInfo(const glm::mat4x4 &projectionViewMat, const glm::ivec3 &offset);

    void startOcculsionQuery();
    void drawOcculsionQuery();
    bool checkOcculsionQuery(unsigned int &samples);

//#ifndef NDEBUG
    void drawOutline(const glm::ivec3 &offset);
//#endif //NDEBUG

//    const RegionChunkIndex<RegionType, ChunkType> getIndex() { RegionChunkIndex<RegionType, ChunkType> index; index.regionIndex=m_chunkHandle->regionIndex(); index.chunkIndex=m_chunkHandle->chunkIndex(); return index; }

    RegionHash getRegionHash() const { return m_chunkHandle->regionHash(); }
    const glm::ivec3 &getRegionIndex() const { return m_chunkHandle->regionIndex(); }
    ChunkHash getChunkHash() const { return m_chunkHandle->hash(); }
    const glm::ivec3 &getChunkIndex() const { return m_chunkHandle->chunkIndex(); }
    SharedChunkHandle getChunkHandle() const { return m_chunkHandle; }
//    const glm::ivec3 &getPosition() { return m_chunkHandle->chunk()->getPosition(); }
    const glm::ivec3 &getGridOffset() const { return m_chunkHandle->regionOffset();/* m_chunkHandle->chunk->getGridOffset();*/ }
    
    glm::ivec3 getRegionCellSize() const { return details::regionCellSize<_Region, _Chunk>(); }

    MeshBuffer &getMeshBuffer() { return m_meshBuffer; }
    size_t getLod() const { return m_lod; }
    void setLod(size_t lod);

    void clear();

    float distanceTo(const glm::ivec3 &region, const glm::ivec3 &chunk)
    {
        return details::distance<RegionType, ChunkType>(m_chunkHandle->regionIndex(), m_chunkHandle->chunkIndex(), region, chunk);
    }

    unsigned int refCount;

private:
    void updateInfoText();
    void calculateMemoryUsed();

//    RenderType *m_parent;

    State m_state;
    RenderAction m_action;
    SharedChunkHandle m_chunkHandle;
    SharedTextureAtlas m_textureAtlas;

    static std::string vertShader;
    static std::string fragmentShader;
    static opengl_util::Program m_program;
    static size_t m_projectionViewId;
    static size_t m_offsetId;

    static std::string vertOutlineShader;
    static std::string fragmentOutlineShader;
    static opengl_util::Program m_outlineProgram;
    static size_t m_outlineProjectionViewId;
    static size_t m_outlineOffsetId;
    static size_t m_outlineColorId;
    static bool m_outlineInstanceGen;
    static unsigned int m_outlineInstanceVertices;

//#ifndef NDEBUG
    bool m_outlineBuilt;
    GLTtext *m_infoText;
    GLTtext *m_infoIndexText;
//#endif
    unsigned int m_queryId;

    MeshBuffer m_meshBuffer;
    glm::vec3 m_chunkOffset;

    bool m_vertexArrayGen;
    unsigned int m_vertexArray;
    unsigned int m_offsetVBO;
    std::vector<glm::vec4> m_ChunkInfoOffset;

    bool m_lodUpdated;
    size_t m_lod;

    size_t m_memoryUsed;
//    ChunkTextureMesh m_mesh;

    gl::GLsync m_vertexBufferSync;
    int m_delayedFrames;

    bool m_outlineGen;
    unsigned int m_outlineVertexArray;
    unsigned int m_outlineOffsetVBO;
};

/////////////////////////////////////////////////////////////////////////////////////////////
//Prep thread mesh code for SimpleChunkRenderer
/////////////////////////////////////////////////////////////////////////////////////////////
//namespace prep
//{
//template<typename _Grid>
//struct RequestMesh<_Grid, SimpleChunkRenderer<typename _Grid::RegionType, typename _Grid::ChunkType>>:public Request
//{
//    typedef typename _Grid::RegionType Region;
//    typedef typename _Grid::ChunkType Chunk;
//    typedef typename _Grid::SharedChunkHandle SharedChunkHandle;
//    typedef SimpleChunkRenderer<Region, Chunk> ChunkRenderer;
//
//    RequestMesh(ChunkRenderer const *renderer, TextureAtlas const *textureAtlas):Request(Mesh), renderer(renderer), textureAtlas(textureAtlas) {}
//
//    void process() override;
//
//    static ChunkTextureMesh scratchMesh;
//    ChunkRenderer const *renderer;
//    TextureAtlas const *textureAtlas;
//    MeshBuffer mesh;
//};
//
//}//namespace prep

}//namespace voxigen

#include "simpleChunkRenderer.inl"

#endif //_voxigen_simpleChunkRenderer_h_