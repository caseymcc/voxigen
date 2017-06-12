#include "opengl_util/initGlew.h"

#include <GL/glew.h>

namespace opengl_util
{

void initGlew()
{
    static bool _initGlew=false;

    if(!_initGlew)
    {
        glewInit();
        _initGlew=true;
    }
}

} //namespace opengl_util

