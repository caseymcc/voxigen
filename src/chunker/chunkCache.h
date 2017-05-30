#ifndef _voxigen_chunk_h_
#define _voxigen_chunk_h_

namespace voxigen
{

class Chunk():public BoundingBox
{
public:
    Chunk(glm::vec3 &dimensions, glm::mat4 &transform):BoundingBox(dimensions, transform);

private:
    bool m_loaded;

};

} //namespace voxigen

#endif //_voxigen_chunk_h_