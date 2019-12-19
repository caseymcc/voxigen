#ifndef _voxigen_simpleShapes_h_
#define _voxigen_simpleShapes_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

template<size_t x, size_t y, size_t z>
struct SimpleCube
{
    static std::vector<float> vertCoords;
};

template<size_t x, size_t y, size_t z>
std::vector<float> SimpleCube<x, y, z>::vertCoords=
{  //vertex                 //normal             //tex coords
//face 1 {x=1}
    1.0f*x, 0.0f*y, 0.0f*z,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, //0.0f, 1.0f, 
    1.0f*x, 1.0f*y, 0.0f*z,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, //1.0f, 1.0f,
    1.0f*x, 1.0f*y, 1.0f*z,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, //1.0f, 0.0f,
    1.0f*x, 0.0f*y, 0.0f*z,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, //0.0f, 1.0f,
    1.0f*x, 1.0f*y, 1.0f*z,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, //1.0f, 0.0f,
    1.0f*x, 0.0f*y, 1.0f*z,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, //0.0f, 1.0f,
//face 2 {y=1}        0.0f, 0.0f, //
    1.0f*x, 1.0f*y, 0.0f*z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, //0.0f, 1.0f,
    0.0f*x, 1.0f*y, 0.0f*z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, //1.0f, 1.0f,
    0.0f*x, 1.0f*y, 1.0f*z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, //1.0f, 0.0f,
    1.0f*x, 1.0f*y, 0.0f*z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, //0.0f, 1.0f,
    0.0f*x, 1.0f*y, 1.0f*z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, //1.0f, 0.0f,
    1.0f*x, 1.0f*y, 1.0f*z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, //0.0f, 1.0f,
//face 3 {x=0}        0.0f, 0.0f, //
    0.0f*x, 1.0f*y, 0.0f*z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, //0.0f, 1.0f,
    0.0f*x, 0.0f*y, 0.0f*z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, //1.0f, 1.0f,
    0.0f*x, 0.0f*y, 1.0f*z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, //1.0f, 0.0f,
    0.0f*x, 1.0f*y, 0.0f*z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, //0.0f, 1.0f,
    0.0f*x, 0.0f*y, 1.0f*z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, //1.0f, 0.0f,
    0.0f*x, 1.0f*y, 1.0f*z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, //0.0f, 1.0f,
//face 4 {y=0}        0.0f, 0.0f, //
    0.0f*x, 0.0f*y, 0.0f*z,  0.0f, -1.0f,  0.0f, 1.0f, 0.5f, //0.0f, 1.0f,
    1.0f*x, 0.0f*y, 0.0f*z,  0.0f, -1.0f,  0.0f, 1.0f, 0.5f, //1.0f, 1.0f,
    1.0f*x, 0.0f*y, 1.0f*z,  0.0f, -1.0f,  0.0f, 1.0f, 0.5f, //1.0f, 0.0f,
    0.0f*x, 0.0f*y, 0.0f*z,  0.0f, -1.0f,  0.0f, 1.0f, 0.5f, //0.0f, 1.0f,
    1.0f*x, 0.0f*y, 1.0f*z,  0.0f, -1.0f,  0.0f, 1.0f, 0.5f, //1.0f, 0.0f,
    0.0f*x, 0.0f*y, 1.0f*z,  0.0f, -1.0f,  0.0f, 1.0f, 0.5f, //0.0f, 1.0f,
//face 5 {z=1}        0.0f, 0.0f, //
    1.0f*x, 0.0f*y, 1.0f*z,  0.0f,  0.0f,  1.0f, 0.5f, 1.0f, //0.0f, 1.0f,
    1.0f*x, 1.0f*y, 1.0f*z,  0.0f,  0.0f,  1.0f, 0.5f, 1.0f, //1.0f, 1.0f,
    0.0f*x, 1.0f*y, 1.0f*z,  0.0f,  0.0f,  1.0f, 0.5f, 1.0f, //1.0f, 0.0f,
    1.0f*x, 0.0f*y, 1.0f*z,  0.0f,  0.0f,  1.0f, 0.5f, 1.0f, //0.0f, 1.0f,
    0.0f*x, 1.0f*y, 1.0f*z,  0.0f,  0.0f,  1.0f, 0.5f, 1.0f, //1.0f, 0.0f,
    0.0f*x, 0.0f*y, 1.0f*z,  0.0f,  0.0f,  1.0f, 0.5f, 1.0f, //0.0f, 1.0f,
//face 6 {z=0}        0.0f, 0.0f, //
    0.0f*x, 0.0f*y, 0.0f*z,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f, //0.0f, 1.0f,
    0.0f*x, 1.0f*y, 0.0f*z,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f, //1.0f, 1.0f,
    1.0f*x, 1.0f*y, 0.0f*z,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f, //1.0f, 0.0f,
    0.0f*x, 0.0f*y, 0.0f*z,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f, //0.0f, 1.0f,
    1.0f*x, 1.0f*y, 0.0f*z,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f, //1.0f, 0.0f,
    1.0f*x, 0.0f*y, 0.0f*z,  0.0f,  0.0f, -1.0f, 0.5f, 0.5f //0.0f, 1.0f
};
}//namespace voxigen

#endif //_voxigen_simpleShapes_h_