#include "voxigen/initOpengl.h"

namespace voxigen
{

void initOpenGL(glbinding::GetProcAddress functionPointerResolver)
{
    static bool _initOpengl=false;

    if(!_initOpengl)
    {
        glbinding::initialize(functionPointerResolver);
        _initOpengl=true;
    }
}

} //namespace voxigen

