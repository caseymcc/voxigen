#pragma once

#include "Urho3D/Container/ArrayPtr.h"
#include "Urho3D/Scene/Component.h"

#include "ChunkDrawable.h"

#include <voxigen/defines.h>

namespace Urho3D
{

class Image;
class IndexBuffer;
class Material;
class Node;

/// Voxel Segment component.
template<typename _Segment>
class URHO3D_API SegmentComponent: public Component
{
    URHO3D_OBJECT(SegmentComponent, Component);

public:
    typedef _Segment SegmentType;
    typedef typename _Segment::ChunkType ChunkType;

    /// Construct.
    SegmentComponent(Context* context);
    /// Destruct.
    ~SegmentComponent();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Handle attribute write access.
    virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
    /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
    virtual void ApplyAttributes();
    /// Handle enabled/disabled state change.
    virtual void OnSetEnabled();
    /// Visualize the component as debug geometry.
    virtual void DrawDebugGeometry(DebugRenderer* debug, bool depthTest);

    ///
    voxigen::SegmentHash GetSegment();
    ///
    void SetSegment(voxigen::SegmentHash hash);

//    /// Set chunk quads per side. Must be a power of two.
//    void SetChunkSize(int size);
//    /// Set vertex (XZ) and height (Y) spacing.
//    void SetSpacing(const Vector3& spacing);
    /// Set maximum number of LOD levels for terrain chunks. This can be between 1-4.
    void SetMaxLodLevels(unsigned levels);
//    /// Set LOD level used for terrain chunk occlusion. By default (M_MAX_UNSIGNED) the coarsest. Since the LOD level used needs to be fixed, using finer LOD levels may result in false positive occlusion in cases where the actual rendered geometry is coarser, so use with caution.
//    void SetOcclusionLodLevel(unsigned level);
//    /// Set smoothing of heightmap.
//    void SetSmoothing(bool enable);
//    /// Set heightmap image. Dimensions should be a power of two + 1. Uses 8-bit grayscale, or optionally red as MSB and green as LSB for 16-bit accuracy. Return true if successful.
//    bool SetHeightMap(Image* image);
    /// Set material.
    void SetMaterial(Material* material);
//    /// Set north (positive Z) neighbor terrain for seamless LOD changes across terrains.
//    void SetNorthNeighbor(SegmentComponent* north);
//    /// Set south (negative Z) neighbor terrain for seamless LOD changes across terrains.
//    void SetSouthNeighbor(SegmentComponent* south);
//    /// Set west (negative X) neighbor terrain for seamless LOD changes across terrains.
//    void SetWestNeighbor(SegmentComponent* west);
//    /// Set east (positive X) neighbor terrain for seamless LOD changes across terrains.
//    void SetEastNeighbor(SegmentComponent* east);
//    /// Set all neighbor terrains at once.
//    void SetNeighbors(SegmentComponent* north, SegmentComponent* south, SegmentComponent* west, SegmentComponent* east);
    /// Set draw distance for chunks.
    void SetDrawDistance(float distance);
    /// Set shadow draw distance for chunks.
    void SetShadowDistance(float distance);
    /// Set LOD bias for chunks. Affects which terrain LOD to display.
    void SetLodBias(float bias);
    /// Set view mask for chunks. Is and'ed with camera's view mask to see if the object should be rendered.
    void SetViewMask(unsigned mask);
    /// Set light mask for chunks. Is and'ed with light's and zone's light mask to see if the object should be lit.
    void SetLightMask(unsigned mask);
    /// Set shadow mask for chunks. Is and'ed with light's light mask and zone's shadow mask to see if the object should be rendered to a shadow map.
    void SetShadowMask(unsigned mask);
    /// Set zone mask for chunks. Is and'ed with zone's zone mask to see if the object should belong to the zone.
    void SetZoneMask(unsigned mask);
    /// Set maximum number of per-pixel lights for chunks. Default 0 is unlimited.
    void SetMaxLights(unsigned num);
    /// Set shadowcaster flag for chunks.
    void SetCastShadows(bool enable);
    /// Set occlusion flag for chunks. Occlusion uses the coarsest LOD by default.
    void SetOccluder(bool enable);
    /// Set occludee flag for chunks.
    void SetOccludee(bool enable);
    /// Apply changes from the heightmap image.
    void ApplyHeightMap();

    /// Return chunk quads per side.
    int GetChunkSize() const { return chunkSize_; }

    /// Return vertex and height spacing.
    const Vector3& GetSpacing() const { return spacing_; }

