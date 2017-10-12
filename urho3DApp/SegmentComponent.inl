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
static const unsigned MIN_LOD_LEVELS=1;
static const unsigned MAX_LOD_LEVELS=4;
static const int DEFAULT_PATCH_SIZE=32;
static const int MIN_PATCH_SIZE=4;
static const int MAX_PATCH_SIZE=128;
static const unsigned STITCH_NORTH=1;
static const unsigned STITCH_SOUTH=2;
static const unsigned STITCH_WEST=4;
static const unsigned STITCH_EAST=8;

inline void GrowUpdateRegion(IntRect& updateRegion, int x, int y)
{
    if(updateRegion.left_<0)
    {
        updateRegion.left_=updateRegion.right_=x;
        updateRegion.top_=updateRegion.bottom_=y;
    }
    else
    {
        if(x<updateRegion.left_)
            updateRegion.left_=x;
        if(x>updateRegion.right_)
            updateRegion.right_=x;
        if(y<updateRegion.top_)
            updateRegion.top_=y;
        if(y>updateRegion.bottom_)
            updateRegion.bottom_=y;
    }
}

template<typename _Segment>
SegmentComponent<_Segment>::SegmentComponent(Context* context):
    Component(context),
    indexBuffer_(new IndexBuffer(context))//,
//    spacing_(DEFAULT_SPACING),
//    lastSpacing_(Vector3::ZERO),
//    //patchWorldOrigin_(Vector2::ZERO),
//    //patchWorldSize_(Vector2::ZERO),
//    numVertices_(IntVector2::ZERO),
//    lastNumVertices_(IntVector2::ZERO),
//    //numPatches_(IntVector2::ZERO),
//    //patchSize_(DEFAULT_PATCH_SIZE),
//    //lastPatchSize_(0),
//    numLodLevels_(1),
//    maxLodLevels_(MAX_LOD_LEVELS),
//    occlusionLodLevel_(M_MAX_UNSIGNED),
//    smoothing_(false),
//    visible_(true),
//    castShadows_(false),
//    occluder_(false),
//    occludee_(true),
//    viewMask_(DEFAULT_VIEWMASK),
//    lightMask_(DEFAULT_LIGHTMASK),
//    shadowMask_(DEFAULT_SHADOWMASK),
//    zoneMask_(DEFAULT_ZONEMASK),
//    drawDistance_(0.0f),
//    shadowDistance_(0.0f),
//    lodBias_(1.0f),
//    maxLights_(0),
//    northID_(0),
//    southID_(0),
//    westID_(0),
//    eastID_(0),
//    recreateTerrain_(false),
//    neighborsDirty_(false)
{
    indexBuffer_->SetShadowed(true);
}

template<typename _Segment>
SegmentComponent<_Segment>::~SegmentComponent()
{}

template<typename _Segment>
void SegmentComponent<_Segment>::RegisterObject(Context* context)
{
    context->RegisterFactory<SegmentComponent>(GEOMETRY_CATEGORY);

    URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
}

template<typename _Segment>
void SegmentComponent<_Segment>::OnSetAttribute(const AttributeInfo& attr, const Variant& src)
{
    Serializable::OnSetAttribute(attr, src);

    // Change of any non-accessor attribute requires recreation of the terrain, or setting the neighbor terrains
    if(!attr.accessor_)
    {
        recreateTerrain_=true;
    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::ApplyAttributes()
{
    if(recreateTerrain_)
        CreateGeometry();
}

template<typename _Segment>
void SegmentComponent<_Segment>::OnSetEnabled()
{
    bool enabled=IsEnabledEffective();

    for(unsigned i=0; i<chunkDrawables_.size(); ++i)
    {
        if(chunkDrawables_[i])
            chunkDrawables_[i]->SetEnabled(enabled);
    }
}

template<typename _Segment>
void SegmentComponent<_Segment>::DrawDebugGeometry(DebugRenderer* debug, bool depthTest)
{}

template<typename _Segment>
voxigen::SegmentHash SegmentComponent<_Segment>::GetSegment()
{
    return segmentHash_;
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetSegment(voxigen::SegmentHash hash)
{
    segmentHash_=hash;
}

template<typename _Segment>
void SegmentComponent<_Segment>::SetOffset(Vector3 offset)
{
    offset_=offset;
}

template<typename _Segment>
void SegmentComponent<_Segment>::CreateGeometry()
{
    recreateTerrain_=false;

    if(!node_)
        return;

}

template<typename _Segment>
template<typename _Grid>
void SegmentComponent<_Segment>::UpdatedChunks(_Grid *grid, voxigen::ChunkHashSet &chunkHashSet)
{
    for(auto drawableIter=chunkDrawables_.begin(); drawableIter!=chunkDrawables_.end(); ++drawableIter)
    {
        ChunkDrawable &chunkDrawable=*(*drawableIter);

        auto iter=chunkHashSet.find(chunkDrawable.GetChunkHash());

        if(iter!=chunkHashSet.end())
            chunkDrawable.Update(grid);
    }
}

template<typename _Segment>
template<typename _Grid>
void SegmentComponent<_Segment>::UpdateChunks(_Grid *grid, voxigen::ChunkHashSet &chunkHashSet)
{
    //find missing chunks, remove not used
    for(auto drawableIter=chunkDrawables_.begin(); drawableIter!=chunkDrawables_.end();)
    {
        ChunkDrawable &chunkDrawable=*(*drawableIter);

        auto iter=chunkHashSet.find(chunkDrawable.GetChunkHash());
        
        if(iter!=chunkHashSet.end())
        {
            chunkHashSet.erase(iter);
            ++drawableIter;
        }
        else
            drawableIter=chunkDrawables_.erase(drawableIter);

    }
    
    //add missing chunks
    for(auto chunkHash:chunkHashSet)
    {
        SharedChunkHandle chunkHandle=grid->getChunk(segmentHash_, chunkHash);

        ChunkDrawable *chunkDrawable=new ChunkDrawable(GetContext());

        chunkDrawable->SetChunk(chunkHandle);
        chunkDrawables_.push_back(chunkDrawable);

        chunkDrawable->Update(grid);
    }
}

}//namespace Urho3D
