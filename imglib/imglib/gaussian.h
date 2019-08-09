#ifndef _imglib_gaussian_h_
#define _imglib_gaussian_h_

#include "imglib_export.h"
#include "imglib/convolve.h"

namespace imglib{
{

///
/// Construct guassian kernel as 2 kernels that work on the x/y direction.
///
IMGLIB_EXPORT SeparableKernel buildGaussianSeparableKernel(float sigma, int &filterSize);

///
/// Runs Guassian Kernel
///
template<typename _Image>
void gaussian(_Image &image, float sigma=0.4, Direction direction=Direction::X, float scale=1.0)
{
}

}//namespace imglib

#endif //_imglib_gaussian_h_