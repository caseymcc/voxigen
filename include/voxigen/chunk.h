#ifndef _voxigen_chunk_h_
#define _voxigen_chunk_h_

#include "voxigen/boundingBox.h"

#include <vector>
#include <memory>

namespace voxigen
{

template<typename _Block>
class Chunk:public BoundingBox
{
public:
    Chunk(glm::ivec3 m_position):BoundingBox(dimensions, transform)
    {}

private:
    bool m_loaded;

    std::vector<_Block> m_blocks;
    glm::ivec3 m_position;
};

template<typename _Block>
using UniqueChunk=std::unique_ptr<Chunk<_Block>>;

} //namespace voxigen

#endif //_voxigen_chunk_h_