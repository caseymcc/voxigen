namespace voxigen
{

template<typename _RegularGridVolume>
class RegularGridSampler
    
template<typename _RegularGridVolume>
RegularGridSampler<_RegularGridVolume>::RegularGridSampler(_RegularGridVolume *volume)
{
}
    
template<typename _RegularGridVolume>
RegularGridSampler<_RegularGridVolume>::~RegularGridSampler()
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::getVoxel(void) const
{}

template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::setPosition(const Vector3DInt32& v3dNewPos)
{}
    
template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::setPosition(int32_t xPos, int32_t yPos, int32_t zPos)
{
}
    
template<typename _RegularGridVolume>
bool RegularGridSampler<_RegularGridVolume>::setVoxel(VoxelType tValue)
{}

template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::movePositiveX(void)
{}

template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::movePositiveY(void)
{}

template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::movePositiveZ(void)
{
}

template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::moveNegativeX(void)
{}

template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::moveNegativeY(void)
{}

template<typename _RegularGridVolume>
void RegularGridSampler<_RegularGridVolume>::moveNegativeZ(void)
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx1ny1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx1ny0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx1ny1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx0py1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx0py0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx0py1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx1py1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx1py0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1nx1py1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px1ny1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px1ny0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px1ny1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px0py1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px0py0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px0py1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px1py1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px1py0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel0px1py1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px1ny1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px1ny0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px1ny1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px0py1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px0py0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px0py1pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px1py1nz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px1py0pz(void) const
{}

template<typename _RegularGridVolume>
VoxelType RegularGridSampler<_RegularGridVolume>::peekVoxel1px1py1pz(void) const
{}



template<typename _RegularGrid>
class RegularGridVolume
{
public:
    typedef _RegularGrid::ChunkType::CellType VoxelType;
    typedef RegularGridSampler Sampler;

private:
};

}//namespace voxigen

#endif //_voxigen_regularGridMeshExtractor_h_