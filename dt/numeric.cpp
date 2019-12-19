#include "dt/numeric.h"

bool almost_equal(const float x, const float y, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x-y)<=std::numeric_limits<float>::epsilon() * std::abs(x+y) * static_cast<float>(ulp)
        // unless the result is subnormal
        ||std::abs(x-y)<std::numeric_limits<float>::min();
}

float half(const float x)
{
	return 0.5f * x;
}
