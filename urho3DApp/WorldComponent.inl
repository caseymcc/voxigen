#include "Urho3D/Precompiled.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Core/Profiler.h"
#include "Urho3D/Graphics/DrawableEvents.h"
#include "Urho3D/Graphics/Geometry.h"
#include "Urho3D/Graphics/IndexBuffer.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Graphics/VertexBuffer.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Resource/Image.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Resource/ResourceEvents.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"

#include "Urho3D/DebugNew.h"

#include "WorldComponent.h"

#include <set>
#include <stack>

namespace Urho3D
{

template<typename _Grid>
WorldComponent<_Grid>::WorldComponent(Context* context) :
Component(context)
{
}

template<typename _Grid>
WorldComponent<_Grid>::~WorldComponent()
{
}

template<typename _Grid>
void WorldComponent<_Grid>::RegisterObject(Context* context)
{
    context->RegisterFactory<WorldComponent<_Grid>>(GEOMETRY_CATEGORY);

    URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
//    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Height Map", GetHeightMapAttr, SetHeightMapAttr, ResourceRef, ResourceRef(Image::GetTypeStatic()),
//        AM_DEFAULT);
//    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Material", GetMaterialAttr, SetMaterialAttr, ResourceRef, ResourceRef(Material::GetTypeStatic()),
//        AM_DEFAULT);
//    URHO3D_ATTRIBUTE("North Neighbor NodeID", unsigned, northID_, 0, AM_DEFAULT | AM_NODEID);
//    URHO3D_ATTRIBUTE("South Neighbor NodeID", unsigned, southID_, 0, AM_DEFAULT | AM_NODEID);
//    URHO3D_ATTRIBUTE("West Neighbor NodeID", unsigned, westID_, 0, AM_DEFAULT | AM_NODEID);
//    URHO3D_ATTRIBUTE("East Neighbor NodeID", unsigned, eastID_, 0, AM_DEFAULT | AM_NODEID);
//    URHO3D_ATTRIBUTE("Vertex Spacing", Vector3, spacing_, DEFAULT_SPACING, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Patch Size", GetPatchSize, SetPatchSizeAttr, int, DEFAULT_PATCH_SIZE, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Max LOD Levels", GetMaxLodLevels, SetMaxLodLevelsAttr, unsigned, MAX_LOD_LEVELS, AM_DEFAULT);
//    URHO3D_ATTRIBUTE("Smooth Height Map", bool, smoothing_, false, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Is Occluder", IsOccluder, SetOccluder, bool, false, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Can Be Occluded", IsOccludee, SetOccludee, bool, true, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Cast Shadows", GetCastShadows, SetCastShadows, bool, false, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Draw Distance", GetDrawDistance, SetDrawDistance, float, 0.0f, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Shadow Distance", GetShadowDistance, SetShadowDistance, float, 0.0f, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("LOD Bias", GetLodBias, SetLodBias, float, 1.0f, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Max Lights", GetMaxLights, SetMaxLights, unsigned, 0, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("View Mask", GetViewMask, SetViewMask, unsigned, DEFAULT_VIEWMASK, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Light Mask", GetLightMask, SetLightMask, unsigned, DEFAULT_LIGHTMASK, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Shadow Mask", GetShadowMask, SetShadowMask, unsigned, DEFAULT_SHADOWMASK, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Zone Mask", GetZoneMask, SetZoneMask, unsigned, DEFAULT_ZONEMASK, AM_DEFAULT);
//    URHO3D_ACCESSOR_ATTRIBUTE("Occlusion LOD level", GetOcclusionLodLevel, SetOcclusionLodLevelAttr, unsigned, M_MAX_UNSIGNED, AM_DEFAULT);
}

template<typename _Grid>
void WorldComponent<_Grid>::OnSetAttribute(const AttributeInfo& attr, const Variant& src)
{
    Serializable::OnSetAttribute(attr, src);

    // Change of any non-accessor attribute requires recreation of the terrain, or setting the neighbor terrains
    if (!attr.accessor_)
    {
//        if (attr.mode_ & AM_NODEID)
//            neighborsDirty_ = true;
//        else
//            recreateTerrain_ = true;
    }
}

template<typename _Grid>
void WorldComponent<_Grid>::ApplyAttributes()
{
//    if (recreateTerrain_)
//        CreateGeometry();
}

template<typename _Grid>
void WorldComponent<_Grid>::OnSetEnabled()
{
    bool enabled = IsEnabledEffective();

    for(size_t i=0; i<segments_.size(); ++i)
    {
        if(segments_[i])
            segments_[i]->SetEnabled(enabled);
    }
//    for (unsigned i = 0; i < patches_.Size(); ++i)
//    {
//        if (patches_[i])
//            patches_[i]->SetEnabled(enabled);
//    }
}

template<typename _Grid>
void WorldComponent<_Grid>::setGrid(GridType *grid)
{
    grid_=grid;
}

template<typename _Grid>
voxigen::SegmentHash WorldComponent<_Grid>::getSegment()
{
    return segment_;
}

template<typename _Grid>
void WorldComponent<_Grid>::setSegment(voxigen::SegmentHash segment)
{
    segment_=segment;
    updateGeometry();
}

template<typename _Grid>
bool WorldComponent<_Grid>::updatePosition(Vector3 &position)
{
    bool updateSegment=false;
    glm::ivec3 playerSegmentIndex=grid_->getSegmentIndex(segment_);
    glm::ivec3 segmentCellSize=grid_->segmentCellSize();

    if(position.x_<0)
    {
        playerSegmentIndex.x--;
        position.x_+=segmentCellSize.x;
        updateSegment=true;
    }
    else if(position.x_>segmentCellSize.x)
    {
        playerSegmentIndex.x++;
        position.x_-=segmentCellSize.x;
        updateSegment=true;
    }

    if(position.y_<0)
    {
        playerSegmentIndex.y--;
        position.y_+=segmentCellSize.y;
        updateSegment=true;
    }
    else if(position.y_>segmentCellSize.y)
    {
        playerSegmentIndex.y++;
        position.y_-=segmentCellSize.y;
        updateSegment=true;
    }

    if(position.z_<0)
    {
        playerSegmentIndex.z--;
        position.z_+=segmentCellSize.z;
        updateSegment=true;
    }
    else if(position.z_>segmentCellSize.z)
    {
        playerSegmentIndex.z++;
        position.z_-=segmentCellSize.z;
        updateSegment=true;
    }

    if(updateSegment)
    {
        unsigned int segmentHash=grid_->getSegmentHash(playerSegmentIndex);

        segment_=segmentHash;
        updateGeometry();
        return true;
    }
    return false;
}

template<typename _Grid>
bool WorldComponent<_Grid>::updateGeometry()
{
    glm::ivec3 currentSegmentIndex=grid_->getSegmentIndex(segment_);
    glm::ivec3 segmentIndex;

    std::set<voxigen::SegmentHash> neededSegments;
    std::stack<size_t> unusedSegments;

    //figure out segments needed
    for(size_t z=currentSegmentIndex.z-1; z<=currentSegmentIndex.z+1; ++z)
    {
        segmentIndex.z=z;
        for(size_t y=currentSegmentIndex.y-1; y<=currentSegmentIndex.y+1; ++y)
        {
            segmentIndex.y=y;
            for(size_t x=currentSegmentIndex.x-1; x<=currentSegmentIndex.x+1; ++x)
            {
                segmentIndex.x=x;
                neededSegments.insert(grid_->getSegmentHash(segmentIndex));
            }

        }

    }

    //remove segments that exists, and keep track of segments no longer in use
    for(size_t i=0; i<segments_.size(); ++i)
    {
        auto iter=neededSegments.find(segments_[i]->GetSegment());

        if(iter!=neededSegments.end())
            neededSegments.erase(iter);
        else
            unusedSegments.push(i);
    }

    //add segments that don't exist
    for(auto &hash:neededSegments)
    {
        if(!unusedSegments.empty())
        {
            size_t index=unusedSegments.top();
            unusedSegments.pop();

            segments_[index]->SetSegment(hash);
        }
        else
        {
            SharedSegmentComponent segment(new SegmentComponentType(GetContext()));

            segment->SetSegment(hash);
            segments_.push_back(segment);
        }
    }

    //hide segments not in use
    while(!unusedSegments.empty())
    {
        size_t index=unusedSegments.top();
        
        unusedSegments.pop();
        segments_[index]->SetEnabled(false);
    }

    return true;
}

}//namespace Urho3D
