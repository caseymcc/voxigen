#include "opengl_util/initOpenGL.h"

#include <glbinding/gl/gl.h>

namespace opengl_util
{

void initOpenGL()
{
    static bool _initOpenGL=false;

    if(!_initOpenGL)
    {
        _initOpenGL=true;
    }
}

} //namespace opengl_util

