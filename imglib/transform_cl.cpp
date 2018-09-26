#include "imglib/transform_cl.h"
#include "imglib/kernels_cl.h"

#include <string>
#include <cassert>

namespace imglib{namespace cl
{

//from emebeded opencl source
extern std::string transform_cl;

std::string buildKernelName(std::string kernel, cl_image_format &format)
{
    std::string kernelName=kernel;

    switch(format.image_channel_data_type)
    {
    case CL_UNSIGNED_INT8:
        kernelName+="_uint";
        break;
    case CL_FLOAT:
        kernelName+="_float";
        break;
    default:
        assert(false); //not suported
        return kernelName;
        break;
    }

    switch(format.image_channel_order)
    {
    case CL_R:
    case CL_A:
        break;
    case CL_RGBA:
    case CL_BGRA:
        kernelName+="4";
        break;
    default:
        assert(false); //not suported
        return kernelName;
        break;
    }

    return kernelName;
}

void reduceSimple(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight, 
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount, cl_event *events, cl_event *event)
{
    if((srcFormat.image_channel_data_type!=dstFormat.image_channel_data_type) || (srcFormat.image_channel_order!=dstFormat.image_channel_order))
    {
        assert(false);
        return;
    }

    cl_int status;

    std::string kernelName=buildKernelName("reduceImageSimple", srcFormat);
    cl_kernel kernel=getKernel(context, kernelName.c_str(), "transform.cl", transform_cl.c_str());

    assert(kernel);
    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), srcImage);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 1, sizeof(size_t), &srcWidth);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 2, sizeof(size_t), &srcHeight);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 3, sizeof(cl_mem), dstImage);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 4, sizeof(size_t), &dstWidth);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 5, sizeof(size_t), &dstHeight);
    assert(status==CL_SUCCESS);
    
    assert(srcWidth/2==dstWidth);
    assert(srcHeight/2==dstHeight);

    size_t offset[2]={0, 0};
    size_t local[2]={0, 0};
    size_t global[2]={dstWidth, dstHeight};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 2, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

void expandSimple(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount, cl_event *events, cl_event *event)
{
    if((srcFormat.image_channel_data_type!=dstFormat.image_channel_data_type)||(srcFormat.image_channel_order!=dstFormat.image_channel_order))
    {
        assert(false);
        return;
    }

    cl_int status;

    std::string kernelName=buildKernelName("expandSimple", srcFormat);
    cl_kernel kernel=getKernel(context, kernelName.c_str(), "transform.cl", transform_cl.c_str());
    assert(kernel);

    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), srcImage);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 1, sizeof(cl_mem), dstImage);
    assert(status==CL_SUCCESS);

    assert(srcWidth==dstWidth/2);
    assert(srcHeight==dstHeight/2);

    size_t offset[2]={0, 0};
    size_t local[2]={0, 0};
    size_t global[2]={dstWidth, dstHeight};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 2, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

void reduce(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight, 
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount, cl_event *events, cl_event *event)
{
    if((srcFormat.image_channel_data_type!=dstFormat.image_channel_data_type)||(srcFormat.image_channel_order!=dstFormat.image_channel_order))
    {
        assert(false);
        return;
    }

    cl_int status;
    std::string kernelName=buildKernelName("reduce", srcFormat);
    cl_kernel kernel=getKernel(context, kernelName.c_str(), "transform.cl", transform_cl.c_str());
    assert(kernel);

    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), srcImage);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 1, sizeof(size_t), &srcWidth);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 2, sizeof(size_t), &srcHeight);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 3, sizeof(cl_mem), dstImage);
    assert(status==CL_SUCCESS);

    assert(srcWidth/2==dstWidth);
    assert(srcHeight/2==dstHeight);

    size_t offset[2]={0, 0};
    size_t local[2]={0, 0};
    size_t global[2]={dstWidth, dstHeight};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 2, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

void expand(cl_context context, cl_command_queue commandQueue, cl_mem srcImage, cl_image_format &srcFormat, size_t srcWidth, size_t srcHeight,
    cl_mem dstImage, cl_image_format &dstFormat, size_t dstWidth, size_t dstHeight, size_t eventCount, cl_event *events, cl_event *event)
{
    if((srcFormat.image_channel_data_type!=dstFormat.image_channel_data_type)||(srcFormat.image_channel_order!=dstFormat.image_channel_order))
    {
        assert(false);
        return;
    }

    cl_int status;

    std::string kernelName=buildKernelName("expand", srcFormat);
    cl_kernel kernel=getKernel(context, kernelName.c_str(), "transform.cl", transform_cl.c_str());
    assert(kernel);

    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), srcImage);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 1, sizeof(cl_mem), dstImage);
    assert(status==CL_SUCCESS);

    assert(srcWidth==dstWidth/2);
    assert(srcHeight==dstHeight/2);

    size_t offset[2]={0, 0};
    size_t local[2]={0, 0};
    size_t global[2]={dstWidth, dstHeight};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 2, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

}}//namespace imglib::cl
