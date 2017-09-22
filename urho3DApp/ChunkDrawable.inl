//
// Copyright (c) 2008-2017 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Precompiled.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Geometry.h"
#include "Urho3D/Graphics/IndexBuffer.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/OcclusionBuffer.h"
#include "Urho3D/Graphics/OctreeQuery.h"
#include "Urho3D/Graphics/VertexBuffer.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Scene/Node.h"

#include "ChunkDrawable.h"
//#include "SegmentComponent.h"

#include "Urho3D/DebugNew.h"

namespace Urho3D
{

static const float LOD_CONSTANT=1.0f/150.0f;

extern const char* GEOMETRY_CATEGORY;

template<typename _Segment, typename _Chunk>
ChunkDrawable<_Segment, _Chunk>::ChunkDrawable(Context* context):
Drawable(context, DRAWABLE_GEOMETRY),
geometry_(new Geometry(context)),
maxLodGeometry_(new Geometry(context)),
occlusionGeometry_(new Geometry(context)),
vertexBuffer_(new VertexBuffer(context)),
coordinates_(IntVector2::ZERO),
lodLevel_(0),
updateGeometry_(false)
{
    geometry_->SetVertexBuffer(0, vertexBuffer_);
    maxLodGeometry_->SetVertexBuffer(0, vertexBuffer_);
    occlusionGeometry_->SetVertexBuffer(0, vertexBuffer_);

    batches_.Resize(1);
    batches_[0].geometry_=geometry_;
    batches_[0].geometryType_=GEOM_STATIC_NOINSTANCING;
}

template<typename _Segment, typename _Chunk>
ChunkDrawable<_Segment, _Chunk>::~ChunkDrawable()
{}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::RegisterObject(Context* context)
{
    context->RegisterFactory<ChunkDrawable>();
}

template<typename _Segment, typename _Chunk>
voxigen::ChunkHash ChunkDrawable<_Segment, _Chunk>::GetChunkHash()
{
    return chunk_->getHash();
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::SetChunk(std::shared_ptr<_Chunk> chunk)
{
    chunk_=chunk;
    updateGeometry_=true;
}

void ChunkDrawable<_Segment, _Chunk>::OnUpdate()
{
    if(updateGeometry_)
        UpdateGeometry();
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results)
{
    RayQueryLevel level=query.level_;

    switch(level)
    {
    case RAY_AABB:
        Drawable::ProcessRayQuery(query, results);
        break;

    case RAY_OBB:
    case RAY_TRIANGLE:
    {
        Matrix3x4 inverse(node_->GetWorldTransform().Inverse());
        Ray localRay=query.ray_.Transformed(inverse);
        float distance=localRay.HitDistance(boundingBox_);
        Vector3 normal=-query.ray_.direction_;

        if(level==RAY_TRIANGLE && distance < query.maxDistance_)
        {
            Vector3 geometryNormal;
            distance=geometry_->GetHitDistance(localRay, &geometryNormal);
            normal=(node_->GetWorldTransform() * Vector4(geometryNormal, 0.0f)).Normalized();
        }

        if(distance < query.maxDistance_)
        {
            RayQueryResult result;
            result.position_=query.ray_.origin_+distance * query.ray_.direction_;
            result.normal_=normal;
            result.distance_=distance;
            result.drawable_=this;
            result.node_=node_;
            result.subObject_=M_MAX_UNSIGNED;
            results.Push(result);
        }
    }
    break;

    case RAY_TRIANGLE_UV:
        URHO3D_LOGWARNING("RAY_TRIANGLE_UV query level is not supported for ChunkDrawable component");
        break;
    }
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::UpdateBatches(const FrameInfo& frame)
{
    const Matrix3x4& worldTransform=node_->GetWorldTransform();
    distance_=frame.camera_->GetDistance(GetWorldBoundingBox().Center());

    float scale=worldTransform.Scale().DotProduct(DOT_SCALE);
    lodDistance_=frame.camera_->GetLodDistance(distance_, scale, lodBias_);

    batches_[0].distance_=distance_;
    batches_[0].worldTransform_=&worldTransform;

    unsigned newLodLevel=0;
    for(unsigned i=0; i < lodErrors_.Size(); ++i)
    {
        if(lodErrors_[i]/lodDistance_ > LOD_CONSTANT)
            break;
        else
            newLodLevel=i;
    }

    lodLevel_=GetCorrectedLodLevel(newLodLevel);
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::UpdateGeometry(const FrameInfo& frame)
{
    updateGeometry_=false;

    if(vertexBuffer_->IsDataLost())
    {
    }
}

template<typename _Segment, typename _Chunk>
UpdateGeometryType ChunkDrawable<_Segment, _Chunk>::GetUpdateGeometryType()
{
    //GPU updates are handled in a seperate thread so all updates are just swaping of state
    //and existing GPU items so always update in the main thread
    return UPDATE_MAIN_THREAD;
}

template<typename _Segment, typename _Chunk>
Geometry* ChunkDrawable<_Segment, _Chunk>::GetLodGeometry(unsigned batchIndex, unsigned level)
{
    if(!level)
        return maxLodGeometry_;
    else
        return geometry_;
}

template<typename _Segment, typename _Chunk>
unsigned ChunkDrawable<_Segment, _Chunk>::GetNumOccluderTriangles()
{
    // Check that the material is suitable for occlusion (default material always is)
    Material* mat=batches_[0].material_;
    if(mat&&!mat->GetOcclusion())
        return 0;
    else
        return occlusionGeometry_->GetIndexCount()/3;
}

template<typename _Segment, typename _Chunk>
bool ChunkDrawable<_Segment, _Chunk>::DrawOcclusion(OcclusionBuffer* buffer)
{
    // Check that the material is suitable for occlusion (default material always is) and set culling mode
    Material* material=batches_[0].material_;
    if(material)
    {
        if(!material->GetOcclusion())
            return true;
        buffer->SetCullMode(material->GetCullMode());
    }
    else
        buffer->SetCullMode(CULL_CCW);

    const unsigned char* vertexData;
    unsigned vertexSize;
    const unsigned char* indexData;
    unsigned indexSize;
    const PODVector<VertexElement>* elements;

    occlusionGeometry_->GetRawData(vertexData, vertexSize, indexData, indexSize, elements);
    // Check for valid geometry data
    if(!vertexData||!indexData||!elements||VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR3, SEM_POSITION)!=0)
        return false;

    // Draw and check for running out of triangles
    return buffer->AddTriangles(node_->GetWorldTransform(), vertexData, vertexSize, indexData, indexSize, occlusionGeometry_->GetIndexStart(),
        occlusionGeometry_->GetIndexCount());
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::DrawDebugGeometry(DebugRenderer* debug, bool depthTest)
{
    // Intentionally no operation
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::SetOwner(SegmentComponent<_Segment>* terrain)
{
    owner_=terrain;
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::SetMaterial(Material* material)
{
    batches_[0].material_=material;
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::SetBoundingBox(const BoundingBox& box)
{
    boundingBox_=box;
    OnMarkedDirty(node_);
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::SetCoordinates(const IntVector2& coordinates)
{
    coordinates_=coordinates;
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::ResetLod()
{
    lodLevel_=0;
}

template<typename _Segment, typename _Chunk>
Geometry* ChunkDrawable<_Segment, _Chunk>::GetGeometry() const
{
    return geometry_;
}

template<typename _Segment, typename _Chunk>
Geometry* ChunkDrawable<_Segment, _Chunk>::GetMaxLodGeometry() const
{
    return maxLodGeometry_;
}

template<typename _Segment, typename _Chunk>
Geometry* ChunkDrawable<_Segment, _Chunk>::GetOcclusionGeometry() const
{
    return occlusionGeometry_;
}

template<typename _Segment, typename _Chunk>
VertexBuffer* ChunkDrawable<_Segment, _Chunk>::GetVertexBuffer() const
{
    return vertexBuffer_;
}

template<typename _Segment, typename _Chunk>
SegmentComponent<_Segment>* ChunkDrawable<_Segment, _Chunk>::GetOwner() const
{
    return owner_;
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::OnWorldBoundingBoxUpdate()
{
    worldBoundingBox_=boundingBox_.Transformed(node_->GetWorldTransform());
}

template<typename _Segment, typename _Chunk>
unsigned ChunkDrawable<_Segment, _Chunk>::GetCorrectedLodLevel(unsigned lodLevel)
{
    return lodLevel;
}

template<typename _Segment, typename _Chunk>
void ChunkDrawable<_Segment, _Chunk>::CreateGeometry()
{
    URHO3D_PROFILE(CreateGeometry);

    unsigned row=(unsigned)(patchSize_+1);
//    VertexBuffer* vertexBuffer=patch->GetVertexBuffer();
//    Geometry* geometry=patch->GetGeometry();
//    Geometry* maxLodGeometry=patch->GetMaxLodGeometry();
//    Geometry* occlusionGeometry=patch->GetOcclusionGeometry();

    if(vertexBuffer_->GetVertexCount()!=row * row)
        vertexBuffer_->SetSize(row * row, MASK_POSITION|MASK_NORMAL|MASK_TEXCOORD1|MASK_TANGENT);

    SharedArrayPtr<unsigned char> cpuVertexData(new unsigned char[row * row*sizeof(Vector3)]);
    SharedArrayPtr<unsigned char> occlusionCpuVertexData(new unsigned char[row * row*sizeof(Vector3)]);

    float* vertexData=(float*)vertexBuffer_->Lock(0, vertexBuffer_->GetVertexCount());
    float* positionData=(float*)cpuVertexData.Get();
    float* occlusionData=(float*)occlusionCpuVertexData.Get();
    BoundingBox box;

    unsigned occlusionLevel=occlusionLodLevel_;
    if(occlusionLevel > numLodLevels_-1)
        occlusionLevel=numLodLevels_-1;

    if(vertexData)
    {
        const IntVector2& coords=patch->GetCoordinates();
        int lodExpand=(1<<(occlusionLevel))-1;
        int halfLodExpand=(1<<(occlusionLevel))/2;

        for(int z=0; z<=patchSize_; ++z)
        {
            for(int x=0; x<=patchSize_; ++x)
            {
                int xPos=coords.x_ * patchSize_+x;
                int zPos=coords.y_ * patchSize_+z;

                // Position
                Vector3 position((float)x * spacing_.x_, GetRawHeight(xPos, zPos), (float)z * spacing_.z_);
                *vertexData++=position.x_;
                *vertexData++=position.y_;
                *vertexData++=position.z_;
                *positionData++=position.x_;
                *positionData++=position.y_;
                *positionData++=position.z_;

                box.Merge(position);

                // For vertices that are part of the occlusion LOD, calculate the minimum height in the neighborhood
                // to prevent false positive occlusion due to inaccuracy between occlusion LOD & visible LOD
                float minHeight=position.y_;
                if(halfLodExpand > 0&&(x & lodExpand)==0&&(z & lodExpand)==0)
                {
                    int minX=Max(xPos-halfLodExpand, 0);
                    int maxX=Min(xPos+halfLodExpand, numVertices_.x_-1);
                    int minZ=Max(zPos-halfLodExpand, 0);
                    int maxZ=Min(zPos+halfLodExpand, numVertices_.y_-1);
                    for(int nZ=minZ; nZ<=maxZ; ++nZ)
                    {
                        for(int nX=minX; nX<=maxX; ++nX)
                            minHeight=Min(minHeight, GetRawHeight(nX, nZ));
                    }
                }
                *occlusionData++=position.x_;
                *occlusionData++=minHeight;
                *occlusionData++=position.z_;

                // Normal
                Vector3 normal=GetRawNormal(xPos, zPos);
                *vertexData++=normal.x_;
                *vertexData++=normal.y_;
                *vertexData++=normal.z_;

                // Texture coordinate
                Vector2 texCoord((float)xPos/(float)(numVertices_.x_-1), 1.0f-(float)zPos/(float)(numVertices_.y_-1));
                *vertexData++=texCoord.x_;
                *vertexData++=texCoord.y_;

                // Tangent
                Vector3 xyz=(Vector3::RIGHT-normal * normal.DotProduct(Vector3::RIGHT)).Normalized();
                *vertexData++=xyz.x_;
                *vertexData++=xyz.y_;
                *vertexData++=xyz.z_;
                *vertexData++=1.0f;
            }
        }

        vertexBuffer_->Unlock();
        vertexBuffer_->ClearDataLost();
    }

    patch->SetBoundingBox(box);

    if(drawRanges_.Size())
    {
        unsigned occlusionDrawRange=occlusionLevel<<4;

        geometry_->SetIndexBuffer(indexBuffer_);
        geometry_->SetDrawRange(TRIANGLE_LIST, drawRanges_[0].first_, drawRanges_[0].second_, false);
        geometry_->SetRawVertexData(cpuVertexData, MASK_POSITION);
        maxLodGeometry_->SetIndexBuffer(indexBuffer_);
        maxLodGeometry_->SetDrawRange(TRIANGLE_LIST, drawRanges_[0].first_, drawRanges_[0].second_, false);
        maxLodGeometry_->SetRawVertexData(cpuVertexData, MASK_POSITION);
        occlusionGeometry_->SetIndexBuffer(indexBuffer_);
        occlusionGeometry_->SetDrawRange(TRIANGLE_LIST, drawRanges_[occlusionDrawRange].first_, drawRanges_[occlusionDrawRange].second_, false);
        occlusionGeometry_->SetRawVertexData(occlusionCpuVertexData, MASK_POSITION);
    }

    patch->ResetLod();
}

}//namespace Urho3D
