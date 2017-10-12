#pragma once

#include "Urho3D/Graphics/IndexBuffer.h"
#include "Urho3D/Graphics/Drawable.h"

namespace Urho3D
{

class Geometry;
template<typename _Region>
class RegionComponent;
class VertexBuffer;


/// Individually rendered part grid
template<typename _Region, typename _Chunk>
class URHO3D_API ChunkDrawable: public Drawable
{
    URHO3D_OBJECT(ChunkDrawable, Drawable);

public:
    enum Status
    {
        Init,
        LoadingChunk,
        BuildingMesh,
        Ready
    };

    typedef _Chunk ChunkType;
    typedef typename _Region::ChunkHandleType ChunkHandle;
    typedef typename _Region::SharedChunkHandle SharedChunkHandle;

    /// Construct.
    ChunkDrawable(Context* context);
    /// Destruct.
    ~ChunkDrawable();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Process octree raycast. May be called from a worker thread.
    virtual void ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results);
    /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
    virtual void UpdateBatches(const FrameInfo& frame);
    /// Prepare geometry for rendering. Called from a worker thread if possible (no GPU update.)
    virtual void UpdateGeometry(const FrameInfo& frame);
    /// Return whether a geometry update is necessary, and if it can happen in a worker thread.
    virtual UpdateGeometryType GetUpdateGeometryType();
    /// Return the geometry for a specific LOD level.
    virtual Geometry* GetLodGeometry(unsigned batchIndex, unsigned level);
    /// Return number of occlusion geometry triangles.
    virtual unsigned GetNumOccluderTriangles();
    /// Draw to occlusion buffer. Return true if did not run out of triangles.
    virtual bool DrawOcclusion(OcclusionBuffer* buffer);
    /// Visualize the component as debug geometry.
    virtual void DrawDebugGeometry(DebugRenderer* debug, bool depthTest);

    ///
    voxigen::ChunkHash GetChunkHash();
    ///
    void SetChunk(SharedChunkHandle chunkHandle);

    Status GetStatus() { return status_; }

    ///Check to see if we need to update anything
    void OnUpdate();
    /// Set owner terrain.
    void SetOwner(RegionComponent<_Region> *terrain);
//    /// Set neighbor patches.
//    void SetNeighbors(ChunkDrawable* north, ChunkDrawable* south, ChunkDrawable* west, ChunkDrawable* east, ChunkDrawable* up, ChunkDrawable* down);
    /// Set material.
    void SetMaterial(Material* material);
    /// Set local-space bounding box.
    void SetBoundingBox(const BoundingBox& box);
    /// Set patch coordinates.
    void SetCoordinates(const IntVector2& coordinates);
    /// Reset to LOD level 0.
    void ResetLod();

    /// Return visible geometry.
    Geometry* GetGeometry() const;
    /// Return max LOD geometry. Used for decals.
    Geometry* GetMaxLodGeometry() const;
    /// Return geometry used for occlusion.
    Geometry* GetOcclusionGeometry() const;
    /// Return vertex buffer.
    VertexBuffer* GetVertexBuffer() const;
    /// Return owner terrain.
    RegionComponent<_Region>* GetOwner() const;

//    /// Return north neighbor patch.
//    ChunkDrawable* GetNorthPatch() const { return north_; }
//
//    /// Return south neighbor patch.
//    ChunkDrawable* GetSouthPatch() const { return south_; }
//
//    /// Return west neighbor patch.
//    ChunkDrawable* GetWestPatch() const { return west_; }
//
//    /// Return east neighbor patch.
//    ChunkDrawable* GetEastPatch() const { return east_; }

    /// Return geometrical error array.
    PODVector<float>& GetLodErrors() { return lodErrors_; }

    /// Return chunk coordinates.
    const IntVector2& GetCoordinates() const { return coordinates_; }

    /// Return current LOD level.
    unsigned GetLodLevel() const { return lodLevel_; }

    ///
    template<typename _Grid>
    void Update(_Grid *grid);

protected:
    /// Recalculate the world-space bounding box.
    virtual void OnWorldBoundingBoxUpdate();

private:
    ///
    void CreateGeometry();

    /// Return a corrected LOD level to ensure stitching can work correctly.
    unsigned GetCorrectedLodLevel(unsigned lodLevel);

    //voxel chunk
    SharedChunkHandle chunkHandle_;

    ///
    Status status_;
    ///
    bool updateGeometry_;
    /// Geometry.
    SharedPtr<Geometry> geometry_;
    /// Geometry that is locked to the max LOD level. Used for decals.
    SharedPtr<Geometry> maxLodGeometry_;
    /// Geometry that is used for occlusion.
    SharedPtr<Geometry> occlusionGeometry_;

    /// Index buffer.
    SharedPtr<IndexBuffer> indexBuffer_;
    /// Vertex buffer.
    SharedPtr<VertexBuffer> vertexBuffer_;
    /// Parent terrain.
    RegionComponent<_Region> *owner_;
//    /// North neighbor chunk.
//    WeakPtr<ChunkDrawable> north_;
//    /// South neighbor chunk.
//    WeakPtr<ChunkDrawable> south_;
//    /// West neighbor chunk.
//    WeakPtr<ChunkDrawable> west_;
//    /// East neighbor chunk.
//    WeakPtr<ChunkDrawable> east_;
//    /// Up neighbor chunk.
//    WeakPtr<ChunkDrawable> up_;
//    /// Down neighbor chunk.
//    WeakPtr<ChunkDrawable> down_;
    /// Geometrical error per LOD level.
    PODVector<float> lodErrors_;
    /// Patch coordinates in the terrain. (0,0) is the northwest corner.
    IntVector2 coordinates_;
    /// Current LOD level.
    unsigned lodLevel_;
};

}//namespace Urho3D

#include "ChunkDrawable.inl"
