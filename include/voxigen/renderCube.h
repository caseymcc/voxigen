#ifndef _voxigen_renderCube_h_
#define _voxigen_renderCube_h

#include "voxigen/regularGrid.h"
#include "voxigen/freeQueue.h"

#include <memory>

namespace voxigen
{

//enum class RenderState
//{
//    Idle,
//    RequestRenderer,
//    Meshing,
//    Updating
//};
//
//template<typename _SharedChunkHandle, typename _ChunkRenderer>
//struct ChunkRenderInfo
//{
//    typedef _SharedChunkHandle SharedChunkHandle;
//    typedef _ChunkRenderer ChunkRenderer;
//
//    ChunkRenderInfo():state(RenderState::Idle), renderer(nullptr) {}
//
//    void release() { state=RenderState::Idle; chunkHandle.reset(); renderer=nullptr; }
//
//    RenderState state;
//    SharedChunkHandle chunkHandle;
//    ChunkRenderer *renderer;
//};

/////////////////////////////////////////////////////////////////////////////////////////
//RenderCube
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Grid, typename _ChunkRenderer>
class RenderCube//:public RegularGridTypes<_Grid>
{
public:
    typedef typename _Grid::GridType GridType;
    typedef typename _Grid::DescriptorType DescriptorType;
    typedef typename _Grid::ChunkType ChunkType;
    typedef typename _Grid::ChunkHandleType ChunkHandleType;
    typedef typename _Grid::SharedChunkHandle SharedChunkHandle;

    typedef _ChunkRenderer ChunkRendererType;
    typedef std::vector<ChunkRendererType> ChunkRenderers;
//    typedef ChunkRenderInfo<SharedChunkHandle, _ChunkRenderer> ChunkRenderInfoType;

    typedef RenderPrepThread<_Grid, _ChunkRenderer> RenderPrepThreadType;

    RenderCube(GridType *grid, DescriptorType *descriptors, RenderPrepThreadType *prepThread);
    ~RenderCube();

    void setViewRadius(const glm::ivec3 &radius);
    void setOutlineInstance(unsigned int outlineInstanceId);

    void init(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);
    void updateCamera(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);
    void update(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);

    void draw(opengl_util::Program *program, size_t offsetId);
    void drawOutline(opengl_util::Program *program, size_t offsetId, size_t colorId);

//    ChunkRenderInfoType *getChunkRenderInfo(const Key &key);
    ChunkRendererType *getChunkRenderInfo(const Key &key);


private:
    glm::ivec3 calcCubeSize(const glm::ivec3 &radius);

    ChunkRendererType *getFreeRenderer();
    void releaseRenderer(ChunkRendererType *renderer);

//    void updateRegion(glm::ivec3 &startRegionIndex, glm::ivec3 &startChunkIndex, glm::ivec3 &size);
    void releaseRegion(const glm::ivec3 &start, const glm::ivec3 &size);
    void getRegion(const glm::ivec3 &start, const glm::ivec3 &startRegionIndex, const glm::ivec3 &startChunkIndex, const glm::ivec3 &size);

//    void releaseChunkInfo(ChunkRenderInfoType &renderInfo);
    void releaseChunkInfo(ChunkRendererType *renderInfo);
    
    GridType *m_grid;
    const DescriptorType *m_descriptors;
    RenderPrepThreadType *m_renderPrepThread;

    glm::ivec3 m_viewRadius;
    unsigned int m_outlineInstanceId;

    glm::ivec3 m_cameraRegionIndex;
    glm::ivec3 m_cameraChunkIndex;

    glm::ivec3 m_regionIndex;
    glm::ivec3 m_chunkIndex;

//    std::unordered_map<Key::Type, size_t> m_renderCubeMap;
//    std::vector<ChunkRenderInfoType> m_renderCube;
    std::vector<ChunkRendererType *> m_renderCube;
    glm::ivec3 m_renderCubeSize;
    glm::ivec3 m_renderCubeCenterIndex;

    std::vector<ChunkRendererType *>m_rendererReleaseQueue;
    FreeQueue<ChunkRendererType> m_rendererQueue;
};

}//namespace voxigen

#include "renderCube.inl"

#endif //_voxigen_renderCube_h