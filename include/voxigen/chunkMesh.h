#ifndef _voxigen_chunkMesh_h_
#define _voxigen_chunkMesh_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"
#include "voxigen/chunk.h"

#include <array>

namespace voxigen
{

struct VOXIGEN_EXPORT ChunkMeshVertex
{
    uint8_t x, y, z;
    uint32_t data;
};

class VOXIGEN_EXPORT ChunkMesh
{
public:
    ChunkMesh();

    void addFace(const std::array<uint8_t, 12>& blockFace, glm::ivec3 position);
    void addFace(const std::array<uint8_t, 12>& blockFace, glm::ivec3 position, unsigned int data);

    std::vector<ChunkMeshVertex> &getVerticies() { return m_verticies; }
    std::vector<int> &getIndices() { return m_indices; }

private:
    std::vector<ChunkMeshVertex> m_verticies;
    std::vector<int> m_indices;
    
};

} //namespace voxigen

#endif //_voxigen_chunkMesh_h_