#ifndef _voxigen_cell_h_
#define _voxigen_cell_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

struct VOXIGEN_EXPORT BlockDescriptor
{

};

struct VOXIGEN_EXPORT Cell
{
    typedef unsigned int Type;

    Type type;

    unsigned int damage;
    unsigned int age;
};

bool empty(Cell &cell) { return (cell.type==0); }
unsigned int type(Cell &cell) { return cell.type; }

}//namesapce voxigen

#endif //_voxigen_cell_h_
