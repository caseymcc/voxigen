namespace voxigen
{

   
template<typename _ChunkVolume>
ChunkSampler<_ChunkVolume>::ChunkSampler(_ChunkVolume *volume):
m_chunkVolume(volume),
m_position(0, 0, 0)
{
    m_chunk=m_chunkVolume->getChunk();

    m_minPosition=glm::ivec3(0, 0, 0);
    m_maxPosition=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value);
}
    
template<typename _ChunkVolume>
ChunkSampler<_ChunkVolume>::~ChunkSampler()
{}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::getVoxel(void) const
{
    assert(glm::all(glm::greaterThanEqual(m_position, m_minPosition)));
    assert(glm::all(glm::lessThan(m_position, m_maxPosition)));

    CellType &cell=m_chunk->getCell(m_position);

    return cell.type;
}

template<typename _ChunkVolume>
void ChunkSampler<_ChunkVolume>::setPosition(int32_t xPos, int32_t yPos, int32_t zPos)
{
    m_position.x=xPos;
    m_position.y=yPos;
    m_position.z=zPos;
}
    
template<typename _ChunkVolume>
bool ChunkSampler<_ChunkVolume>::setVoxel(VoxelType tValue)
{
    assert(glm::all(glm::greaterThanEqual(m_position, m_minPosition)));
    assert(glm::all(glm::lessThan(m_position, m_maxPosition)));

    CellType &cell=m_chunk->getCell(m_position);

    cell.type=tValue;
}

template<typename _ChunkVolume>
void ChunkSampler<_ChunkVolume>::movePositiveX(void)
{
    m_position.x++;
}

template<typename _ChunkVolume>
void ChunkSampler<_ChunkVolume>::movePositiveY(void)
{
    m_position.y++;
}

template<typename _ChunkVolume>
void ChunkSampler<_ChunkVolume>::movePositiveZ(void)
{
    m_position.z++;
}

template<typename _ChunkVolume>
void ChunkSampler<_ChunkVolume>::moveNegativeX(void)
{
    m_position.x--;
}

template<typename _ChunkVolume>
void ChunkSampler<_ChunkVolume>::moveNegativeY(void)
{
    m_position.y--;
}

template<typename _ChunkVolume>
void ChunkSampler<_ChunkVolume>::moveNegativeZ(void)
{
    m_position.z--;
}

template<typename _ChunkVolume>
template<int _offsetX, int _offsetY, int _offsetZ>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peek(void) const
{
    glm::ivec3 position=m_position+glm::ivec3(_offsetX, _offsetY, _offsetZ);

    if(glm::any(glm::lessThan(position, m_minPosition)))
        return 0;
    if(glm::any(glm::greaterThanEqual(position, m_maxPosition)))
        return 0;

    CellType &cell=m_chunk->getCell(position);

    return cell.type;
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx1ny1nz(void) const
{
    return peek<-1, -1, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx1ny0pz(void) const
{
    return peek<-1, -1, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx1ny1pz(void) const
{
    return peek<-1, -1, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx0py1nz(void) const
{
    return peek<-1, 0, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx0py0pz(void) const
{
    return peek<-1, 0, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx0py1pz(void) const
{
    return peek<-1, 0, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx1py1nz(void) const
{
    return peek<-1, 1, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx1py0pz(void) const
{
    return peek<-1, 1, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1nx1py1pz(void) const
{
    return peek<-1, 1, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px1ny1nz(void) const
{
    return peek<0, -1, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px1ny0pz(void) const
{
    return peek<0, -1, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px1ny1pz(void) const
{
    return peek<0, -1, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px0py1nz(void) const
{
    return peek<0, 0, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px0py0pz(void) const
{
    return peek<0, 0, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px0py1pz(void) const
{
    return peek<0, 0, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px1py1nz(void) const
{
    return peek<0, 1, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px1py0pz(void) const
{
    return peek0<0, 1, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel0px1py1pz(void) const
{
    return peek<0, 1, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px1ny1nz(void) const
{
    return peek<1, -1, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px1ny0pz(void) const
{
    return peek<1, -1, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px1ny1pz(void) const
{
    return peek<1, -1, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px0py1nz(void) const
{
    return peek<1, 0, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px0py0pz(void) const
{
    return peek<1, 0, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px0py1pz(void) const
{
    return peek<1, 0, 1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px1py1nz(void) const
{
    return peek<1, 1, -1>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px1py0pz(void) const
{
    return peek<1, 1, 0>();
}

template<typename _ChunkVolume>
typename ChunkSampler<_ChunkVolume>::VoxelType ChunkSampler<_ChunkVolume>::peekVoxel1px1py1pz(void) const
{
    return peek<1, 1, 1>();
}

}//namespace voxigen
