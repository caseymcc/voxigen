#ifndef _imglib_scharr_cl_h_
#define _imglib_scharr_cl_h

#include "imglib_export.h"
#include "imglib/convolve_cl.h"

#include "CL/cl.hpp"

namespace imglib{namespace cl
{

///
/// Builds Scharr Kernel, for use with a separable convolution
///
IMGLIB_EXPORT SeparableKernel buildScharrSeparableKernel(::cl::Context &context, int kernelSize, bool normalize);

///
/// Runs Scharr Kernel
///
IMGLIB_EXPORT void scharr(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, ::cl::Image &dst, int kernelSize=3, Direction direction=Direction::X, bool normalize=true, float scale=1.0,
    std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
IMGLIB_EXPORT void scharr(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, size_t width, size_t height, ::cl::Image &dst, int kernelSize=3, Direction direction=Direction::X, bool normalize=true, float scale=1.0,
    std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
IMGLIB_EXPORT void scharr(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, size_t width, size_t height, ::cl::Image &dst, ::cl::Image &scratch, SeparableKernel &kernelBuffer, Direction direction=Direction::X,
    bool normalize=true, float scale=1.0, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
}}//namespace cvlib::cl

#endif //_cvlib_scharr_cl_h