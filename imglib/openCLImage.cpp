#include "imglib/OpenCLImage.h"

#include <cassert>

namespace imglib
{

::cl::ImageFormat openCLFormat(Format format, Depth depth)
{
    cl_image_format clFormat;

    switch(format)
    {
    case Format::Binary:
    case Format::GreyScale:
        clFormat.image_channel_order=CL_R;
        break;
    case Format::RA:
        clFormat.image_channel_order=CL_RA;
        break;
    case Format::RGB:
    case Format::YUV:
    case Format::RGBA:
        clFormat.image_channel_order=CL_RGBA;
        break;
    case Format::BGRA:
        clFormat.image_channel_order=CL_BGRA;
        break;
    }

    switch(depth)
    {
    case Depth::Bit1:
        assert(false);
        clFormat.image_channel_data_type=CL_UNSIGNED_INT8;
        break;
    case Depth::Bit8:
        clFormat.image_channel_data_type=CL_UNSIGNED_INT8;
        break;
    case Depth::Bit10:
    case Depth::Bit12:
    case Depth::Bit14:
        assert(false);
        clFormat.image_channel_data_type=CL_UNSIGNED_INT16;
        break;
    case Depth::Bit16:
        clFormat.image_channel_data_type=CL_UNSIGNED_INT16;
        break;
    case Depth::Float:
        clFormat.image_channel_data_type=CL_FLOAT;
        break;
    case Depth::Double:
        assert(false);
        clFormat.image_channel_data_type=CL_FLOAT;
        break;
    }

    return ::cl::ImageFormat(clFormat.image_channel_order, clFormat.image_channel_data_type);
}

OpenCLImage::OpenCLImage(::cl::Context &openGLContext, Format format, Depth depth, size_t width, size_t height):
m_openGLContext(openGLContext), m_format(format), m_width(width), m_stride(width), m_height(height)
{
    allocData(format, depth, width, height);
}

OpenCLImage::OpenCLImage(::cl::Context &openGLContext, ::cl::CommandQueue &commandQueue, Format format, Depth depth, size_t width, size_t height, uint8_t *data, size_t dataSize):
m_openGLContext(openGLContext), m_format(format), m_depth(depth), m_width(width), m_stride(width), m_height(height)
{
    cl::Event event;

    write(commandQueue, data, format, depth, width, height, nullptr, &event);
    event.wait();
}

OpenCLImage::~OpenCLImage()
{
    freeData();
}

void OpenCLImage::allocData(Format format, Depth depth, size_t width, size_t height)
{
    if(format==Format::RGB)
        m_format=Format::RGBA;
    else
        m_format=format;

    m_depth=depth;
    m_width=width;
    m_height=height;

    ::cl::ImageFormat imageFormat=openCLFormat(format, depth);

    m_image=::cl::Image2D(m_openGLContext, CL_MEM_READ_WRITE, imageFormat, width, height);
}

void OpenCLImage::freeData()
{
    m_image=::cl::Image2D();
}

bool OpenCLImage::write(::cl::CommandQueue &commandQueue, unsigned char *buffer, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    if(width<=0)
        return false;
    if(height<=0)
        return false;

    if(format==Format::RGB) //RGB is only allowed if using packed format, which is currently not implemented
        return false;

    if((m_format!=format)||(m_depth!=depth)||(m_width!=width)||(m_height!=height))
    {
        allocData(format, depth, width, height);
    }

    cl_int status;
    cl::size_t<3> origin;
    cl::size_t<3> region;

    origin[0]=0;
    origin[1]=0;
    origin[2]=0;

    region[0]=m_width;
    region[1]=m_height;
    region[2]=1;

    status=commandQueue.enqueueWriteImage(m_image, CL_FALSE, origin, region, 0, 0, buffer, events, event);

    if(status!=CL_SUCCESS)
    {
        assert(false);
        return false;
    }
    return true;
}

bool OpenCLImage::read(::cl::CommandQueue &commandQueue, unsigned char *buffer, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    if(width<=0)
        return false;
    if(height<=0)
        return false;

    if(m_format!=format)
        return false;
    if(m_depth!=depth)
        return false;
    if((m_width!=width)||(m_height!=height))
        return false;

    cl_int status;
    cl::size_t<3> origin;
    cl::size_t<3> region;

    origin[0]=0;
    origin[1]=0;
    origin[2]=0;

    region[0]=m_width;
    region[1]=m_height;
    region[2]=1;

    status=commandQueue.enqueueReadImage(m_image, CL_FALSE, origin, region, 0, 0, buffer, events, event);

    if(status!=CL_SUCCESS)
    {
        assert(false);
        return false;
    }
    return true;
}

bool OpenCLImage::copy(::cl::CommandQueue &commandQueue, OpenCLImage &srcImage, std::vector<::cl::Event> *waitEvents, ::cl::Event *event)
{
    return copy(commandQueue, srcImage.m_image, srcImage.m_format, srcImage.m_depth, srcImage.m_width, srcImage.m_height, waitEvents, event);
}

bool OpenCLImage::copy(::cl::CommandQueue &commandQueue, ::cl::Image2D &srcImage, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    cl::Event localEvent;

    if((m_width!=width) || (m_height!=height))
        allocData(format, depth, width, height);

    cl_int status;
    cl::size_t<3> origin;
    cl::size_t<3> region;

    origin[0]=0;
    origin[1]=0;
    origin[2]=0;

    region[0]=m_width;
    region[1]=m_height;
    region[2]=1;

    status=commandQueue.enqueueCopyImage(srcImage, m_image, origin, origin, region, events, event);

    if(status!=CL_SUCCESS)
    {
        assert(false);
        return false;
    }
    return true;
}

}//namespace imglib
