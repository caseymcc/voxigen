#include "atomic.cl"

__kernel void laplacian(read_only image2d_t input, int width, int height)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
}
