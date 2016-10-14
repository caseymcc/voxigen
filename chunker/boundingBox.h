#ifndef _voxigen_chunk_h_
#define _voxigen_chunk_h_

namespace voxigen
{

class BoundingBox()
{
public:
    BoundingBox(glm::vec3 &dimensions, glm::mat4 &transform):m_dimensions(dimensions), m_transform(transform){};

    const glm::mat4 &transform() { return m_transform; }

    const glm::vec3 &dimensions() { return m_dimensions; }
    
    glm::mat3 rotation() { return glm::mat3(m_transform[0][0], m_transform[1][0], m_transform[2][0],
                                            m_transform[0][1], m_transform[1][1], m_transform[2][1],
                                            m_transform[0][2], m_transform[1][2], m_transform[2][2]) };

    glm::vec3 position() { return glm::vec3(m_transform[0][3], m_transform[1][3], m_transform[2][3]); }

private:
    glm::mat4 m_transform;
    glm vec3 m_dimensions;
};

} //namespace voxigen

#endif //_voxigen_chunk_h_