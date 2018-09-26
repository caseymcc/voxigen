#ifndef _imglib_lalacian_cl_h_
#define _imglib_lalacian_cl_h_

#include "imglib_export.h"
#include "imglib/convolve.h"

#include "CL/cl.hpp"

namespace imglib{namespace cl
{

struct IMGLIB_EXPORT Kernel
{
    ::cl::Buffer kernel;
    int size;
};

struct IMGLIB_EXPORT SeparableKernel
{
    ::cl::Buffer kernel1;
    ::cl::Buffer kernel2;
    int size;
};

///
/// Performs convolution on serparable kernels
///
IMGLIB_EXPORT void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, ::cl::Image &dst, SeparableKernel &kernel, float scale=1.0,
    std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
IMGLIB_EXPORT void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height, ::cl::Image &dst, SeparableKernel &kernel, float scale=1.0,
    std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
IMGLIB_EXPORT void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height, ::cl::Image &dst, ::cl::Image &scratch,
    SeparableKernel &kernel, float scale=1.0f, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
IMGLIB_EXPORT void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height,  ::cl::Image &dst, ::cl::Image &scratch,
    ::cl::Buffer &kernelXBuffer, int kernelXSize, ::cl::Buffer &kernelYBuffer, int kernelYSize, float scale=1.0f, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
IMGLIB_EXPORT void separableConvolve_local(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height, ::cl::Image &dst, ::cl::Image &scratch,
    ::cl::Buffer &kernelXBuffer, int kernelXSize, ::cl::Buffer &kernelYBuffer, int kernelYSize, float scale=1.0f, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);

}}//namespace imglib::cl

#endif //_imglib_lalacian_cl_h_