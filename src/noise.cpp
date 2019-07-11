#include "voxigen/noise.h"

namespace voxigen
{

void initNoise()
{
    static bool noiseInit=false;

    if(!noiseInit)
    {
        HastyNoise::loadSimd("");
        noiseInit=true;
    }
}

}//namespace voxigen