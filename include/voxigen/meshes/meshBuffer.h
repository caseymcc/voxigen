#ifndef _voxigen_meshBuffer_h_
#define _voxigen_meshBuffer_h_

//#include "voxigen/defines.h"
#include <glbinding/gl/gl.h>
//typedef struct __GLsync *GLsync;

namespace voxigen
{

struct MeshBuffer
{
    MeshBuffer():valid(false), ready(false) {}

    bool valid;
    bool ready;

    //    unsigned int vertexArray;
    unsigned int vertexBuffer;
    unsigned int indexBuffer;
    unsigned int indices;

    unsigned int indexType;

    gl::GLsync sync;
    unsigned int frame;
};

} //namespace voxigen

#endif //_voxigen_meshBuffer_h_ 