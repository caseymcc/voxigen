#ifndef _imglib_scharr_h_
#define _imglib_scharr_h

#include "imglib_export.h"
#include "imglib/convolve_cl.h"

namespace imglib
{

///
/// Builds Scharr Kernel, for use with a separable convolution
///
IMGLIB_EXPORT SeparableKernel buildScharrSeparableKernel(int kernelSize, bool normalize);

template<typename _SourceImage, typename _DestinationImage>
void scharr(_SourceImage &src, _DestinationImage &dst, int kernelSize=3, Direction direction=Direction::X, bool normalize=true, float scale=1.0);

///
/// Runs Scharr Kernel
///
IMGLIB_EXPORT void scharr(uint8_t *srcBuffer, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstWidth, size_t dstHeight, int kernelSize=3, Direction direction=Direction::X, bool normalize=true, float scale=1.0);
IMGLIB_EXPORT void scharr(uint8_t *srcBuffer, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstWidth, size_t dstHeight, SeparableKernel &kernel);

}}//namespace imglib

#endif //_imglib_scharr_h