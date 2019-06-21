#ifndef _voxigen_simpleRenderer_h_
#define _voxigen_simpleRenderer_h_

#include "voxigen/rendererSettings.h"

#include "voxigen/voxigen_export.h"
#include "voxigen/initOpengl.h"
#include "voxigen/regularGrid.h"
#include "voxigen/simpleCamera.h"
#include "voxigen/simpleChunkRenderer.h"
#include "voxigen/simpleRegionRenderer.h"
#include "voxigen/simpleShapes.h"
#include "voxigen/object.h"
#include "voxigen/renderPrepThread.h"
#include "voxigen/renderCube.h"
#include "voxigen/voxigen_gltext.h"

#include <string>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opengl_util/program.h>
#include <deque>

namespace voxigen
{

//template<typename _ChunkRenderer>
//struct RegionRenderer
//{
//    typedef std::unordered_map<ChunkHash, _ChunkRenderer *> ChunkRendererMap;
//
//    RegionRenderer() {}
//    RegionRenderer(RegionHash hash, const glm::ivec3 &index, const glm::vec3 &offset):hash(hash), index(index), offset(offset) {}
////    RegionRenderer(RegionRenderer<_ChunkRenderer> &renderer):hash(renderer.hash), offset(renderer.offset) {}
//
//    RegionHash hash;
//    glm::ivec3 index;
//    glm::vec3 offset;
//    ChunkRendererMap chunkRenderers;
//};


/////////////////////////////////////////////////////////////////////////////////////////
//SimpleRenderer
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Grid>
class SimpleRenderer//:public RegularGridTypes<_Grid>
{
public:
    typedef typename _Grid::GridType GridType;
    typedef typename _Grid::DescriptorType DescriptorType;
    typedef typename _Grid::RegionType Region;
    typedef typename _Grid::RegionHandleType RegionHandle;
    typedef typename _Grid::ChunkType ChunkType;
    typedef typename _Grid::ChunkHandleType ChunkHandleType;
    typedef typename _Grid::SharedChunkHandle SharedChunkHandle;

    typedef SimpleChunkRenderer<Region, typename _Grid::ChunkType> ChunkRenderType;
    typedef SimpleChunkRenderer<Region, typename _Grid::ChunkType> ChunkRendererType;
    typedef std::unique_ptr<ChunkRenderType> UniqueChunkRenderer;

    typedef RegionRenderer<RegionHandle> RegionRendererType;
//    typedef typename RegionRendererType::ChunkRendererMap ChunkRendererMap;
//    typedef std::unordered_map<RegionHash, RegionRendererType> RegionRendererMap;

    typedef RegionChunkIndex<typename _Grid::RegionType, typename _Grid::ChunkType> RegionChunkIndexType;
    typedef RenderCube<GridType, ChunkRendererType, RegionChunkIndexType> RenderCubeType;
    typedef RegionIndex<typename _Grid::RegionType> RegionIndexType;
    typedef RenderCube<GridType, RegionRendererType, RegionIndexType> RegionRenderCubeType;

    typedef prep::RequestMesh<_Grid, ChunkRendererType> ChunkRequestMesh;
    typedef prep::RequestMesh<_Grid, RegionRendererType> RegionRequestMesh;

    SimpleRenderer(GridType *grid);
    ~SimpleRenderer();

    GridType *getGrid() { return m_grid; }

    void build();
    void destroy();
    
    void updateProjection(size_t width, size_t height);
    void updateView();
    
    void draw();
    void update(bool &regionsUpdated, bool &chunksUpdated);

    void setCamera(SimpleFpsCamera *camera);
    void setCameraChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);
    void setViewRadius(const glm::ivec3 &radius);
    void setPlayerChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex);
//    void updateChunks();

    void setTextureAtlas(SharedTextureAtlas textureAtlas) { m_textureAtlas=textureAtlas; m_textureAtlasDirty=true; }

    std::vector<ChunkRendererType *> getChunkRenderers();

////    void addPrepQueue(ChunkRenderType *chunkRenderer);
//    void addPrepQueue(const std::vector<ChunkRenderType *> &chunkRenderers);
//    void removePrepQueue(const std::vector<ChunkRenderType *> &chunkRenderers);

//    void startPrepThread();
//    void stopPrepThread();
//    void prepThread();

