#ifndef _imglib_kernels_cl_h_
#define _imglib_kernels_cl_h_

#include "imglib_export.h"

#include <CL/cl.hpp>
#include <memory>

namespace imglib{namespace cl
{

struct  KernelInfo
{
    cl_kernel kernel;

    size_t workGroupSize;
    size_t preferredWorkGroupMultiple;
    cl_ulong localMemoryUsed;

    cl_uint deviceComputeUnits;
    cl_ulong deviceLocalMemory;
};

IMGLIB_EXPORT cl_kernel getKernel(cl_context context, const char *kernelName, const char *programName, const char *source);
IMGLIB_EXPORT KernelInfo getKernelInfo(cl_context context, const char *, const char *programName, const char *source);

}}//namespace imglib::cl

#endif //_imglib_kernels_cl_h_