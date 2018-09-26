#include "imglib/sobel_cl.h"
#include "imglib/kernels_cl.h"

#include "imglib/convolve.h"

#include "Eigen/Core"

namespace imglib{namespace cl
{

SeparableKernel buildSobelSeparableKernel(::cl::Context &context, int kernelSize, bool normalize)
{
    SeparableKernel kernel;

    assert(kernelSize%2==1);
    int convolveSize=kernelSize/2;

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> smoothingKernel;
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> edgeKernel;
    Eigen::Vector3f smoothingVector;
    Eigen::Vector3f edgeVector;

    float smoothNorm=4.0;
    float edgeNorm=2.0;

    smoothingVector<<1, 2, 1;
    edgeVector<<-1, 0, 1;

    smoothingKernel=smoothingVector;
    edgeKernel=edgeVector;

    while(convolveSize>1)
    {
        smoothingKernel=convolveMatrix(smoothingKernel, smoothingVector);
        smoothNorm*=4.0;
        edgeKernel=convolveMatrix(edgeKernel, smoothingVector);
        edgeNorm*=4.0;
        convolveSize--;
    }

    if(normalize)
    {
        smoothingKernel/=smoothNorm;
        edgeKernel/=edgeNorm;
    }

    kernel.kernel1=::cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*smoothingKernel.size(), smoothingKernel.data());
    kernel.kernel2=::cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*edgeKernel.size(), edgeKernel.data());

    return kernel;
}

void sobel(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, ::cl::Image &dst, int kernelSize, Direction direction, bool normalize, float scale,
    std::vector<::cl::Event> *events, ::cl::Event *event)
{
    int width, height;

    src.getImageInfo(CL_IMAGE_WIDTH, &width);
    src.getImageInfo(CL_IMAGE_HEIGHT, &height);

    sobel(context, commandQueue, src, width, height, dst, kernelSize, direction, normalize, scale, events, event);
}

void sobel(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, size_t width, size_t height, ::cl::Image &dst, int kernelSize, Direction direction, bool normalize, float scale,
    std::vector<::cl::Event> *events, ::cl::Event *event)
{
    SeparableKernel convolveKernel=buildSobelSeparableKernel(context, kernelSize, normalize);
    cl_image_format format;

    src.getImageInfo(CL_IMAGE_FORMAT, &format);

    ::cl::Image2D scratch(context, CL_MEM_READ_WRITE, ::cl::ImageFormat(format.image_channel_order, format.image_channel_data_type), width, height);

    sobel(context, commandQueue, src, width, height, dst, scratch, convolveKernel, direction, normalize, scale, events, event);

    if(event!=nullptr)
        event->wait();
}

void sobel(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Image &src, size_t width, size_t height, ::cl::Image &dst, ::cl::Image &scratch, SeparableKernel &kernelBuffer,
    Direction direction, bool normalize, float scale, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    if(direction==Direction::Y)
        separableConvolve(context, commandQueue, src, width, height, dst, scratch, kernelBuffer.kernel1, kernelBuffer.size, kernelBuffer.kernel2, kernelBuffer.size, scale, events, event);
    else
        separableConvolve(context, commandQueue, src, width, height, dst, scratch, kernelBuffer.kernel2, kernelBuffer.size, kernelBuffer.kernel1, kernelBuffer.size, scale, events, event);
}

}}//namespace imglib::cl