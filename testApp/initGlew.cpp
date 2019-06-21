#include "initGlew.h"
#include <GL/glew.h>

void initGlew()
{
    static bool isGlewInit=false;

    if(!isGlewInit)
    {
        glewInit();
        isGlewInit=true;
    }
}