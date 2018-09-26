#include "imglib/convolve_cl.h"
#include "imglib/kernels_cl.h"
#include "imglib/utils_cl.h"

#include "Eigen/Core"

namespace imglib{namespace cl
{

//from emebeded opencl source
extern std::string convolve_cl;

void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, ::cl::Image &dst, SeparableKernel &kernel,
    float scale, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    size_t width, height;

    getDimensions(src, width, height);

    separableConvolve(context, commandQueue, src, width, height, dst, kernel, scale, events, event);
}

void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height, ::cl::Image &dst, SeparableKernel &kernel,
    float scale, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    cl_image_format format;
    
    src.getImageInfo(CL_IMAGE_FORMAT, &format);

    ::cl::Image2D scratch(context, CL_MEM_READ_WRITE, ::cl::ImageFormat(format.image_channel_order, format.image_channel_data_type), width, height);

    separableConvolve(context, commandQueue, src, width, height, dst, scratch, kernel, scale, events, event);

    if(event != nullptr)
        event->wait();
}

void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height, ::cl::Image &dst, ::cl::Image &scratch,
    SeparableKernel &kernel, float scale, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    separableConvolve(context, commandQueue, src, width, height, dst, scratch, kernel.kernel1, kernel.size, kernel.kernel1, kernel.size, scale, events, event);
}

void separableConvolve(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height, ::cl::Image &dst, ::cl::Image &scratch,
    ::cl::Buffer &kernelXBuffer, int kernelXSize, ::cl::Buffer &kernelYBuffer, int kernelYSize, float scale, std::vector<::cl::Event> *events, ::cl::Event *event)
{
//    ::cl::Kernel kernelX=getKernel(context, "separableConvolveXImage", "convolve.cl", convolve_cl);
//    ::cl::Kernel kernelY=getKernel(context, "separableConvolveYImage", "convolve.cl", convolve_cl);
//    cl_int status;
//    int index=0;
//    std::vector<::cl::Event> kernelYEvent(0);
//
//    status=kernelY.setArg(index++, src);
//    status=kernelY.setArg(index++, kernelYBuffer);
//    status=kernelY.setArg(index++, kernelYSize);
//    status=kernelY.setArg(index++, (float)1.0); //only scale once, so no scale here
//    status=kernelY.setArg(index++, scratch);
//
//    ::cl::NDRange globalThreads(width, height);
//
//    status=commandQueue.enqueueNDRangeKernel(kernelY, ::cl::NullRange, globalThreads, ::cl::NullRange, events, &kernelYEvent[0]);
//
//    index=0;
//    status=kernelX.setArg(index++, scratch);
//    status=kernelX.setArg(index++, kernelXBuffer);
//    status=kernelX.setArg(index++, kernelXSize);
//    status=kernelX.setArg(index++, scale);
//    status=kernelX.setArg(index++, dst);
//
//    status=commandQueue.enqueueNDRangeKernel(kernelX, ::cl::NullRange, globalThreads, ::cl::NullRange, &kernelYEvent, event);
    separableConvolve_local(context, commandQueue, src, width, height, dst, scratch, kernelXBuffer, kernelXSize, kernelYBuffer, kernelYSize, scale, events, event);
}

void separableConvolve_local(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, int width, int height, ::cl::Image &dst, ::cl::Image &scratch, 
    ::cl::Buffer &kernelXBuffer, int kernelXSize, ::cl::Buffer &kernelYBuffer, int kernelYSize, float scale, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    ::cl::Kernel kernelX=getKernel(context, "separableConvolveXImage2D_local", "convolve.cl", convolve_cl);
    ::cl::Kernel kernelY=getKernel(context, "separableConvolveYImage2D_local", "convolve.cl", convolve_cl);
    cl_int status;
    std::vector<::cl::Event> kernelYEvent(1);

    size_t localX=16;
    size_t localY=16;
    size_t globalX=(width/localX)*localX;
    size_t globalY=(height/localY)*localY;

    if(globalX<width)
        globalX+=localX;
    if(globalY<height)
        globalY+=localY;

    int cacheX=(kernelXSize/2)*2+localX;
    int cacheY=(kernelYSize/2)*2+localY;

    int index=0;
    status=kernelY.setArg(index++, src);
    assert(status==CL_SUCCESS);
    status=kernelY.setArg(index++, (int)width);
    assert(status==CL_SUCCESS);
    status=kernelY.setArg(index++, (int)height);
    assert(status==CL_SUCCESS);
    status=kernelY.setArg(index++, kernelYBuffer);
    assert(status==CL_SUCCESS);
    status=kernelY.setArg(index++, kernelYSize);
    assert(status==CL_SUCCESS);
    status=kernelY.setArg(index++, (float)1.0); //only scale once, so no scale here
    assert(status==CL_SUCCESS);
    status=kernelY.setArg(index++, scratch);
    assert(status==CL_SUCCESS);
    status=kernelY.setArg(index++, cacheX*cacheY*sizeof(float), nullptr); //setup local image cache
    assert(status==CL_SUCCESS);

    ::cl::NDRange globalThreads(globalX, globalY);
    ::cl::NDRange localThreads(localX, localY);

    status=commandQueue.enqueueNDRangeKernel(kernelY, ::cl::NullRange, globalThreads, localThreads, events, &kernelYEvent[0]);
    assert(status==CL_SUCCESS);

    index=0;
    status=kernelX.setArg(index++, scratch);
    assert(status==CL_SUCCESS);
    status=kernelX.setArg(index++, (int)width);
    assert(status==CL_SUCCESS);
    status=kernelX.setArg(index++, (int)height);
    assert(status==CL_SUCCESS);
    status=kernelX.setArg(index++, kernelXBuffer);
    assert(status==CL_SUCCESS);
    status=kernelX.setArg(index++, kernelXSize);
    assert(status==CL_SUCCESS);
    status=kernelX.setArg(index++, scale);
    assert(status==CL_SUCCESS);
    status=kernelX.setArg(index++, dst);
    assert(status==CL_SUCCESS);
    status=kernelX.setArg(index++, cacheX*cacheY*sizeof(float), nullptr); //setup local image cache
    assert(status==CL_SUCCESS);

    status=commandQueue.enqueueNDRangeKernel(kernelX, ::cl::NullRange, globalThreads, localThreads, &kernelYEvent, event);
    assert(status==CL_SUCCESS);
}

}}//namespace imglib::cl