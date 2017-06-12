#ifndef _voxigen_chunkHandler_h_
#define _voxigen_chunkHandler_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/boundingBox.h"
#include "voxigen/chunk.h"
#include "voxigen/block.h"

#include <vector>

namespace voxigen
{

class VOXIGEN_EXPORT ChunkHandler:public BoundingBox
{
public:
    ChunkHandler(glm::ivec3 chunkSize);

//    glm::vec3 chunks();


private:
    std::vector<Chunk<Block>> m_chunks;

    glm::ivec3 m_chunkSize;
};

} //namespace voxigen

#endif //_voxigen_chunkHandler_h_