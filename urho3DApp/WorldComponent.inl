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
#include "voxigen/chunkFunctions.h"

#include <set>
#include <stack>
#include <unordered_map>

#include <glm/ext.hpp>

namespace Urho3D
{

template<typename _Grid>
WorldComponent<_Grid>::WorldComponent(Context* context) :
Component(context),
chunkRadius_(0)
{
    UpdateSubscriptions();
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
    URHO3D_ACCESSOR_ATTRIBUTE("Draw Distance", GetDrawDistance, SetDrawDistance, float, 120.0f, AM_DEFAULT);

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
    UpdateSubscriptions();
}

template<typename _Grid>
void WorldComponent<_Grid>::UpdateSubscriptions()
{
    if(IsEnabled())
        SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(WorldComponent<_Grid>, OnSceneUpdate));
    else
        UnsubscribeFromEvent(Urho3D::E_UPDATE);
}

template<typename _Grid>
void WorldComponent<_Grid>::SetGrid(GridType *grid)
{
    grid_=grid;
}

template<typename _Grid>
voxigen::SegmentHash WorldComponent<_Grid>::GetSegment()
{
    return segmentHash_;
}

template<typename _Grid>
void WorldComponent<_Grid>::SetSegment(voxigen::SegmentHash segment)
{
    segmentHash_=segment;
}

template<typename _Grid>
float WorldComponent<_Grid>::GetDrawDistance() const
{
    return viewRadius_;
}

template<typename _Grid>
void WorldComponent<_Grid>::SetDrawDistance(float distance)
{
    viewRadius_=distance;
    viewRadiusMax_=distance*1.5f;

    glm::ivec3 chunkSize=grid_->getChunkSize();

    chunkRadiusMax_=std::ceil(distance/glm::compMax(chunkSize));

    chunkIndices_.clear();
    chunkIndices_.resize(chunkRadiusMax_);

    for(size_t i=0; i<chunkIndices_.size(); ++i)
        voxigen::ringCube<_Grid::ChunkType>(chunkIndices_[i], i);

//    chunkIndices_.clear();
//    std::vector<glm::ivec3> chunkIndices;
//
//    spiralCube<_Grid::ChunkType>(chunkIndices, viewRadiusMax_);
//
//    if(chunkIndices.empty()) //always want at least the current chunk
//        chunkIndices_.push_back(glm::ivec3(0, 0, 0));
//    else
//    {
//        chunkIndices_.resize(chunkIndices.size());
//        for(size_t i=0; i<chunkIndices.size(); ++i)
//        {
//            glm::ivec3 &index=chunkIndices[j];
//            chunkIndices_[i]=Vector3(index.x, index.y, index.z);
//        }
//    }

    chunkRadius_=0;
    UpdateGeometry();
}

template<typename _Grid>
bool WorldComponent<_Grid>::UpdatePosition(Vector3 &position)
{
    bool updateSegment=false;
    glm::ivec3 playerSegmentIndex=grid_->getSegmentIndex(segmentHash_);
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

    currentPosition_=glm::vec3(position.x_, position.y_, position.z_);

    if(updateSegment)
    {
        unsigned int segmentHash=grid_->getSegmentHash(playerSegmentIndex);

        segmentHash_=segmentHash;

        //reset from center
        chunkRadius_=0;
        UpdateGeometry();
        return true;
    }
    return false;
}

template<typename _Grid>
void WorldComponent<_Grid>::OnSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    std::vector<voxigen::Key> updatedChunks=grid_->getUpdatedChunks();

    if(!updatedChunks.empty())
    {
        std::unordered_map<voxigen::Key::SegmentHashType, voxigen::ChunkHashSet> map;

        for(voxigen::Key &key:updatedChunks)
            map[key.segmentHash].insert(key.chunkHash);

        for(size_t i=0; i<segments_.size(); ++i)
        {
            auto iter=map.find(segments_[i]->GetSegment());

            if(iter!=map.end())
                segments_[i]->UpdatedChunks(grid_, iter->second);
        }
    }

    UpdateGeometry();
}

struct SegmentInfo
{
    SegmentInfo():index(std::numeric_limits<size_t>::max()) {}
    SegmentInfo(size_t index):index(index) {}

    size_t index;
//    voxigen::ChunkHashSet updateChunks;
    voxigen::ChunkHashSet chunks;
};
typedef std::unordered_map<voxigen::Key::SegmentHashType, SegmentInfo> SegmentMap;

template<typename _Grid>
bool WorldComponent<_Grid>::UpdateGeometry()
{
    if(chunkRadius_>=chunkRadiusMax_)
        return false;

    glm::ivec3 playerSegmentIndex=grid_->getSegmentIndex(segmentHash_);
    glm::ivec3 playerChunkIndex=grid_->getChunkIndex(currentPosition_);
    std::vector<glm::ivec3> &chunkIndices=chunkIndices_[chunkRadius_];
    SegmentMap segments;
    glm::ivec3 index;
    glm::ivec3 currentSegmentIndex;
    
    for(size_t i=0; i<chunkIndices.size(); ++i)
    {
        index=playerChunkIndex+chunkIndices[i];
        currentSegmentIndex=playerSegmentIndex;

        glm::vec3 segmentOffset=grid_->getDescriptors().adjustSegment(currentSegmentIndex, index);
        voxigen::Key key=grid_->getHashes(currentSegmentIndex, index);

        segments[key.segmentHash].chunks.insert(key.chunkHash);
    }

    for(size_t i=0; i<segments_.size(); ++i)
    {
        auto iter=segments.find(segments_[i]->GetSegment());
    
        if(iter!=segments.end())
            iter->second.index=i;
    }

    for(auto iter:segments)
    {
        size_t index=iter.second.index;

        if(index==std::numeric_limits<size_t>::max())
        {
            SharedSegmentComponent segment(new SegmentComponentType(GetContext()));
            
            segment->SetSegment(iter.first);
            index=segments_.size();
            segments_.push_back(segment);
        }

        segments_[index]->UpdateChunks(grid_, iter.second.chunks);
    }

    chunkRadius_++;
    return true;
}

}//namespace Urho3D
