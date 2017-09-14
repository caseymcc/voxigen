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

#include "SegmentComponent.h"
#include "ChunkDrawable.h"

namespace Urho3D
{

extern const char* GEOMETRY_CATEGORY;

static const Vector3 DEFAULT_SPACING(1.0f, 0.25f, 1.0f);
static const unsigned MIN_LOD_LEVELS = 1;
static const unsigned MAX_LOD_LEVELS = 4;
static const int DEFAULT_PATCH_SIZE = 32;
static const int MIN_PATCH_SIZE = 4;
static const int MAX_PATCH_SIZE = 128;
static const unsigned STITCH_NORTH = 1;
static const unsigned STITCH_SOUTH = 2;
static const unsigned STITCH_WEST = 4;
static const unsigned STITCH_EAST = 8;

inline void GrowUpdateRegion(IntRect& updateRegion, int x, int y)
{
    if (updateRegion.left_ < 0)
    {
        updateRegion.left_ = updateRegion.right_ = x;
        updateRegion.top_ = updateRegion.bottom_ = y;
    }
    else
    {
        if (x < updateRegion.left_)
            updateRegion.left_ = x;
        if (x > updateRegion.right_)
            updateRegion.right_ = x;
        if (y < updateRegion.top_)
            updateRegion.top_ = y;
        if (y > updateRegion.bottom_)
            updateRegion.bottom_ = y;
    }
}

template<typename _Segment>
SegmentComponent<_Segment>::SegmentComponent(Context* context) :
Component(context),
indexBuffer_(new IndexBuffer(context)),
spacing_(DEFAULT_SPACING),
lastSpacing_(Vector3::ZERO),
//patchWorldOrigin_(Vector2::ZERO),
//patchWorldSize_(Vector2::ZERO),
numVertices_(IntVector2::ZERO),
lastNumVertices_(IntVector2::ZERO),
//numPatches_(IntVector2::ZERO),
//patchSize_(DEFAULT_PATCH_SIZE),
//lastPatchSize_(0),
numLodLevels_(1),
maxLodLevels_(MAX_LOD_LEVELS),
occlusionLodLevel_(M_MAX_UNSIGNED),
smoothing_(false),
visible_(true),
castShadows_(false),
occluder_(false),
occludee_(true),
viewMask_(DEFAULT_VIEWMASK),
lightMask_(DEFAULT_LIGHTMASK),
shadowMask_(DEFAULT_SHADOWMASK),
zoneMask_(DEFAULT_ZONEMASK),
drawDistance_(0.0f),
shadowDistance_(0.0f),
lodBias_(1.0f),
maxLights_(0),
northID_(0),
southID_(0),
westID_(0),
eastID_(0),
recreateTerrain_(false),
neighborsDirty_(false)
{
    indexBuffer_->SetShadowed(true);
}

template<typename _Segment>
SegmentComponent<_Segment>::~SegmentComponent()
{
}

template<typename _Segment>
void SegmentComponent<_Segment>::RegisterObject(Context* context)
{
    context->RegisterFactory<SegmentComponent>(GEOMETRY_CATEGORY);

    URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Height Map", GetHeightMapAttr, SetHeightMapAttr, ResourceRef, ResourceRef(Image::GetTypeStatic()),
        AM_DEFAULT);
    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Material", GetMaterialAttr, SetMaterialAttr, ResourceRef, ResourceRef(Material::GetTypeStatic()),
        AM_DEFAULT);
    URHO3D_ATTRIBUTE("North Neighbor NodeID", unsigned, northID_, 0, AM_DEFAULT | AM_NODEID);
    URHO3D_ATTRIBUTE("South Neighbor NodeID", unsigned, southID_, 0, AM_DEFAULT | AM_NODEID);
    URHO3D_ATTRIBUTE("West Neighbor NodeID", unsigned, westID_, 0, AM_DEFAULT | AM_NODEID);
    URHO3D_ATTRIBUTE("East Neighbor NodeID", unsigned, eastID_, 0, AM_DEFAULT | AM_NODEID);
    URHO3D_ATTRIBUTE("Vertex Spacing", Vector3, spacing_, DEFAULT_SPACING, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Patch Size", GetPatchSize, SetPatchSizeAttr, int, DEFAULT_PATCH_SIZE, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Max LOD Levels", GetMaxLodLevels, SetMaxLodLevelsAttr, unsigned, MAX_LOD_LEVELS, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Smooth Height Map", bool, smoothing_, false, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Is Occluder", IsOccluder, SetOccluder, bool, false, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Can Be Occluded", IsOccludee, SetOccludee, bool, true, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Cast Shadows", GetCastShadows, SetCastShadows, bool, false, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Draw Distance", GetDrawDistance, SetDrawDistance, float, 0.0f, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Shadow Distance", GetShadowDistance, SetShadowDistance, float, 0.0f, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("LOD Bias", GetLodBias, SetLodBias, float, 1.0f, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Max Lights", GetMaxLights, SetMaxLights, unsigned, 0, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("View Mask", GetViewMask, SetViewMask, unsigned, DEFAULT_VIEWMASK, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Light Mask", GetLightMask, SetLightMask, unsigned, DEFAULT_LIGHTMASK, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Shadow Mask", GetShadowMask, SetShadowMask, unsigned, DEFAULT_SHADOWMASK, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Zone Mask", GetZoneMask, SetZoneMask, unsigned, DEFAULT_ZONEMASK, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Occlusion LOD level", GetOcclusionLodLevel, SetOcclusionLodLevelAttr, unsigned, M_MAX_UNSIGNED, AM_DEFAULT);
}

template<typename _Segment>
void SegmentComponent<_Segment>::OnSetAttribute(const AttributeInfo& attr, const Variant& src)
{
    Serializable::OnSetAttribute(attr, src);

    // Change of any non-accessor attribute requires recreation of the terrain, or setting the neighbor terrains
    if (!attr.accessor_)
    {
        if (attr.mode_ & AM_NODEID)
            neighborsDirty_ = true;
        else
            recreateTerrain_ = true;
    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::ApplyAttributes()
{
    if (recreateTerrain_)
        CreateGeometry();

    if (neighborsDirty_)
    {
        Scene* scene = GetScene();
        Node* north = scene ? scene->GetNode(northID_) : (Node*)0;
        Node* south = scene ? scene->GetNode(southID_) : (Node*)0;
        Node* west = scene ? scene->GetNode(westID_) : (Node*)0;
        Node* east = scene ? scene->GetNode(eastID_) : (Node*)0;
//        SegmentComponent* northTerrain = north ? north->GetComponent<SegmentComponent>() : (SegmentComponent*)0;
//        SegmentComponent* southTerrain = south ? south->GetComponent<SegmentComponent>() : (SegmentComponent*)0;
//        SegmentComponent* westTerrain = west ? west->GetComponent<SegmentComponent>() : (SegmentComponent*)0;
//        SegmentComponent* eastTerrain = east ? east->GetComponent<SegmentComponent>() : (SegmentComponent*)0;
//        SetNeighbors(northTerrain, southTerrain, westTerrain, eastTerrain);
        neighborsDirty_ = false;
    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::OnSetEnabled()
{
    bool enabled = IsEnabledEffective();

    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetEnabled(enabled);
    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::DrawDebugGeometry(DebugRenderer* debug, bool depthTest)
{
}

template<typename _Segment>
voxigen::SegmentHash SegmentComponent<_Segment>::GetSegment()
{
    return segment_;
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetSegment(voxigen::SegmentHash hash)
{
    segment_=hash;
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetMaxLodLevels(unsigned levels)
{
    levels = Clamp(levels, MIN_LOD_LEVELS, MAX_LOD_LEVELS);
    if (levels != maxLodLevels_)
    {
        maxLodLevels_ = levels;
        lastPatchSize_ = 0; // Force full recreate

        CreateGeometry();
        MarkNetworkUpdate();
    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetMaterial(Material* material)
{
    material_ = material;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetMaterial(material);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetDrawDistance(float distance)
{
    drawDistance_ = distance;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetDrawDistance(distance);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetShadowDistance(float distance)
{
    shadowDistance_ = distance;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetShadowDistance(distance);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetLodBias(float bias)
{
    lodBias_ = bias;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetLodBias(bias);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetViewMask(unsigned mask)
{
    viewMask_ = mask;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetViewMask(mask);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetLightMask(unsigned mask)
{
    lightMask_ = mask;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetLightMask(mask);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetShadowMask(unsigned mask)
{
    shadowMask_ = mask;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetShadowMask(mask);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetZoneMask(unsigned mask)
{
    zoneMask_ = mask;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetZoneMask(mask);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetMaxLights(unsigned num)
{
    maxLights_ = num;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetMaxLights(num);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetCastShadows(bool enable)
{
    castShadows_ = enable;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetCastShadows(enable);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetOccluder(bool enable)
{
    occluder_ = enable;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetOccluder(enable);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetOccludee(bool enable)
{
    occludee_ = enable;
    for (unsigned i = 0; i < chunks_.Size(); ++i)
    {
        if (chunks_[i])
            chunks_[i]->SetOccludee(enable);
    }

    MarkNetworkUpdate();
}

template<typename _Segment>
void SegmentComponent<_Segment>::ApplyHeightMap()
{
    if (heightMap_)
        CreateGeometry();
}

template<typename _Segment>
Image* SegmentComponent<_Segment>::GetHeightMap() const
{
    return heightMap_;
}

template<typename _Segment>
Material* SegmentComponent<_Segment>::GetMaterial() const
{
    return material_;
}

template<typename _Segment>
float SegmentComponent<_Segment>::GetHeight(const Vector3& worldPosition) const
{
    if (node_)
    {
        Vector3 position = node_->GetWorldTransform().Inverse() * worldPosition;
        float xPos = (position.x_ - patchWorldOrigin_.x_) / spacing_.x_;
        float zPos = (position.z_ - patchWorldOrigin_.y_) / spacing_.z_;
        float xFrac = Fract(xPos);
        float zFrac = Fract(zPos);
        float h1, h2, h3;

        if (xFrac + zFrac >= 1.0f)
        {
            h1 = GetRawHeight((unsigned)xPos + 1, (unsigned)zPos + 1);
            h2 = GetRawHeight((unsigned)xPos, (unsigned)zPos + 1);
            h3 = GetRawHeight((unsigned)xPos + 1, (unsigned)zPos);
            xFrac = 1.0f - xFrac;
            zFrac = 1.0f - zFrac;
        }
        else
        {
            h1 = GetRawHeight((unsigned)xPos, (unsigned)zPos);
            h2 = GetRawHeight((unsigned)xPos + 1, (unsigned)zPos);
            h3 = GetRawHeight((unsigned)xPos, (unsigned)zPos + 1);
        }

        float h = h1 * (1.0f - xFrac - zFrac) + h2 * xFrac + h3 * zFrac;
        /// \todo This assumes that the terrain scene node is upright
        return node_->GetWorldScale().y_ * h + node_->GetWorldPosition().y_;
    }
    else
        return 0.0f;
}

template<typename _Segment>
Vector3 SegmentComponent<_Segment>::GetNormal(const Vector3& worldPosition) const
{
    if (node_)
    {
        Vector3 position = node_->GetWorldTransform().Inverse() * worldPosition;
        float xPos = (position.x_ - patchWorldOrigin_.x_) / spacing_.x_;
        float zPos = (position.z_ - patchWorldOrigin_.y_) / spacing_.z_;
        float xFrac = Fract(xPos);
        float zFrac = Fract(zPos);
        Vector3 n1, n2, n3;

        if (xFrac + zFrac >= 1.0f)
        {
            n1 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos + 1);
            n2 = GetRawNormal((unsigned)xPos, (unsigned)zPos + 1);
            n3 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos);
            xFrac = 1.0f - xFrac;
            zFrac = 1.0f - zFrac;
        }
        else
        {
            n1 = GetRawNormal((unsigned)xPos, (unsigned)zPos);
            n2 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos);
            n3 = GetRawNormal((unsigned)xPos, (unsigned)zPos + 1);
        }

        Vector3 n = (n1 * (1.0f - xFrac - zFrac) + n2 * xFrac + n3 * zFrac).Normalized();
        return node_->GetWorldRotation() * n;
    }
    else
        return Vector3::UP;
}

template<typename _Segment>
IntVector2 SegmentComponent<_Segment>::WorldToHeightMap(const Vector3& worldPosition) const
{
    if (!node_)
        return IntVector2::ZERO;

    Vector3 position = node_->GetWorldTransform().Inverse() * worldPosition;
    int xPos = (int)((position.x_ - patchWorldOrigin_.x_) / spacing_.x_ + 0.5f);
    int zPos = (int)((position.z_ - patchWorldOrigin_.y_) / spacing_.z_ + 0.5f);
    xPos = Clamp(xPos, 0, numVertices_.x_ - 1);
    zPos = Clamp(zPos, 0, numVertices_.y_ - 1);

    return IntVector2(xPos, numVertices_.y_ - 1 - zPos);
}


template<typename _Segment>
void SegmentComponent<_Segment>::SetMaterialAttr(const ResourceRef& value)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetMaterial(cache->GetResource<Material>(value.name_));
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetHeightMapAttr(const ResourceRef& value)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Image* image = cache->GetResource<Image>(value.name_);
    SetHeightMapInternal(image, false);
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetMaxLodLevelsAttr(unsigned value)
{
    value = Clamp(value, MIN_LOD_LEVELS, MAX_LOD_LEVELS);

    if (value != maxLodLevels_)
    {
        maxLodLevels_ = value;
        lastPatchSize_ = 0; // Force full recreate
        recreateTerrain_ = true;
    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetOcclusionLodLevelAttr(unsigned value)
{
    if (value != occlusionLodLevel_)
    {
        occlusionLodLevel_ = value;
        lastPatchSize_ = 0; // Force full recreate
        recreateTerrain_ = true;
    }
}

template<typename _Segment>
ResourceRef SegmentComponent<_Segment>::GetMaterialAttr() const
{
    return GetResourceRef(material_, Material::GetTypeStatic());
}

template<typename _Segment>
ResourceRef SegmentComponent<_Segment>::GetHeightMapAttr() const
{
    return GetResourceRef(heightMap_, Image::GetTypeStatic());
}

template<typename _Segment>
void SegmentComponent<_Segment>::CreateGeometry()
{
    recreateTerrain_ = false;

    if (!node_)
        return;

//    URHO3D_PROFILE(CreateTerrainGeometry);
//
//    unsigned prevNumPatches = chunks_.Size();
//
//    // Determine number of LOD levels
//    unsigned lodSize = (unsigned)patchSize_;
//    numLodLevels_ = 1;
//    while (lodSize > MIN_PATCH_SIZE && numLodLevels_ < maxLodLevels_)
//    {
//        lodSize >>= 1;
//        ++numLodLevels_;
//    }
//
//    // Determine total terrain size
//    patchWorldSize_ = Vector2(spacing_.x_ * (float)patchSize_, spacing_.z_ * (float)patchSize_);
//    bool updateAll = false;
//
//    if (heightMap_)
//    {
//        numPatches_ = IntVector2((heightMap_->GetWidth() - 1) / patchSize_, (heightMap_->GetHeight() - 1) / patchSize_);
//        numVertices_ = IntVector2(numPatches_.x_ * patchSize_ + 1, numPatches_.y_ * patchSize_ + 1);
//        patchWorldOrigin_ =
//            Vector2(-0.5f * (float)numPatches_.x_ * patchWorldSize_.x_, -0.5f * (float)numPatches_.y_ * patchWorldSize_.y_);
//        if (numVertices_ != lastNumVertices_ || lastSpacing_ != spacing_ || patchSize_ != lastPatchSize_)
//            updateAll = true;
//        unsigned newDataSize = (unsigned)(numVertices_.x_ * numVertices_.y_);
//
//        // Create new height data if terrain size changed
//        if (!heightData_ || updateAll)
//            heightData_ = new float[newDataSize];
//
//        // Ensure that the source (unsmoothed) data exists if smoothing is active
//        if (smoothing_ && (!sourceHeightData_ || updateAll))
//        {
//            sourceHeightData_ = new float[newDataSize];
//            updateAll = true;
//        }
//        else if (!smoothing_)
//            sourceHeightData_.Reset();
//    }
//    else
//    {
//        numPatches_ = IntVector2::ZERO;
//        numVertices_ = IntVector2::ZERO;
//        patchWorldOrigin_ = Vector2::ZERO;
//        heightData_.Reset();
//        sourceHeightData_.Reset();
//    }
//
//    lastNumVertices_ = numVertices_;
//    lastPatchSize_ = patchSize_;
//    lastSpacing_ = spacing_;
//
//    // Remove old patch nodes which are not needed
//    if (updateAll)
//    {
//        URHO3D_PROFILE(RemoveOldPatches);
//
//        PODVector<Node*> oldPatchNodes;
//        node_->GetChildrenWithComponent<ChunkDrawable<SegmentType, ChunkType>>(oldPatchNodes);
//        for (PODVector<Node*>::Iterator i = oldPatchNodes.Begin(); i != oldPatchNodes.End(); ++i)
//        {
//            bool nodeOk = false;
//            Vector<String> coords = (*i)->GetName().Substring(6).Split('_');
//            if (coords.Size() == 2)
//            {
//                int x = ToInt(coords[0]);
//                int z = ToInt(coords[1]);
//                if (x < numPatches_.x_ && z < numPatches_.y_)
//                    nodeOk = true;
//            }
//
//            if (!nodeOk)
//                node_->RemoveChild(*i);
//        }
//    }
//
//    // Keep track of which patches actually need an update
//    PODVector<bool> dirtyPatches((unsigned)(numPatches_.x_ * numPatches_.y_));
//    for (unsigned i = 0; i < dirtyPatches.Size(); ++i)
//        dirtyPatches[i] = updateAll;
//
//    chunks_.Clear();
//
//    if (heightMap_)
//    {
//        // Copy heightmap data
//        const unsigned char* src = heightMap_->GetData();
//        float* dest = smoothing_ ? sourceHeightData_ : heightData_;
//        unsigned imgComps = heightMap_->GetComponents();
//        unsigned imgRow = heightMap_->GetWidth() * imgComps;
//        IntRect updateRegion(-1, -1, -1, -1);
//
//        if (imgComps == 1)
//        {
//            URHO3D_PROFILE(CopyHeightData);
//
//            for (int z = 0; z < numVertices_.y_; ++z)
//            {
//                for (int x = 0; x < numVertices_.x_; ++x)
//                {
//                    float newHeight = (float)src[imgRow * (numVertices_.y_ - 1 - z) + x] * spacing_.y_;
//
//                    if (updateAll)
//                        *dest = newHeight;
//                    else
//                    {
//                        if (*dest != newHeight)
//                        {
//                            *dest = newHeight;
//                            GrowUpdateRegion(updateRegion, x, z);
//                        }
//                    }
//
//                    ++dest;
//                }
//            }
//        }
//        else
//        {
//            URHO3D_PROFILE(CopyHeightData);
//
//            // If more than 1 component, use the green channel for more accuracy
//            for (int z = 0; z < numVertices_.y_; ++z)
//            {
//                for (int x = 0; x < numVertices_.x_; ++x)
//                {
//                    float newHeight = ((float)src[imgRow * (numVertices_.y_ - 1 - z) + imgComps * x] +
//                                       (float)src[imgRow * (numVertices_.y_ - 1 - z) + imgComps * x + 1] / 256.0f) * spacing_.y_;
//
//                    if (updateAll)
//                        *dest = newHeight;
//                    else
//                    {
//                        if (*dest != newHeight)
//                        {
//                            *dest = newHeight;
//                            GrowUpdateRegion(updateRegion, x, z);
//                        }
//                    }
//
//                    ++dest;
//                }
//            }
//        }
//
//        // If updating a region of the heightmap, check which patches change
//        if (!updateAll)
//        {
//            int lodExpand = 1 << (numLodLevels_ - 1);
//            // Expand the right & bottom 1 pixel more, as patches share vertices at the edge
//            updateRegion.left_ -= lodExpand;
//            updateRegion.right_ += lodExpand + 1;
//            updateRegion.top_ -= lodExpand;
//            updateRegion.bottom_ += lodExpand + 1;
//
//            int sX = Max(updateRegion.left_ / patchSize_, 0);
//            int eX = Min(updateRegion.right_ / patchSize_, numPatches_.x_ - 1);
//            int sY = Max(updateRegion.top_ / patchSize_, 0);
//            int eY = Min(updateRegion.bottom_ / patchSize_, numPatches_.y_ - 1);
//            for (int y = sY; y <= eY; ++y)
//            {
//                for (int x = sX; x <= eX; ++x)
//                    dirtyPatches[y * numPatches_.x_ + x] = true;
//            }
//        }
//
//        chunks_.Reserve((unsigned)(numPatches_.x_ * numPatches_.y_));
//
//        bool enabled = IsEnabledEffective();
//
//        {
//            URHO3D_PROFILE(CreatePatches);
//
//            // Create patches and set node transforms
//            for (int z = 0; z < numPatches_.y_; ++z)
//            {
//                for (int x = 0; x < numPatches_.x_; ++x)
//                {
//                    String nodeName = "Patch_" + String(x) + "_" + String(z);
//                    Node* patchNode = node_->GetChild(nodeName);
//
//                    if (!patchNode)
//                    {
//                        // Create the patch scene node as local and temporary so that it is not unnecessarily serialized to either
//                        // file or replicated over the network
//                        patchNode = node_->CreateTemporaryChild(nodeName, LOCAL);
//                    }
//
//                    patchNode->SetPosition(Vector3(patchWorldOrigin_.x_ + (float)x * patchWorldSize_.x_, 0.0f,
//                        patchWorldOrigin_.y_ + (float)z * patchWorldSize_.y_));
//
//                    ChunkDrawable<SegmentType, ChunkType>* patch = patchNode->GetComponent<ChunkDrawable<SegmentType, ChunkType>>();
//                    if (!patch)
//                    {
//                        patch = patchNode->CreateComponent<ChunkDrawable<SegmentType, ChunkType>>();
//                        patch->SetOwner(this);
//                        patch->SetCoordinates(IntVector2(x, z));
//
//                        // Copy initial drawable parameters
//                        patch->SetEnabled(enabled);
//                        patch->SetMaterial(material_);
//                        patch->SetDrawDistance(drawDistance_);
//                        patch->SetShadowDistance(shadowDistance_);
//                        patch->SetLodBias(lodBias_);
//                        patch->SetViewMask(viewMask_);
//                        patch->SetLightMask(lightMask_);
//                        patch->SetShadowMask(shadowMask_);
//                        patch->SetZoneMask(zoneMask_);
//                        patch->SetMaxLights(maxLights_);
//                        patch->SetCastShadows(castShadows_);
//                        patch->SetOccluder(occluder_);
//                        patch->SetOccludee(occludee_);
//                    }
//
//                    chunks_.Push(WeakPtr<ChunkDrawable<SegmentType, ChunkType>>(patch));
//                }
//            }
//        }
//
//        // Create the shared index data
//        if (updateAll)
//            CreateIndexData();
//
//        // Create vertex data for patches. First update smoothing to ensure normals are calculated correctly across patch borders
//        if (smoothing_)
//        {
//            URHO3D_PROFILE(UpdateSmoothing);
//
//            for (unsigned i = 0; i < chunks_.Size(); ++i)
//            {
//                if (dirtyPatches[i])
//                {
//                    ChunkDrawable<SegmentType, ChunkType>* patch = chunks_[i];
//                    const IntVector2& coords = patch->GetCoordinates();
//                    int startX = coords.x_ * patchSize_;
//                    int endX = startX + patchSize_;
//                    int startZ = coords.y_ * patchSize_;
//                    int endZ = startZ + patchSize_;
//
//                    for (int z = startZ; z <= endZ; ++z)
//                    {
//                        for (int x = startX; x <= endX; ++x)
//                        {
//                            float smoothedHeight = (
//                                GetSourceHeight(x - 1, z - 1) + GetSourceHeight(x, z - 1) * 2.0f + GetSourceHeight(x + 1, z - 1) +
//                                GetSourceHeight(x - 1, z) * 2.0f + GetSourceHeight(x, z) * 4.0f + GetSourceHeight(x + 1, z) * 2.0f +
//                                GetSourceHeight(x - 1, z + 1) + GetSourceHeight(x, z + 1) * 2.0f + GetSourceHeight(x + 1, z + 1)
//                            ) / 16.0f;
//
//                            heightData_[z * numVertices_.x_ + x] = smoothedHeight;
//                        }
//                    }
//                }
//            }
//        }
//
//        for (unsigned i = 0; i < chunks_.Size(); ++i)
//        {
//            ChunkDrawable<SegmentType, ChunkType>* patch = chunks_[i];
//
//            if (dirtyPatches[i])
//            {
//                CreatePatchGeometry(patch);
//                CalculateLodErrors(patch);
//            }
//
//            SetPatchNeighbors(patch);
//        }
//    }
//
//    // Send event only if new geometry was generated, or the old was cleared
//    if (chunks_.Size() || prevNumPatches)
//    {
//        using namespace TerrainCreated;
//
//        VariantMap& eventData = GetEventDataMap();
//        eventData[P_NODE] = node_;
//        node_->SendEvent(E_TERRAINCREATED, eventData);
//    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::CreateIndexData()
{
    URHO3D_PROFILE(CreateIndexData);

    PODVector<unsigned short> indices;
    drawRanges_.Clear();
    unsigned row = (unsigned)(patchSize_ + 1);

    /* Build index data for each LOD level. Each LOD level except the lowest can stitch to the next lower LOD from the edges:
       north, south, west, east, or any combination of them, requiring 16 different versions of each LOD level's index data

       Normal edge:     Stitched edge:
       +----+----+      +---------+
       |\   |\   |      |\       /|
       | \  | \  |      | \     / |
       |  \ |  \ |      |  \   /  |
       |   \|   \|      |   \ /   |
       +----+----+      +----+----+
    */
    for (unsigned i = 0; i < numLodLevels_; ++i)
    {
        unsigned combinations = (i < numLodLevels_ - 1) ? 16 : 1;
        int skip = 1 << i;

        for (unsigned j = 0; j < combinations; ++j)
        {
            unsigned indexStart = indices.Size();

            int zStart = 0;
            int xStart = 0;
            int zEnd = patchSize_;
            int xEnd = patchSize_;

            if (j & STITCH_NORTH)
                zEnd -= skip;
            if (j & STITCH_SOUTH)
                zStart += skip;
            if (j & STITCH_WEST)
                xStart += skip;
            if (j & STITCH_EAST)
                xEnd -= skip;

            // Build the main grid
            for (int z = zStart; z < zEnd; z += skip)
            {
                for (int x = xStart; x < xEnd; x += skip)
                {
                    indices.Push((unsigned short)((z + skip) * row + x));
                    indices.Push((unsigned short)(z * row + x + skip));
                    indices.Push((unsigned short)(z * row + x));
                    indices.Push((unsigned short)((z + skip) * row + x));
                    indices.Push((unsigned short)((z + skip) * row + x + skip));
                    indices.Push((unsigned short)(z * row + x + skip));
                }
            }

            // Build the north edge
            if (j & STITCH_NORTH)
            {
                int z = patchSize_ - skip;
                for (int x = 0; x < patchSize_; x += skip * 2)
                {
                    if (x > 0 || (j & STITCH_WEST) == 0)
                    {
                        indices.Push((unsigned short)((z + skip) * row + x));
                        indices.Push((unsigned short)(z * row + x + skip));
                        indices.Push((unsigned short)(z * row + x));
                    }
                    indices.Push((unsigned short)((z + skip) * row + x));
                    indices.Push((unsigned short)((z + skip) * row + x + 2 * skip));
                    indices.Push((unsigned short)(z * row + x + skip));
                    if (x < patchSize_ - skip * 2 || (j & STITCH_EAST) == 0)
                    {
                        indices.Push((unsigned short)((z + skip) * row + x + 2 * skip));
                        indices.Push((unsigned short)(z * row + x + 2 * skip));
                        indices.Push((unsigned short)(z * row + x + skip));
                    }
                }
            }

            // Build the south edge
            if (j & STITCH_SOUTH)
            {
                int z = 0;
                for (int x = 0; x < patchSize_; x += skip * 2)
                {
                    if (x > 0 || (j & STITCH_WEST) == 0)
                    {
                        indices.Push((unsigned short)((z + skip) * row + x));
                        indices.Push((unsigned short)((z + skip) * row + x + skip));
                        indices.Push((unsigned short)(z * row + x));
                    }
                    indices.Push((unsigned short)(z * row + x));
                    indices.Push((unsigned short)((z + skip) * row + x + skip));
                    indices.Push((unsigned short)(z * row + x + 2 * skip));
                    if (x < patchSize_ - skip * 2 || (j & STITCH_EAST) == 0)
                    {
                        indices.Push((unsigned short)((z + skip) * row + x + skip));
                        indices.Push((unsigned short)((z + skip) * row + x + 2 * skip));
                        indices.Push((unsigned short)(z * row + x + 2 * skip));
                    }
                }
            }

            // Build the west edge
            if (j & STITCH_WEST)
            {
                int x = 0;
                for (int z = 0; z < patchSize_; z += skip * 2)
                {
                    if (z > 0 || (j & STITCH_SOUTH) == 0)
                    {
                        indices.Push((unsigned short)(z * row + x));
                        indices.Push((unsigned short)((z + skip) * row + x + skip));
                        indices.Push((unsigned short)(z * row + x + skip));
                    }
                    indices.Push((unsigned short)((z + 2 * skip) * row + x));
                    indices.Push((unsigned short)((z + skip) * row + x + skip));
                    indices.Push((unsigned short)(z * row + x));
                    if (z < patchSize_ - skip * 2 || (j & STITCH_NORTH) == 0)
                    {
                        indices.Push((unsigned short)((z + 2 * skip) * row + x));
                        indices.Push((unsigned short)((z + 2 * skip) * row + x + skip));
                        indices.Push((unsigned short)((z + skip) * row + x + skip));
                    }
                }
            }

            // Build the east edge
            if (j & STITCH_EAST)
            {
                int x = patchSize_ - skip;
                for (int z = 0; z < patchSize_; z += skip * 2)
                {
                    if (z > 0 || (j & STITCH_SOUTH) == 0)
                    {
                        indices.Push((unsigned short)(z * row + x));
                        indices.Push((unsigned short)((z + skip) * row + x));
                        indices.Push((unsigned short)(z * row + x + skip));
                    }
                    indices.Push((unsigned short)((z + skip) * row + x));
                    indices.Push((unsigned short)((z + 2 * skip) * row + x + skip));
                    indices.Push((unsigned short)(z * row + x + skip));
                    if (z < patchSize_ - skip * 2 || (j & STITCH_NORTH) == 0)
                    {
                        indices.Push((unsigned short)((z + skip) * row + x));
                        indices.Push((unsigned short)((z + 2 * skip) * row + x));
                        indices.Push((unsigned short)((z + 2 * skip) * row + x + skip));
                    }
                }
            }

            drawRanges_.Push(MakePair(indexStart, indices.Size() - indexStart));
        }
    }

    indexBuffer_->SetSize(indices.Size(), false);
    indexBuffer_->SetData(&indices[0]);
}

template<typename _Segment>
float SegmentComponent<_Segment>::GetRawHeight(int x, int z) const
{
    if (!heightData_)
        return 0.0f;

    x = Clamp(x, 0, numVertices_.x_ - 1);
    z = Clamp(z, 0, numVertices_.y_ - 1);
    return heightData_[z * numVertices_.x_ + x];
}

template<typename _Segment>
float SegmentComponent<_Segment>::GetSourceHeight(int x, int z) const
{
    if (!sourceHeightData_)
        return 0.0f;

    x = Clamp(x, 0, numVertices_.x_ - 1);
    z = Clamp(z, 0, numVertices_.y_ - 1);
    return sourceHeightData_[z * numVertices_.x_ + x];
}

template<typename _Segment>
float SegmentComponent<_Segment>::GetLodHeight(int x, int z, unsigned lodLevel) const
{
    unsigned offset = (unsigned)(1 << lodLevel);
    float divisor = (float)offset;
    float xFrac = (float)(x % offset) / divisor;
    float zFrac = (float)(z % offset) / divisor;
    float h1, h2, h3;

    if (xFrac + zFrac >= 1.0f)
    {
        h1 = GetRawHeight(x + offset, z + offset);
        h2 = GetRawHeight(x, z + offset);
        h3 = GetRawHeight(x + offset, z);
        xFrac = 1.0f - xFrac;
        zFrac = 1.0f - zFrac;
    }
    else
    {
        h1 = GetRawHeight(x, z);
        h2 = GetRawHeight(x + offset, z);
        h3 = GetRawHeight(x, z + offset);
    }

    return h1 * (1.0f - xFrac - zFrac) + h2 * xFrac + h3 * zFrac;
}

template<typename _Segment>
Vector3 SegmentComponent<_Segment>::GetRawNormal(int x, int z) const
{
    float baseHeight = GetRawHeight(x, z);
    float nSlope = GetRawHeight(x, z - 1) - baseHeight;
    float neSlope = GetRawHeight(x + 1, z - 1) - baseHeight;
    float eSlope = GetRawHeight(x + 1, z) - baseHeight;
    float seSlope = GetRawHeight(x + 1, z + 1) - baseHeight;
    float sSlope = GetRawHeight(x, z + 1) - baseHeight;
    float swSlope = GetRawHeight(x - 1, z + 1) - baseHeight;
    float wSlope = GetRawHeight(x - 1, z) - baseHeight;
    float nwSlope = GetRawHeight(x - 1, z - 1) - baseHeight;
    float up = 0.5f * (spacing_.x_ + spacing_.z_);

    return (Vector3(0.0f, up, nSlope) +
            Vector3(-neSlope, up, neSlope) +
            Vector3(-eSlope, up, 0.0f) +
            Vector3(-seSlope, up, -seSlope) +
            Vector3(0.0f, up, -sSlope) +
            Vector3(swSlope, up, -swSlope) +
            Vector3(wSlope, up, 0.0f) +
            Vector3(nwSlope, up, nwSlope)).Normalized();
}

template<typename _Segment>
void SegmentComponent<_Segment>::CalculateLodErrors(ChunkDrawable<SegmentType, ChunkType>* patch)
{
    URHO3D_PROFILE(CalculateLodErrors);

    const IntVector2& coords = patch->GetCoordinates();
    PODVector<float>& lodErrors = patch->GetLodErrors();
    lodErrors.Clear();
    lodErrors.Reserve(numLodLevels_);

    int xStart = coords.x_ * patchSize_;
    int zStart = coords.y_ * patchSize_;
    int xEnd = xStart + patchSize_;
    int zEnd = zStart + patchSize_;

    for (unsigned i = 0; i < numLodLevels_; ++i)
    {
        float maxError = 0.0f;
        int divisor = 1 << i;

        if (i > 0)
        {
            for (int z = zStart; z <= zEnd; ++z)
            {
                for (int x = xStart; x <= xEnd; ++x)
                {
                    if (x % divisor || z % divisor)
                    {
                        float error = Abs(GetLodHeight(x, z, i) - GetRawHeight(x, z));
                        maxError = Max(error, maxError);
                    }
                }
            }

            // Set error to be at least same as (half vertex spacing x LOD) to prevent horizontal stretches getting too inaccurate
            maxError = Max(maxError, 0.25f * (spacing_.x_ + spacing_.z_) * (float)(1 << i));
        }

        lodErrors.Push(maxError);
    }
}

template<typename _Segment>
bool SegmentComponent<_Segment>::SetHeightMapInternal(Image* image, bool recreateNow)
{
    if (image && image->IsCompressed())
    {
        URHO3D_LOGERROR("Can not use a compressed image as a terrain heightmap");
        return false;
    }

    // Unsubscribe from the reload event of previous image (if any), then subscribe to the new
    if (heightMap_)
        UnsubscribeFromEvent(heightMap_, E_RELOADFINISHED);
    if (image)
        SubscribeToEvent(image, E_RELOADFINISHED, URHO3D_HANDLER(SegmentComponent, HandleHeightMapReloadFinished));

    heightMap_ = image;

    if (recreateNow)
        CreateGeometry();
    else
        recreateTerrain_ = true;

    return true;
}

template<typename _Segment>
void SegmentComponent<_Segment>::HandleHeightMapReloadFinished(StringHash eventType, VariantMap& eventData)
{
    CreateGeometry();
}

template<typename _Segment>
void SegmentComponent<_Segment>::HandleNeighborTerrainCreated(StringHash eventType, VariantMap& eventData)
{
    UpdateEdgePatchNeighbors();
}

}