    /// Return heightmap size in vertices.
    const IntVector2& GetNumVertices() const { return numVertices_; }

    /// Return heightmap size in chunks.
    const IntVector2& GetNumChunkes() const { return numChunkes_; }

    /// Return maximum number of LOD levels for terrain chunks. This can be between 1-4.
    unsigned GetMaxLodLevels() const { return maxLodLevels_; }
    
    /// Return LOD level used for occlusion.
    unsigned GetOcclusionLodLevel() const { return occlusionLodLevel_; }
    
    /// Return whether smoothing is in use.
    bool GetSmoothing() const { return smoothing_; }

    /// Return heightmap image.
    Image* GetHeightMap() const;
    /// Return material.
    Material* GetMaterial() const;
    /// Return chunk by index.
    ChunkDrawable<SegmentType, ChunkType>* GetChunk(unsigned index) const;
    /// Return chunk by chunk coordinates.
    ChunkDrawable<SegmentType, ChunkType>* GetChunk(int x, int z) const;
    /// Return chunk by chunk coordinates including neighbor terrains.
    ChunkDrawable<SegmentType, ChunkType>* GetNeighborChunk(int x, int z) const;
    /// Return height at world coordinates.
    float GetHeight(const Vector3& worldPosition) const;
    /// Return normal at world coordinates.
    Vector3 GetNormal(const Vector3& worldPosition) const;
    /// Convert world position to heightmap pixel position. Note that the internal height data representation is reversed vertically, but in the heightmap image north is at the top.
    IntVector2 WorldToHeightMap(const Vector3& worldPosition) const;

    /// Return north neighbor terrain.
    SegmentComponent<SegmentType>* GetNorthNeighbor() const { return north_; }
    
    /// Return south neighbor terrain.
    SegmentComponent<SegmentType>* GetSouthNeighbor() const { return south_; }
    
    /// Return west neighbor terrain.
    SegmentComponent<SegmentType>* GetWestNeighbor() const { return west_; }
    
    /// Return east neighbor terrain.
    SegmentComponent<SegmentType>* GetEastNeighbor() const { return east_; }

    /// Return raw height data.
    SharedArrayPtr<float> GetHeightData() const { return heightData_; }

    /// Return draw distance.
    float GetDrawDistance() const { return drawDistance_; }

    /// Return shadow draw distance.
    float GetShadowDistance() const { return shadowDistance_; }

    /// Return LOD bias.
    float GetLodBias() const { return lodBias_; }

    /// Return view mask.
    unsigned GetViewMask() const { return viewMask_; }

    /// Return light mask.
    unsigned GetLightMask() const { return lightMask_; }

    /// Return shadow mask.
    unsigned GetShadowMask() const { return shadowMask_; }

    /// Return zone mask.
    unsigned GetZoneMask() const { return zoneMask_; }

    /// Return maximum number of per-pixel lights.
    unsigned GetMaxLights() const { return maxLights_; }

    /// Return visible flag.
    bool IsVisible() const { return visible_; }

    /// Return shadowcaster flag.
    bool GetCastShadows() const { return castShadows_; }

    /// Return occluder flag.
    bool IsOccluder() const { return occluder_; }

    /// Return occludee flag.
    bool IsOccludee() const { return occludee_; }

