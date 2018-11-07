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

inline bool empty(const Cell &cell) { return (cell.type==0); }
inline unsigned int type(Cell const &cell) { return cell.type; }
inline unsigned int &type(Cell &cell) { return cell.type; }

struct CellHeight:Cell
{
    int height;
};

inline int height(CellHeight const &cell) { return cell.height; }
inline int &height(CellHeight &cell) { return cell.height; }

}//namesapce voxigen

#endif //_voxigen_cell_h_
