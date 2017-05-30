#ifndef _voxigen_chunkHandler_h_
#define _voxigen_chunkHandler_h_

#include "boundingBox.h"

namespace voxigen
{

class ChunkHandler:public BoundingBox
{
public:
    ChunkHandler(glm::vec3i chunkSize);

    glm::vec3 chunks();


private:
    std::vector<Chunk> m_chunks;

    glm::vec3i m_chunkSize;
};

} //namespace voxigen

#endif //_voxigen_chunkHandler_h_