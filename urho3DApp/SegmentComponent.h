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
    ///
    void SetOffset(Vector3 offset);
    ///
    void SegmentComponent<_Segment>::UpdateChunks(voxigen::ChunkHashSet &chunkSet)


private:
    /// Regenerate terrain geometry.
    void CreateGeometry();

    ///Segment Hash
    voxigen::SegmentHash segmentHash_;
    /// Shared index buffer.
    SharedPtr<IndexBuffer> indexBuffer_;
    ///
    Vector3 offset_;
    ///
    std::vector<ChunkDrawables *> chunks_;

    /// Terrain needs regeneration flag.
    bool recreateTerrain_;
};

}//Urho3D

#include "SegmentComponent.inl"
