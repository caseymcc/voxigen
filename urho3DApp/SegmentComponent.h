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

/// Voxel Region component.
template<typename _Region>
class URHO3D_API RegionComponent: public Component
{
    URHO3D_OBJECT(RegionComponent, Component);

public:
    typedef _Region RegionType;
    typedef typename _Region::ChunkType ChunkType;
    typedef typename _Region::SharedChunkHandle SharedChunkHandle;
    typedef ChunkDrawable<RegionType, ChunkType> ChunkDrawable;

    /// Construct.
    RegionComponent(Context* context);
    /// Destruct.
    ~RegionComponent();
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
    voxigen::RegionHash GetRegion();
    ///
    void SetRegion(voxigen::RegionHash hash);
    ///
    void SetOffset(Vector3 offset);

    /// Chunks have been updated in some manner outside main thread
    template<typename _Grid>
    void UpdatedChunks(_Grid *grid, voxigen::ChunkHashSet &chunkHashSet);

    /// Chunks need to be updated
    template<typename _Grid>
    void UpdateChunks(_Grid *grid, voxigen::ChunkHashSet &chunkHashSet);


private:
    /// Regenerate terrain geometry.
    void CreateGeometry();

    ///Region Hash
    voxigen::RegionHash regionHash_;
    /// Shared index buffer.
    SharedPtr<IndexBuffer> indexBuffer_;
    ///
    Vector3 offset_;
    ///
    std::vector<ChunkDrawable *> chunkDrawables_;

    /// Terrain needs regeneration flag.
    bool recreateTerrain_;
};

}//Urho3D

#include "RegionComponent.inl"
