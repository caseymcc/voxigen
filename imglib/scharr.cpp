#include "imglib/scharr_cl.h"

namespace imglib
{

void scharr(uint8_t *srcBuffer, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstWidth, size_t dstHeight, int kernelSize=3, Direction direction=Direction::X, bool normalize=true, float scale=1.0)
{
    SeparableKernel imageKernel=buildScharrSeparableKernel();

    scharr(srcBuffer, srcWidth, srcHeight, dstBuffer, dstWidth, dstHeight, imageKernel);
}

void scharr(uint8_t *srcBuffer, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstWidth, size_t dstHeight, SeparableKernel &kernel)
{
    //not implemented
    assert(false);
}

}}//namespace imglib

#endif //_imglib_scharr_h