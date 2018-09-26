#ifndef _imglib_utils_cl_h_
#define _imglib_utils_cl_h_

#include "imglib_export.h"
#ifdef IMGLIB_OPENCL_CPP
#include "CL/cl.hpp"
#else
#include "CL/cl.h"
#endif//IMGLIB_OPENCL_CPP


namespace imglib{namespace cl
{

IMGLIB_EXPORT void getDimensions(cl_mem image, size_t &width, size_t &height);
IMGLIB_EXPORT void getImageDetails(cl_mem image, cl_image_format &format, size_t &width, size_t &height);
///
/// Zeros values of a buffer
///
IMGLIB_EXPORT void zeroBuffer(cl_context context, cl_command_queue commandQueue, cl_mem buffer, size_t size, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
IMGLIB_EXPORT void zeroFloatBuffer(cl_context context, cl_command_queue commandQueue, cl_mem buffer, size_t size, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
IMGLIB_EXPORT void zeroIntBuffer(cl_context context, cl_command_queue commandQueue, cl_mem buffer, size_t size, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);

///
/// Zeros values of an image
///
IMGLIB_EXPORT void zeroImage(cl_context context, cl_command_queue commandQueue, cl_mem image, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
IMGLIB_EXPORT void zeroImage(cl_context context, cl_command_queue commandQueue, cl_mem image, cl_image_format &format, size_t width, size_t height, 
    size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);

#ifdef IMGLIB_OPENCL_CPP

inline void getDimensions(::cl::Image &image, size_t &width, size_t &height)
{
    getDimensions(image(), width, height);
}

inline void getImageDetails(::cl::Image &image, cl_image_format &format, size_t &width, size_t &height)
{
    getImageDetails(image(), format, width, height);
}

inline void zeroBuffer(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Buffer &buffer, size_t size, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    zeroBuffer(context(), commandQueue(), buffer(), size, 
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void zeroFloatBuffer(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Buffer &buffer, size_t size, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    zeroFloatBuffer(context(), commandQueue(), buffer(), size,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void zeroIntBuffer(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Buffer &buffer, size_t size, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    zeroIntBuffer(context(), commandQueue(), buffer(), size,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void zeroImage(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &image, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    cl_event tmp;

    zeroImage(context(), commandQueue(), image(),
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

#endif//IMGLIB_OPENCL_CPP

}}//namespace imglib::cl

#endif //_imglib_utils_cl_h_