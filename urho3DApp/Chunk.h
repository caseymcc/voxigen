#pragma once

#include "../Graphics/Drawable.h"

namespace Urho3D
{

class Geometry;
class World;
class VertexBuffer;

/// Individually rendered part of a heightmap terrain.
class URHO3D_API Chunk: public Drawable
{
    URHO3D_OBJECT(Chunk, Drawable);

public:
    /// Construct.
    Chunk(Context* context);
    /// Destruct.
    ~Chunk();
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

    /// Set owner terrain.
    void SetOwner(World* terrain);
    /// Set neighbor patches.
    void SetNeighbors(Chunk* north, Chunk* south, Chunk* west, Chunk* east, Chunk* up, Chunk* down);
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
    World* GetOwner() const;

    /// Return north neighbor patch.
    Chunk* GetNorthPatch() const { return north_; }

    /// Return south neighbor patch.
    Chunk* GetSouthPatch() const { return south_; }

    /// Return west neighbor patch.
    Chunk* GetWestPatch() const { return west_; }

    /// Return east neighbor patch.
    Chunk* GetEastPatch() const { return east_; }

    /// Return geometrical error array.
    PODVector<float>& GetLodErrors() { return lodErrors_; }

    /// Return chunk coordinates.
    const IntVector2& GetCoordinates() const { return coordinates_; }

    /// Return current LOD level.
    unsigned GetLodLevel() const { return lodLevel_; }

protected:
    /// Recalculate the world-space bounding box.
    virtual void OnWorldBoundingBoxUpdate();

private:
    /// Return a corrected LOD level to ensure stitching can work correctly.
    unsigned GetCorrectedLodLevel(unsigned lodLevel);

    /// Geometry.
    SharedPtr<Geometry> geometry_;
    /// Geometry that is locked to the max LOD level. Used for decals.
    SharedPtr<Geometry> maxLodGeometry_;
    /// Geometry that is used for occlusion.
    SharedPtr<Geometry> occlusionGeometry_;
    /// Vertex buffer.
    SharedPtr<VertexBuffer> vertexBuffer_;
    /// Parent terrain.
    WeakPtr<World> owner_;
    /// North neighbor chunk.
    WeakPtr<Chunk> north_;
    /// South neighbor chunk.
    WeakPtr<Chunk> south_;
    /// West neighbor chunk.
    WeakPtr<Chunk> west_;
    /// East neighbor chunk.
    WeakPtr<Chunk> east_;
    /// Up neighbor chunk.
    WeakPtr<Chunk> up_;
    /// Down neighbor chunk.
    WeakPtr<Chunk> down_;
    /// Geometrical error per LOD level.
    PODVector<float> lodErrors_;
    /// Patch coordinates in the terrain. (0,0) is the northwest corner.
    IntVector2 coordinates_;
    /// Current LOD level.
    unsigned lodLevel_;
};

}
