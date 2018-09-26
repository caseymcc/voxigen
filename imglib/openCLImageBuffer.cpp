#include "imglib/OpenCLImageBuffer.h"

#include <cassert>

namespace imglib
{

OpenCLImageBuffer::OpenCLImageBuffer(::cl::Context &openGLContext, Format format, Depth depth, size_t width, size_t height):
m_openGLContext(openGLContext), m_format(format), m_width(width), m_stride(width), m_height(height)
{
    allocData(format, depth, width, height);
}

OpenCLImageBuffer::OpenCLImageBuffer(::cl::Context &openGLContext, ::cl::CommandQueue &commandQueue, Format format, Depth depth, size_t width, size_t height, uint8_t *data, size_t dataSize):
m_openGLContext(openGLContext), m_format(format), m_depth(depth), m_width(width), m_stride(width), m_height(height)
{
    cl::Event event;

    write(commandQueue, data, format, depth, width, height, nullptr, &event);
    event.wait();
}

OpenCLImageBuffer::~OpenCLImageBuffer()
{
    freeData();
}

void OpenCLImageBuffer::allocData(Format format, Depth depth, size_t width, size_t height)
{
    m_format=format;
    m_depth=depth;
    m_width=width;
    m_height=height;

    m_image=::cl::Buffer(m_openGLContext, CL_MEM_READ_WRITE, sizeOfImage(m_format, m_depth));
}

void OpenCLImageBuffer::freeData()
{
    m_image=::cl::Buffer();
}

bool OpenCLImageBuffer::write(::cl::CommandQueue &commandQueue, unsigned char *buffer, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    if(width<=0)
        return false;
    if(height<=0)
        return false;

    if((m_format!=format)||(m_depth!=depth)||(m_width!=width)||(m_height!=height))
    {
        allocData(format, depth, width, height);
    }

    cl_int status;

    status=commandQueue.enqueueWriteBuffer(m_image, CL_FALSE, 0, sizeOfImage(m_format, m_depth), buffer, events, event);

    if(status!=CL_SUCCESS)
    {
        assert(false);
        return false;
    }
    return true;
}

bool OpenCLImageBuffer::read(::cl::CommandQueue &commandQueue, unsigned char *buffer, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events, ::cl::Event *event)
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

    status=commandQueue.enqueueReadBuffer(m_image, CL_FALSE, 0, sizeOfImage(m_format, m_depth), buffer, events, event);

    if(status!=CL_SUCCESS)
    {
        assert(false);
        return false;
    }
    return true;
}

bool OpenCLImageBuffer::copy(::cl::CommandQueue &commandQueue, OpenCLImageBuffer &srcImage, std::vector<::cl::Event> *waitEvents, ::cl::Event *event)
{
    return copy(commandQueue, srcImage.m_image, srcImage.m_format, srcImage.m_depth, srcImage.m_width, srcImage.m_height, waitEvents, event);
}

bool OpenCLImageBuffer::copy(::cl::CommandQueue &commandQueue, ::cl::Buffer &srcImage, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events, ::cl::Event *event)
{
    cl::Event localEvent;
    std::vector<cl::Event> localWaitEvents;

    if((m_width!=width) || (m_height!=height))
        allocData(format, depth, width, height);

    cl_int status;

    status=commandQueue.enqueueCopyBuffer(srcImage, m_image, 0, 0, sizeOfImage(m_format, m_depth), events, event);
    
    if(status!=CL_SUCCESS)
    {
        assert(false);
        return false;
    }
    return true;
}

}//namespace imglib
