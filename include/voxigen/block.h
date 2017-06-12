#ifndef _voxigen_block_h_
#define _voxigen_block_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

struct VOXIGEN_EXPORT BlockDescriptor
{

};

class VOXIGEN_EXPORT Block
{
public:
    Block(unsigned int type);

private:
    unsigned int m_type;

    unsigned int m_damage;
    unsigned int m_age;
};

}//namesapce voxigen

#endif //_voxigen_block_h_
