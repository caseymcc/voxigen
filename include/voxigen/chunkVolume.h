#ifndef _voxigen_chunkVolume_h_
#define _voxigen_chunkVolume_h_

namespace voxigen
{

template<typename _ChunkVolume>
class ChunkSampler
{
public:
    typedef typename _ChunkVolume::ChunkType ChunkType;
    typedef typename _ChunkVolume::CellType CellType;
    typedef typename _ChunkVolume::VoxelType VoxelType;

    ChunkSampler(_ChunkVolume *volume);
    ~ChunkSampler();

    inline VoxelType getVoxel(void) const;

    void setPosition(int32_t xPos, int32_t yPos, int32_t zPos);
    inline bool setVoxel(VoxelType tValue);

    void movePositiveX(void);
    void movePositiveY(void);
    void movePositiveZ(void);

    void moveNegativeX(void);
    void moveNegativeY(void);
    void moveNegativeZ(void);

    template<int _offsetX, int _offsetY, int _offsetZ>
    VoxelType peek(void) const;

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
    
private:
    _ChunkVolume *m_chunkVolume;
    ChunkType *m_chunk;

    glm::ivec3 m_position;
    glm::ivec3 m_minPosition;
    glm::ivec3 m_maxPosition;
};


template<typename _Chunk>
class ChunkVolume
{
public:
    typedef _Chunk ChunkType;
    typedef typename _Chunk::CellType CellType;
    typedef typename _Chunk::CellType::Type VoxelType;
    typedef ChunkSampler<ChunkVolume> Sampler;

    ChunkVolume(_Chunk *chunk):m_chunk(chunk){}

    _Chunk *getChunk() { return m_chunk; }
private:
    _Chunk *m_chunk;
};

}//namespace voxigen


#include "chunkVolume.inl"

#endif //_voxigen_chunkVolume_h_