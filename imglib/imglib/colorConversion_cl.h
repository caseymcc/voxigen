#ifndef _imglib_colorConversion_cl_h_
#define _imglib_colorConversion_cl_h_

#ifdef IMGLIB_OPENCL

#include "imglib_export.h"

#ifdef IMGLIB_OPENCL_CPP
#include "CL/cl.hpp"
#else
#include "CL/cl.h"
#endif
#include "imglib/utils_cl.h"

#include <cassert>

namespace imglib{namespace cl
{

IMGLIB_EXPORT void rgbToRGBA(cl_context context, cl_command_queue commandQueue, cl_mem &src, size_t srcWidth, size_t srcHeight,
    cl_mem &dst, size_t dstWidth, size_t dstHeight, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr);

template<typename _ImageType1, typename _ImageType2>
void rgbToRGBA(cl_context context, cl_command_queue commandQueue, _ImageType1 &srcImage, _ImageType2 &dstImage, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr)
{
    if((location(srcImage)!=Location::OpenCL)||(location(dstImage)==Location::OpenCL))
    {
        assert(false);
        return;
    }

    cl_mem_object_type memoryType;

    cl_mem nativeSrc=(cl_mem)nativeId(srcImage);
    clGetMemObjectInfo(nativeSrc, CL_MEM_TYPE, sizeof(cl_mem_object_type), &memoryType, nullptr);

    if(memoryType!=CL_MEM_OBJECT_BUFFER)
    {
        assert(false);
        return;
    }

    cl_mem nativeDst=(cl_mem)nativeId(dstImage);
    clGetMemObjectInfo(nativeDst, CL_MEM_TYPE, sizeof(cl_mem_object_type), &memoryType, nullptr);

    if(memoryType!=CL_MEM_OBJECT_IMAGE2D)
    {
        assert(false);
        return;
    }

    rgbToRGBA(context, commandQueue, nativeSrc, width(srcImage), height(srcImage), nativeDst, width(dstImage), height(dstImage), eventCount, events, event);
}

template<typename _ImageType1, typename _ImageType2>
bool convert(cl_context context, cl_command_queue commandQueue, _ImageType1 &srcImage, _ImageType2 &dstImage, size_t eventCount=0, cl_event *events=nullptr, cl_event *event=nullptr)
{
    if((location(srcImage)!=Location::OpenCL)||(location(dstImage)==Location::OpenCL))
    {
        assert(false);
        return false;
    }

    switch(format(srcImage))
    {
    case Format::RGB:
    {
        switch(format(dstImage))
        {
        case Format::RGBA:
            cl::rgbToRGBA(context, commandQueue, srcImage, dstImage, eventCount, events, event);
            return true;
        default:
            assert(false); //not implemented
            return false;
            break;
        }
    }
    break;
    default:
        assert(false); //not implemented
        return false;
        break;
    }

    return false;
}

#ifdef IMGLIB_OPENCL_CPP

inline void rgbToRGBA(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Buffer &srcImage, size_t srcWidth, size_t srcHeight, ::cl::Image &dstImage, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    size_t dstWidth, dstHeight;

    getDimensions(dstImage, dstWidth, dstHeight);

    cl_event tmp;

    rgbToRGBA(context(), commandQueue(), srcImage(), srcWidth, srcHeight, dstImage(), dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

inline void rgbToRGBA(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Buffer &srcImage, size_t srcWidth, size_t srcHeight,
    ::cl::Image &dstImage, size_t dstWidth, size_t dstHeight, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    rgbToRGBA(context(), commandQueue(), srcImage(), srcWidth, srcHeight, dstImage(), dstWidth, dstHeight,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

template<typename _ImageType1, typename _ImageType2>
void rgbToRGBA(::cl::Context &context, ::cl::CommandQueue &commandQueue, _ImageType1 &srcImage, _ImageType2 &dstImage, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    rgbToRGBA(context(), commandQueue(), srcImage, dstImage, 
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

template<typename _ImageType1, typename _ImageType2>
void convert(::cl::Context &context, ::cl::CommandQueue &commandQueue, _ImageType1 &srcImage, _ImageType2 &dstImage, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
{
    cl_event tmp;

    convert(context(), commandQueue(), srcImage, dstImage,
        (events!=nullptr)?(cl_uint)events->size():0,
        (events!=nullptr && events->size() > 0)?(cl_event*)&events->front():nullptr,
        (event!=nullptr)?&tmp:nullptr);

    if(event!=nullptr)
        *event=tmp;
}

#endif//IMGLIB_OPENCL_CPP

}}//namespace imglib::cl

#endif //IMGLIB_OPENCL

#endif //_imglib_colorConversion_cl_h_