//    ChunkRenderType *createRenderNode(Key key);
//
//    bool updateCallback(SharedChunkHandle chunkHandle);
    void showRegions(bool show) { m_showRegions=show;  m_forceUpdate=true; };
    void showChunks(bool show) { m_showChunks=show;  m_forceUpdate=true; };
    void displayOutline(bool display) { m_displayOutline=display; m_forceUpdate=true; }
    bool isDisplayOutline() { return m_displayOutline; }

    void displayInfo(bool display)
    {
        if(m_displayOutline)
        {
            m_displayInfo=display;
            m_forceUpdate=true;
        }
    }
    bool isDisplayInfo() { return m_displayInfo; }

private:
    void updateChunkHandles(bool &regionsUpdated, bool &chunksUpdated);
    void updatePrepChunks();

//    void processAdd(typename RenderPrepThread::RequestAdd *request);
//    void processRemove(typename RenderPrepThread::RequestRemove *request);
    void processMesh(typename RenderPrepThread::Request *request);
    void processChunkMesh(ChunkRequestMesh *request);
    void processRegionMesh(RegionRequestMesh *request);

    RenderPrepThread m_renderPrepThread;
//    void rendererUpdateChunks();
//    void prepUpdateChunks(std::vector<ChunkRendererType *> &addRenderers, std::vector<ChunkRendererType *> &updateRenderers, std::vector<ChunkRendererType *> &removeRenderers);
//    void updateOcclusionQueries();

//    ChunkRenderType *getFreeRenderer();

    static std::string vertShader;
    static std::string fragmentShader;

    static std::string vertOutlineShader;
    static std::string fragmentOutlineShader;

    glm::ivec3 m_viewRadius;
    float m_viewRadiusMax; //keep renderer till it is outside of this range
    float m_viewLODDistance;
    int m_maxChunkRing;
    int m_currentQueryRing;
    bool m_queryComplete;

    SimpleFpsCamera *m_camera;
    Object m_playerPos;
    glm::vec3 m_lastUpdatePosition;
//    glm::ivec3 m_playerRegionIndex;
//    glm::ivec3 m_playerChunkIndex;
    RegionChunkIndexType m_playerIndex;

    RenderCubeType m_renderCube;
    RegionRenderCubeType m_regionRenderCube;

    gl::GLuint m_textureAtlasId;
    SharedTextureAtlas m_textureAtlas;
    bool m_textureAtlasDirty;


//    typedef std::vector<ChunkRenderType *> SearchRing;
//    typedef std::vector<SearchRing> SearchMap;
//    SearchMap m_searchMap;
//    
//    bool m_updateChunks;
    std::vector<prep::Request *> m_completedRequest;

    std::vector<ChunkRenderType *> m_addedChunkRenderers;
    std::vector<ChunkRenderType *> m_updatedChunkRenderers;
    std::vector<ChunkRenderType *> m_removedChunkRenderers;

    bool m_forceUpdate;
    bool m_showRegions;
    bool m_showChunks;
    bool m_displayOutline;
    bool m_displayInfo;

    GridType *m_grid;

    std::vector<std::vector<glm::ivec3>> m_chunkIndices;
#ifdef OLD_SEARCH
    std::vector<std::vector<ChunkRenderType *>> m_chunkQueryOrder;
#endif//OLD_SEARCH
    int m_outstandingChunkLoads;
    int m_outstandingChunkPreps;
    //    std::vector<ChunkRenderType> m_chunkRenderers;
    std::vector<UniqueChunkRenderer> m_chunkRenderers; //all allocated renderers

//update data structs
    std::mutex m_chunkRendererMapMutex;
    typedef std::unordered_map<Key::Type, ChunkRenderType *> ChunkRenderMap;
    ChunkRenderMap m_chunkRendererMap; //all allocated renderers

    std::vector<ChunkRenderType *> m_addChunkRenderer;
    std::vector<ChunkRenderType *> m_removeChunkRenderer;

//    RegionRendererMap m_regionRenderers;
//    std::vector<ChunkRenderType *> m_freeChunkRenderers; //renderers available for re-use

    //    RegionRendererMap m_rendererMap;
//    RendererMap m_rendererMap;

    opengl_util::Program m_program;
    size_t m_uniformProjectionViewId;
    opengl_util::Program m_outlineProgram;
    size_t m_uniformOutlintProjectionViewId;
    size_t m_outlineLightPositionId;
    size_t m_offsetId;
    size_t m_outlineOffsetId;
    size_t m_outlineStatusColor;

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
    NativeGL m_nativeGL;
    GLTtext *m_cameraInfo;

    //Status updates
    std::vector<ChunkRenderType *> m_chunksUpdated;

    unsigned int m_outlineInstanceVertices;
};

}//namespace voxigen

#include "simpleRenderer.inl"

#endif //_voxigen_simpleRenderer_h_