#ifndef _voxigen_simpleRenderer_h_
#define _voxigen_simpleRenderer_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/initGlew.h"
#include "voxigen/regularGrid.h"
#include "voxigen/SimpleCamera.h"
#include "voxigen/SimpleChunkRenderer.h"
#include "voxigen/simpleShapes.h"

#include <string>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opengl_util/program.h>

#ifdef _WINDOWS
#include "windows.h"
#endif

namespace voxigen
{

template<typename _ChunkRenderer>
struct SegmentRenderer
{
    typedef std::unordered_map<ChunkHash, _ChunkRenderer *> ChunkRendererMap;

    SegmentRenderer() {}
    SegmentRenderer(SegmentHash hash, glm::ivec3 &index, glm::vec3 &offset):hash(hash), index(index), offset(offset) {}
//    SegmentRenderer(SegmentRenderer<_ChunkRenderer> &renderer):hash(renderer.hash), offset(renderer.offset) {}

    SegmentHash hash;
    glm::ivec3 index;
    glm::vec3 offset;
    ChunkRendererMap chunkRenderers;
};

/////////////////////////////////////////////////////////////////////////////////////////
//SimpleRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Grid>
class SimpleRenderer
{
public:
    typedef _Grid GridType;
    typedef typename GridType::ChunkType ChunkType;
    typedef typename _Grid::SharedChunkHandle SharedChunkHandle;
    typedef SimpleChunkRenderer<SimpleRenderer, typename _Grid::ChunkType> ChunkRenderType;
    typedef std::unique_ptr<ChunkRenderType> UniqueChunkRenderer;

    //    typedef std::unordered_map<ChunkHash, size_t> ChunkRendererMap;
    //    typedef std::unordered_map<SegmentHash, ChunkRendererMap> SegmentRendererMap;
//    typedef std::unordered_map<unsigned __int64, size_t> RendererMap;
    
    
    typedef SegmentRenderer<ChunkRenderType> SegmentRendererType;
    typedef typename SegmentRendererType::ChunkRendererMap ChunkRendererMap;
    typedef std::unordered_map<SegmentHash, SegmentRendererType> SegmentRendererMap;

    SimpleRenderer(GridType *grid);
    ~SimpleRenderer();

    GridType *getGrid() { return m_grid; }

    void build();
    void destroy();
    void update();
    void updateProjection(size_t width, size_t height);
    void updateView();
    void draw();

    void setCamera(SimpleFpsCamera *camera);
    void setViewRadius(float radius);
    void updateChunks();

    void addPrepQueue(ChunkRenderType *chunkRenderer);
    void startPrepThread();
    void stopPrepThread();
    void prepThread();

private:
    void spiralCube(std::vector<glm::ivec3> &positions, float radius);

    ChunkRenderType *getFreeRenderer();

    static std::string vertShader;
    static std::string fragmentShader;

    static std::string vertOutlineShader;
    static std::string fragmentOutlineShader;

    float m_viewRadius;
    float m_viewRadiusMax; //keep renderer till it is outside of this range
    SimpleFpsCamera *m_camera;
    glm::vec3 m_lastUpdatePosition;

    bool m_outlineChunks;

    GridType *m_grid;

    std::vector<glm::ivec3> m_chunkIndices;
    //    std::vector<ChunkRenderType> m_chunkRenderers;
    std::vector<UniqueChunkRenderer> m_chunkRenderers; //all allocated renderers

    SegmentRendererMap m_segmentRenderers;
    std::vector<ChunkRenderType *> m_freeChunkRenderers; //renderers available for re-use

    //    SegmentRendererMap m_rendererMap;
//    RendererMap m_rendererMap;

    opengl_util::Program m_program;
    size_t m_uniformProjectionViewId;
    opengl_util::Program m_outlineProgram;
    size_t m_uniformOutlintProjectionViewId;
    size_t m_outlineLightPositionId;
    size_t m_offsetId;
    size_t m_outlineOffsetId;

    size_t m_lightPositionId;
    size_t m_lighColorId;

    bool m_projectionViewMatUpdated;
    glm::mat4 m_projectionViewMat;
    glm::mat4 m_projectionMat;
    glm::mat4 m_viewMat;

    unsigned int m_instanceSquareVertices;
    unsigned int m_instanceVertices;
    unsigned int m_instanceTexCoords;

    //render prep thread/queue
#ifdef _WINDOWS
    HDC m_prepDC;
    HGLRC m_prepGlContext;
#else
#endif
    std::mutex m_prepMutex;
    std::thread m_prepThread;
    std::queue<ChunkRenderType *> m_prepQueue;
    std::condition_variable m_prepEvent;
    bool m_prepThreadRun;

    //Status updates
//    std::mutex m_chunkUpdatedMutex;
//    std::vector<Key> m_chunksUpdated;
    std::vector<ChunkRenderType *> m_chunksUpdated;

#ifndef NDEBUG
    unsigned int m_outlineInstanceVertices;
#endif //NDEBUG
};

}//namespace voxigen

#include "simpleRenderer.inl"

#endif //_voxigen_simpleRenderer_h_