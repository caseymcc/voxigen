#include "imglib/utils_cl.h"
#include "imglib/kernels_cl.h"
#include <cassert>

namespace imglib{namespace cl
{

//from emebeded opencl source
extern std::string utils_cl;

void getDimensions(cl_mem image, size_t &width, size_t &height)
{
    clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(size_t), &width, nullptr);
    clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(size_t), &height, nullptr);
}

void getImageDetails(cl_mem image, cl_image_format &format, size_t &width, size_t &height)
{
    clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &format, nullptr);
    clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(size_t), &width, nullptr);
    clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(size_t), &height, nullptr);
}

void zeroBuffer(cl_context context, cl_command_queue commandQueue, cl_mem buffer, size_t size, size_t eventCount, cl_event *events, cl_event *event)
{
    cl_int status;
    cl_kernel  kernel=getKernel(context, "zeroMemory", "utils.cl", utils_cl.c_str());

    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), buffer);
    assert(status==CL_SUCCESS);

    size_t offset[1]={0};
    size_t local[1]={0};
    size_t global[1]={size};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 1, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

void zeroFloatBuffer(cl_context context, cl_command_queue commandQueue, cl_mem buffer, size_t size, size_t eventCount, cl_event *events, cl_event *event)
{
    cl_int status;
    cl_kernel  kernel=getKernel(context, "zeroFloatMemory", "utils.cl", utils_cl.c_str());

    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), buffer);
    assert(status==CL_SUCCESS);

    size_t offset[1]={0};
    size_t local[1]={0};
    size_t global[1]={size};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 1, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

void zeroIntBuffer(cl_context context, cl_command_queue commandQueue, cl_mem buffer, size_t size, size_t eventCount, cl_event *events, cl_event *event)
{
    cl_int status;
    cl_kernel  kernel=getKernel(context, "zeroIntMemory", "utils.cl", utils_cl.c_str());

    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), buffer);
    assert(status==CL_SUCCESS);

    size_t offset[1]={0};
    size_t local[1]={0};
    size_t global[1]={size};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 1, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

void zeroImage(cl_context context, cl_command_queue commandQueue, cl_mem image, size_t eventCount, cl_event *events, cl_event *event)
{
    cl_image_format format;
    size_t width, height;

    getImageDetails(image, format, width, height);

    zeroImage(context, commandQueue, image, format, width, height, eventCount, events, event);
}

void zeroImage(cl_context context, cl_command_queue commandQueue, cl_mem image, cl_image_format &format, size_t width, size_t height, size_t eventCount, cl_event *events, cl_event *event)
{
    cl_int status;
    cl_kernel  kernel=getKernel(context, "zeroFloatImage", "utils.cl", utils_cl.c_str());

    assert(kernel);

    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), image);
    assert(status==CL_SUCCESS);

    size_t offset[2]={0, 0};
    size_t local[2]={0, 0};
    size_t global[2]={width, height};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 2, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

}}//namespace imglib::cl
