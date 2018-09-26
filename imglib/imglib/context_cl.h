#ifndef _imglib_context_cl_h_
#define _imglib_context_cl_h_

#include "imglib_export.h"

#include <CL/cl.hpp>

#include <limits>
#include <memory>

namespace imglib{namespace cl
{

struct KernelInfo
{
    ::cl::Kernel kernel;
    
    size_t workGroupSize;
    size_t preferredWorkGroupMultiple;
    cl_ulong localMemoryUsed;

    cl_uint deviceComputeUnits;
    cl_ulong deviceLocalMemory;
};
typedef std::shared_ptr<KernelInfo> SharedKernelInfo;

::cl::Kernel &getKernel(::cl::Context context, std::string kernelName, std::string fileName);
SharedKernelInfo getKernelInfo(::cl::Context context, std::string kernelName, std::string fileName);

struct IMGLIB_EXPORT OpenClDevice
{
    enum Type
    {
        CPU,
        GPU
    };
    cl_device_id deviceId;
    std::string name;
    Type type;

    std::string platform;
    std::string vendor;
    std::string version;

    cl_uint computeUnits;
    std::string builtInKernels;
    std::string extensions;
    cl_ulong globalCache;
    cl_ulong globalMemory;
    cl_ulong localMemory;
    size_t maxWorkGroupSize;
    cl_uint maxWorkItemDims;
    std::vector<size_t> maxWorkItemSizes;
};

IMGLIB_EXPORT std::vector<OpenClDevice> getDevices();

IMGLIB_EXPORT ::cl::Context openDevice(OpenClDevice &selectedDevice);
IMGLIB_EXPORT ::cl::Context openDevice(std::string deviceName, OpenClDevice &deviceInfo);
IMGLIB_EXPORT ::cl::Context openDevice(std::string platform, std::string deviceName, OpenClDevice &deviceInfo);

IMGLIB_EXPORT ::cl::Context getCurrentContext();
IMGLIB_EXPORT void setCurrentContext(::cl::Context &context);

IMGLIB_EXPORT ::cl::CommandQueue getCurrentCommandQueue();
IMGLIB_EXPORT void setCurrentCommandQueue(::cl::CommandQueue &commandqueue);

}}//namespace imglib::cl

#endif //_imglib_context_cl_h_