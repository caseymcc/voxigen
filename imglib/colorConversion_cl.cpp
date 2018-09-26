#include "imglib/colorConversion_cl.h"
#include "imglib/kernels_cl.h"
#include "imglib/utils_cl.h"

#include <string>
#include <cassert>

namespace imglib{namespace cl
{

//from emebeded opencl source
extern std::string colorConversion_cl;

void rgbToRGBA(cl_context context, cl_command_queue commandQueue, cl_mem &src, size_t srcWidth, size_t srcHeight, cl_mem &dst, size_t dstWidth, size_t dstHeight, size_t eventCount, cl_event *events, cl_event *event)
{
    cl_int status;

    std::string kernalName;

    if(srcWidth%4 == 0)
        kernalName="rgbToRGBA_uint";
    else
    {
        assert(false);
        return;
    }

    cl_kernel kernel=getKernel(context, kernalName.c_str(), "colorConversion.cl", colorConversion_cl.c_str());
    assert(kernel);

    assert(kernel);
    status=clSetKernelArg(kernel, 0, sizeof(cl_mem), src);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 1, sizeof(size_t), &dstWidth);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 2, sizeof(size_t), &dstHeight);
    assert(status==CL_SUCCESS);
    status=clSetKernelArg(kernel, 3, sizeof(cl_mem), dst);
    assert(status==CL_SUCCESS);

    size_t offset[2]={0, 0};
    size_t local[2]={0, 0};
    size_t global[2]={dstWidth/4, dstHeight};

    status=clEnqueueNDRangeKernel(commandQueue, kernel, 2, offset, global, local, eventCount, events, event);
    assert(status==CL_SUCCESS);
}

}}//namespace imglib::cl
