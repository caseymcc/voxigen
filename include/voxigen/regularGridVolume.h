#ifndef _voxigen_regularGridVolume_h_
#define _voxigen_regularGridVolume_h_

namespace voxigen
{

template<typename _RegularGridVolume>
class RegularGridSampler
{
public:
    typedef _RegularGridVolume::VoxelType VoxelType;

    RegularGridSampler(_RegularGridVolume *volume);
    ~RegularGridSampler();

    inline VoxelType getVoxel(void) const;

    void setPosition(const Vector3DInt32& v3dNewPos);
    void setPosition(int32_t xPos, int32_t yPos, int32_t zPos);
    inline bool setVoxel(VoxelType tValue);

    void movePositiveX(void);
    void movePositiveY(void);
    void movePositiveZ(void);

    void moveNegativeX(void);
    void moveNegativeY(void);
    void moveNegativeZ(void);

    inline VoxelType peekVoxel1nx1ny1nz(void) const;
    inline VoxelType peekVoxel1nx1ny0pz(void) const;
    inline VoxelType peekVoxel1nx1ny1pz(void) const;
    inline VoxelType peekVoxel1nx0py1nz(void) const;
    inline VoxelType peekVoxel1nx0py0pz(void) const;
    inline VoxelType peekVoxel1nx0py1pz(void) const;
    inline VoxelType peekVoxel1nx1py1nz(void) const;
    inline VoxelType peekVoxel1nx1py0pz(void) const;
    inline VoxelType peekVoxel1nx1py1pz(void) const;

    inline VoxelType peekVoxel0px1ny1nz(void) const;
    inline VoxelType peekVoxel0px1ny0pz(void) const;
    inline VoxelType peekVoxel0px1ny1pz(void) const;
    inline VoxelType peekVoxel0px0py1nz(void) const;
    inline VoxelType peekVoxel0px0py0pz(void) const;
    inline VoxelType peekVoxel0px0py1pz(void) const;
    inline VoxelType peekVoxel0px1py1nz(void) const;
    inline VoxelType peekVoxel0px1py0pz(void) const;
    inline VoxelType peekVoxel0px1py1pz(void) const;

    inline VoxelType peekVoxel1px1ny1nz(void) const;
    inline VoxelType peekVoxel1px1ny0pz(void) const;
    inline VoxelType peekVoxel1px1ny1pz(void) const;
    inline VoxelType peekVoxel1px0py1nz(void) const;
    inline VoxelType peekVoxel1px0py0pz(void) const;
    inline VoxelType peekVoxel1px0py1pz(void) const;
    inline VoxelType peekVoxel1px1py1nz(void) const;
    inline VoxelType peekVoxel1px1py0pz(void) const;
    inline VoxelType peekVoxel1px1py1pz(void) const;
};


template<typename _RegularGrid>
class RegularGridVolume
{
public:
    typedef _RegularGrid::ChunkType::CellType VoxelType;
    typedef RegularGridSampler Sampler;

private:
};

}//namespace voxigen


#include "regularGridVolume.inl"

#endif //_voxigen_regularGridVolume_h_