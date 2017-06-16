#ifndef _voxigen_block_h_
#define _voxigen_block_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

struct VOXIGEN_EXPORT BlockDescriptor
{

};

struct VOXIGEN_EXPORT Block
{
    unsigned int type;

    unsigned int damage;
    unsigned int age;
};

}//namesapce voxigen

#endif //_voxigen_block_h_
