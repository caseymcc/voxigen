#ifndef _voxigen_cell_h_
#define _voxigen_cell_h_
#pragma once

#include "voxigen/voxigen_export.h"

namespace voxigen
{

struct BlockDescriptor
{

};

struct Cell
{
    typedef unsigned int Type;

    Type type;

    unsigned int damage;
    unsigned int age;
};

inline bool empty(Cell &cell) { return (cell.type==0); }
inline unsigned int type(Cell &cell) { return cell.type; }

}//namesapce voxigen

#endif //_voxigen_cell_h_