    /// Regenerate chunk geometry.
    void CreateChunkGeometry(ChunkDrawable<SegmentType, ChunkType>* chunk);
    /// Update chunk based on LOD and neighbor LOD.
    void UpdateChunkLod(ChunkDrawable<SegmentType, ChunkType>* chunk);
    /// Set heightmap attribute.
    void SetHeightMapAttr(const ResourceRef& value);
    /// Set material attribute.
    void SetMaterialAttr(const ResourceRef& value);
    /// Set chunk size attribute.
    void SetChunkSizeAttr(int value);
    /// Set max LOD levels attribute.
    void SetMaxLodLevelsAttr(unsigned value);
    /// Set occlusion LOD level attribute.
    void SetOcclusionLodLevelAttr(unsigned value);
    /// Return heightmap attribute.
    ResourceRef GetHeightMapAttr() const;
    /// Return material attribute.
    ResourceRef GetMaterialAttr() const;

private:
    /// Regenerate terrain geometry.
    void CreateGeometry();
    /// Create index data shared by all chunks.
    void CreateIndexData();
    /// Return an uninterpolated terrain height value, clamping to edges.
    float GetRawHeight(int x, int z) const;
    /// Return a source terrain height value, clamping to edges. The source data is used for smoothing.
    float GetSourceHeight(int x, int z) const;
    /// Return interpolated height for a specific LOD level.
    float GetLodHeight(int x, int z, unsigned lodLevel) const;
    /// Get slope-based terrain normal at position.
    Vector3 GetRawNormal(int x, int z) const;
    /// Calculate LOD errors for a chunk.
    void CalculateLodErrors(ChunkDrawable<SegmentType, ChunkType>* chunk);
    /// Set neighbors for a chunk.
    void SetChunkNeighbors(ChunkDrawable<SegmentType, ChunkType>* chunk);
    /// Set heightmap image and optionally recreate the geometry immediately. Return true if successful.
    bool SetHeightMapInternal(Image* image, bool recreateNow);
    /// Handle heightmap image reload finished.
    void HandleHeightMapReloadFinished(StringHash eventType, VariantMap& eventData);
    /// Handle neighbor terrain geometry being created. Update the edge chunk neighbors as necessary.
    void HandleNeighborTerrainCreated(StringHash eventType, VariantMap& eventData);
    /// Update edge chunk neighbors when neighbor terrain(s) change or are recreated.
    void UpdateEdgeChunkNeighbors();

    ///Segment Hash
    voxigen::SegmentHash segment_;

    /// Shared index buffer.
    SharedPtr<IndexBuffer> indexBuffer_;
//    /// Heightmap image.
//    SharedPtr<Image> heightMap_;
//    /// Height data.
//    SharedArrayPtr<float> heightData_;
//    /// Source height data for smoothing.
//    SharedArrayPtr<float> sourceHeightData_;
    /// Material.
    SharedPtr<Material> material_;
    /// Terrain chunks.
    Vector<WeakPtr<ChunkDrawable<SegmentType, ChunkType>> > chunks_;
    /// Draw ranges for different LODs and stitching combinations.
    PODVector<Pair<unsigned, unsigned> > drawRanges_;
    /// North neighbor terrain.
    WeakPtr<SegmentComponent<SegmentType>> north_;
    /// South neighbor terrain.
    WeakPtr<SegmentComponent<SegmentType>> south_;
    /// West neighbor terrain.
    WeakPtr<SegmentComponent<SegmentType>> west_;
    /// East neighbor terrain.
    WeakPtr<SegmentComponent<SegmentType>> east_;
    /// Vertex and height spacing.
    Vector3 spacing_;
    /// Vertex and height sacing at the time of last update.
    Vector3 lastSpacing_;
    /// Origin of chunks on the XZ-plane.
    Vector2 chunkWorldOrigin_;
    /// Size of a chunk on the XZ-plane.
    Vector2 chunkWorldSize_;
    /// Terrain size in vertices.
    IntVector2 numVertices_;
    /// Terrain size in vertices at the time of last update.
    IntVector2 lastNumVertices_;
    /// Terrain size in chunks.
    IntVector2 numChunkes_;
    /// Chunk size, quads per side.
    int chunkSize_;
    /// Chunk size at the time of last update.
    int lastChunkSize_;
    /// Number of terrain LOD levels.
    unsigned numLodLevels_;
    /// Maximum number of LOD levels.
    unsigned maxLodLevels_;
    /// LOD level used for occlusion.
    unsigned occlusionLodLevel_;
    /// Smoothing enable flag.
    bool smoothing_;
    /// Visible flag.
    bool visible_;
    /// Shadowcaster flag.
    bool castShadows_;
    /// Occluder flag.
    bool occluder_;
    /// Occludee flag.
    bool occludee_;
    /// View mask.
    unsigned viewMask_;
    /// Light mask.
    unsigned lightMask_;
    /// Shadow mask.
    unsigned shadowMask_;
    /// Zone mask.
    unsigned zoneMask_;
    /// Draw distance.
    float drawDistance_;
    /// Shadow distance.
    float shadowDistance_;
    /// LOD bias.
    float lodBias_;
    /// Maximum lights.
    unsigned maxLights_;
    /// Node ID of north neighbor.
    unsigned northID_;
    /// Node ID of south neighbor.
    unsigned southID_;
    /// Node ID of west neighbor.
    unsigned westID_;
    /// Node ID of east neighbor.
    unsigned eastID_;
    /// Terrain needs regeneration flag.
    bool recreateTerrain_;
    /// Terrain neighbor attributes dirty flag.
    bool neighborsDirty_;
};

}//Urho3D

#include "SegmentComponent.inl"
