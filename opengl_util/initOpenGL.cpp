#include "opengl_util/initOpenGL.h"

//#include <GL/glew.h>
#include <glbinding/gl/gl.h>

namespace opengl_util
{

void initOpenGL()
{
    static bool _initOpenGL=false;

    if(!_initOpenGL)
    {
//        glewInit();
        _initOpenGL=true;
    }
}

} //namespace opengl_util

