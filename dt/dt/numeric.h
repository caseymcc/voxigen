#ifndef H_NUMERIC
#define H_NUMERIC

#include <cmath>
#include <limits>

/**
 * @brief use of machine epsilon to compare floating-point values for equality
 * http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
 */
bool almost_equal(const float x, const float y, int ulp=2);

float half(const float x);

#endif
