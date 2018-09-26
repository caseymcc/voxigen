#ifndef _imglib_operators_cl_h_
#define _imglib_operators_cl_h_

#include "imglib_export.h"
#include "CL/cl.hpp"

#include <vector>

namespace imglib{namespace cl
{

///
/// Sums values accross entire image
///
IMGLIB_EXPORT float sum(cl_context context, cl_command_queue commandQueue, cl_mem image);
template<typename _ImageType>
float sum(cl_context context, cl_command_queue commandQueue, _ImageType &image)
{
    if(location(image)!=Location::OpenCL)
    {
        assert(false);
        return 0.0f;
    }

    cl_mem native=(cl_mem)nativeId(image);

    sum(context, commandQueue, native);
}

IMGLIB_EXPORT void sum(cl_context context, cl_command_queue commandQueue, cl_mem image, size_t width, size_t height, cl_mem sumBuffer, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
template<typename _ImageType>
void sum(cl_context context, cl_command_queue commandQueue, _ImageType &image, cl_mem sumBuffer, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr)
{
    if(location(image)!=Location::OpenCL)
    {
        assert(false);
        return 0.0f;
    }

    cl_mem native=(cl_mem)nativeId(image);

    sum(context, commandQueue, native, width(srcImage), height(srcImage), sumBuffer, eventCount, events, event);
}


#ifdef IMGLIB_OPENCL_CPP

inline float sum(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &image)
{
    return sum(context(), commandQueue(), image());
}

inline void sum(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &image, size_t width, size_t height, ::cl::Buffer &sumBuffer, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    sum(context(), commandQueue(), image(), width, height, sumBuffer(),
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

#endif//IMGLIB_OPENCL_CPP

}}//namespace imglib::cl

#endif //_imglib_operators_cl_h_