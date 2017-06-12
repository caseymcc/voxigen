#include "voxigen/initGlew.h"
#include <GL/glew.h>

namespace voxigen
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

} //namespace voxigen

