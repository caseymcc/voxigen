#include "imglib/convolve.h"

#include "imglib/convolve_cl.h"
#include "imglib/gaussian_cl.h"
#include "imglib/kernels_cl.h"

namespace imglib{namespace cl
{

SeparableKernel buildGaussianSeparableKernel(::cl::Context &context, float sigma)
{
    SeparableKernel kernel;
    int size=(int)ceil((1.0+(sigma-0.8)/(0.3)));
    float sum=0.0f;
    float twoSigmaSquared=2*sigma*sigma;
    int kernelSize=2*size+1;
    std::vector<float> kernelBuffer(kernelSize);
    float temp;

    int x=-size;
    for(int i=0; i<kernelSize; ++i, ++x)
    {
        temp=std::exp(-(float)(x*x)/twoSigmaSquared);
        
        kernelBuffer[i]=temp;
        sum+=temp;
    }

    //normalize filter
    for(int i=0; i < kernelSize; i++)
        kernelBuffer[i]=kernelBuffer[i]/sum;

    kernel.kernel1=::cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*kernelBuffer.size(), kernelBuffer.data());
    kernel.kernel2=kernel.kernel1;
    kernel.size=kernelSize;

    return kernel;
}

void gaussian(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image2D &src, ::cl::Image2D &dst, float sigma, Direction direction, float scale,
    std::vector<::cl::Event> *events, ::cl::Event *event)
{
    int width, height;

    src.getImageInfo(CL_IMAGE_WIDTH, &width);
    src.getImageInfo(CL_IMAGE_HEIGHT, &height);

    gaussian(context, commandQueue, src, width, height, dst, sigma, direction, scale, events, event);
}

void gaussian(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image2D &src, int width, int height, ::cl::Image2D &dst, float sigma, Direction direction, float scale,
    std::vector<::cl::Event> *events, ::cl::Event *event)
{
    SeparableKernel convolveKernel=buildGaussianSeparableKernel(context, sigma);
    cl_image_format format;

    src.getImageInfo(CL_IMAGE_FORMAT, &format);

    ::cl::Image2D scratch(context, CL_MEM_READ_WRITE, ::cl::ImageFormat(format.image_channel_order, format.image_channel_data_type), width, height);

    gaussian(context, commandQueue, src, width, height, dst, scratch, convolveKernel, direction, scale, events, event);

    if(event!=nullptr)
        event->wait();
}

void gaussian(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image2D &src, int width, int height, ::cl::Image2D &dst, ::cl::Image2D &scratch, SeparableKernel &kernelBuffer,
    Direction direction, float scale, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    if(direction==Direction::Y)
        separableConvolve(context, commandQueue, src, width, height, dst, scratch, kernelBuffer.kernel1, kernelBuffer.size, kernelBuffer.kernel2, kernelBuffer.size, scale, events, event);
    else
        separableConvolve(context, commandQueue, src, width, height, dst, scratch, kernelBuffer.kernel2, kernelBuffer.size, kernelBuffer.kernel1, kernelBuffer.size, scale, events, event);
}

}}//namespace imglib::cl