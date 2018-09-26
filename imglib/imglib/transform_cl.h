#ifndef _imglib_transform_cl_h_
#define _imglib_transform_cl_h_

#ifdef IMGLIB_OPENCL

#include "imglib_export.h"
#include "CL/cl.hpp"
#include "imglib/utils_cl.h"

namespace imglib{namespace cl
{

IMGLIB_EXPORT void reduceSimple(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
inline void reduceSimple(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_mem dstImage, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;
//    size_t paramSize;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    reduceSimple(context, commandQueue, srcImage, srcFormat, srcWidth, srcHeight, dstImage, dstFormat, dstWidth, dstHeight, eventCount, events, event);
}

IMGLIB_EXPORT void expandSimple(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
inline void expandSimple(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_mem dstImage, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;
//    size_t paramSize;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    expandSimple(context, commandQueue, srcImage, srcFormat, srcWidth, srcHeight, dstImage, dstFormat, dstWidth, dstHeight, eventCount, events, event);
}

IMGLIB_EXPORT void reduce(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
inline void reduce(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_mem dstImage, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;
//    size_t paramSize;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    reduce(context, commandQueue, srcImage, srcFormat, srcWidth, srcHeight, dstImage, dstFormat, dstWidth, dstHeight, eventCount, events, event);
}

IMGLIB_EXPORT void expand(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);
inline void expand(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_mem dstImage, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;
//    size_t paramSize;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    expand(context, commandQueue, srcImage, srcFormat, srcWidth, srcHeight, dstImage, dstFormat, dstWidth, dstHeight, eventCount, events, event);
}

#ifdef IMGLIB_OPENCL_CPP

inline void reduceSimple(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, ::cl::Image &dstImage, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    cl_event tmp;

    reduceSimple(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight, 
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void reduceSimple(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    ::cl::Image &dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    cl_event tmp;

    reduceSimple(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void expandSimple(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, ::cl::Image &dstImage, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    cl_event tmp;

    expandSimple(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void expandSimple(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    ::cl::Image &dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    expandSimple(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void reduce(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, ::cl::Image &dstImage, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    cl_event tmp;

    reduce(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void reduce(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    ::cl::Image &dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    reduce(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void expand(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, ::cl::Image &dstImage, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_image_format srcFormat;
    size_t srcWidth, srcHeight;
    cl_image_format dstFormat;
    size_t dstWidth, dstHeight;

    getImageDetails(srcImage, srcFormat, srcWidth, srcHeight);
    getImageDetails(dstImage, dstFormat, dstWidth, dstHeight);

    cl_event tmp;

    expand(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void expand(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    ::cl::Image &dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    expand(context(), commandQueue(), srcImage(), srcFormat, srcWidth, srcHeight, dstImage(), dstFormat, dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

#endif//IMGLIB_OPENCL_CPP
}}//namespace imglib::cl

#endif //IMGLIB_OPENCL

#endif //_imglib_transform_cl_h_