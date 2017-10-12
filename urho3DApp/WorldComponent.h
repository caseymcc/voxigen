#pragma once

#include "RegionComponent.h"

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
    typedef typename _Grid::RegionType RegionType;
    typedef typename _Grid::ChunkType ChunkType;

    typedef RegionComponent<RegionType> RegionComponentType;
    typedef std::shared_ptr<RegionComponentType> SharedRegionComponent;
    typedef std::vector<SharedRegionComponent> SharedRegionComponents;

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
    void UpdateSubscriptions();
    ///
    void SetGrid(GridType *grid);
    ///
    voxigen::RegionHash GetRegion();
    ///
    void SetRegion(voxigen::RegionHash region);
    ///
    float GetDrawDistance() const;
    ///
    void SetDrawDistance(float distance);
    ///
    bool UpdatePosition(Vector3 &position);
    ///
    void OnSceneUpdate(StringHash eventType, VariantMap &eventData);
    ///
    bool UpdateGeometry();

private:
    GridType *grid_;

    SharedRegionComponents regions_;

    voxigen::RegionHash regionHash_;
    glm::vec3 currentPosition_;

    float viewRadius_;
    float viewRadiusMax_;
    std::vector<std::vector<glm::ivec3>> chunkIndices_;

    int chunkRadiusMax_;
    int chunkRadius_;
};

}//Urho3D

#include "WorldComponent.inl"
