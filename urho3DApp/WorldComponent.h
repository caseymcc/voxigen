#pragma once

#include "SegmentComponent.h"

#include "Urho3D/Container/ArrayPtr.h"
#include "Urho3D/Scene/Component.h"

#include <memory>
#include <vector>

namespace Urho3D
{

class Image;
class IndexBuffer;
class Material;
class Node;

/// Voxel World component.
template<typename _Grid>
class URHO3D_API WorldComponent: public Component
{
    URHO3D_OBJECT(WorldComponent, Component);

public:
    typedef _Grid GridType;
    typedef typename _Grid::SegmentType SegmentType;
    typedef typename _Grid::ChunkType ChunkType;

    typedef SegmentComponent<SegmentType> SegmentComponentType;
    typedef std::shared_ptr<SegmentComponentType> SharedSegmentComponent;
    typedef std::vector<SharedSegmentComponent> SharedSegmentComponents;

    /// Construct.
    WorldComponent(Context* context);
    /// Destruct.
    ~WorldComponent();

    /// Register object factory.
    static void RegisterObject(Context* context);
    /// Handle attribute write access.
    virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
    /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
    virtual void ApplyAttributes();
    /// Handle enabled/disabled state change.
    virtual void OnSetEnabled();
    ///
    void SetGrid(GridType *grid);
    ///
    voxigen::SegmentHash GetSegment();
    ///
    void SetSegment(voxigen::SegmentHash segment);
    ///
    float GetDrawDistance();
    ///
    void SetDrawDistance(float distance);
    ///
    bool UpdatePosition(Vector3 &position);
    ///
    bool UpdateGeometry();

private:
    GridType *grid_;

    SharedSegmentComponents segments_;

    voxigen::SegmentHash segment_;

    float viewRadius_;
    float viewRadiusMax_;
    std::vector<std::vector<glm::ivec3>> chunkIndices_;

    int chunkRadiusMax_;
    int chunkRadius_;
};

}//Urho3D

#include "WorldComponent.inl"
