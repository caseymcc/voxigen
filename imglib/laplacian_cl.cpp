#include "imglib/laplacian_cl.h"
#include "imglib/kernels_cl.h"

#include <string>
#include <cassert>

namespace imglib{namespace cl
{

//from emebeded opencl source
extern std::string laplacian_cl;

void laplacianPyramid(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &image, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    cl_int status;
    size_t width, height;
    ::cl::Kernel kernel=getKernel(context, "laplacianPyramid", "laplacian.cl", laplacian_cl);

    assert(kernel());

    image.getImageInfo(CL_IMAGE_WIDTH, &width);
    image.getImageInfo(CL_IMAGE_HEIGHT, &height);
    
    status=kernel.setArg(0, image);
    assert(status==CL_SUCCESS);

    ::cl::NDRange globalThreads(width, height);

    status=commandQueue.enqueueNDRangeKernel(kernel, ::cl::NullRange, globalThreads, ::cl::NullRange, events, event);
    assert(status==CL_SUCCESS);
}

}}//namespace imglib::cl