#include "imglib/operators_cl.h"
#include "imglib/kernels_cl.h"

#include <string>
#include <cassert>

namespace imglib{namespace cl
{

//from emebeded opencl source
extern std::string operators_cl;

float sum(cl_context context, cl_command_queue commandQueue, cl_mem image)
{
//    ::cl::Buffer sumBuffer;
    cl_mem sumBuffer;

    size_t width, height;
    cl_event sumEvent;
    cl_event readEvent;

    float sumValue;

    cl_int error;

    sumBuffer=::clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float), nullptr, &error);

    clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(size_t), &width, nullptr);
    clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(size_t), &height, nullptr);

    sum(context, commandQueue, image, width, height, sumBuffer, 0, nullptr, &sumEvent);
    clEnqueueReadBuffer(commandQueue, sumBuffer, false, 0, sizeof(float), &sumValue, 1, &sumEvent, &readEvent);
    clWaitForEvents(1, &readEvent);

    return sumValue;
}

void sum(cl_context context, cl_command_queue commandQueue, cl_mem image, size_t width, size_t height, cl_mem sumBuffer, size_t eventCount, cl_event *events, cl_event *event)
{
    cl_int status;
    cl_kernel kernel=getKernel(context, "sum", "operators.cl", operators_cl.c_str());
    assert(kernel);

    assert(kernel);
    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), image);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 1, sizeof(size_t), &width);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 2, sizeof(size_t), &height);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 3, sizeof(cl_mem), sumBuffer);
    assert(status==CL_SUCCESS);

    size_t offset[1]={0};
    size_t local[1]={0};
    size_t global[1]={height};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 1, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

}}//namespace imglib::cl