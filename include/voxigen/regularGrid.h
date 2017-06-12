#ifndef _voxigen_regularGrid_h_
#define _voxigen_regularGrid_h_

#include "glm/glm.h"

namespace voxigen
{

template<typename _Block>
class RegularGrid
{
public:
    RegularGrid(glm::vec3 m_anchorPoint, glm::vec3 m_orinetation);

    _Block &block(glm::ivec3 pos);
private:
    glm::vec3 m_anchorPoint;
    glm::vec3 m_orinetation;

    std::vector<_Block> m_blocks;
};

}//namespace voxigen

#endif //_voxigen_regularGrid_h_
