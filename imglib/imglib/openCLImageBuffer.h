#ifndef _imglib_openCLImageBuffer_h_
#define _imglib_openCLImageBuffer_h_

#include "imglib/image.h"
#include "imglib_export.h"

#include "CL/cl.hpp"

#include <vector>

namespace imglib
{

/// A gpu image class provided by the library for quick use.
class IMGLIB_EXPORT OpenCLImageBuffer
{
public:
    OpenCLImageBuffer(::cl::Context &openGLContext):m_width(0), m_stride(0), m_height(0){};
    OpenCLImageBuffer(::cl::Context &openGLContext, Format format, Depth depth, size_t width, size_t height);
    OpenCLImageBuffer(::cl::Context &openGLContext, ::cl::CommandQueue &commandQueue, Format format, Depth depth, size_t width, size_t height, uint8_t *data, size_t dataSize);
    template<typename Image>
    OpenCLImageBuffer(::cl::Context &openGLContext, ::cl::CommandQueue &commandQueue, Image &image, bool copyImage=false):
    m_openGLContext(openGLContext)
    {
        allocData(format(image), depth(image), width(image), height(image));

        if(copyImage)
        {
            ::cl::Event event;

            copy(commandQueue, image, nullptr, &event);
            event.wait();
        }
    }
    ~OpenCLImageBuffer();

    ::cl::Buffer &clBuffer() { return m_image; }

    void allocData(Format format, Depth depth, size_t width, size_t height);
    void freeData();

    bool write(::cl::CommandQueue &commandQueue, unsigned char *buffer, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
    bool read(::cl::CommandQueue &commandQueue, unsigned char *buffer, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);
    
    template<typename Image>
    bool copy(::cl::CommandQueue &commandQueue, Image &image, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr)
    {
        if(location(image)==Location::System)
            write(commandQueue, data(image), format(image), depth(image), width(image), height(image), events, event);
        else if(location(image)==Location::OpenCL)
        {
            ::cl::Buffer clImage((cl_mem)nativeId(image));

            copy(commandQueue, clImage, format(image), depth(image), width(image), height(image), events, event);
        }
        else
            return false;
        return true;
    }
    bool copy(::cl::CommandQueue &commandQueue, OpenCLImageBuffer &srcImage, std::vector<::cl::Event> *waitEvents=nullptr, ::cl::Event *event=nullptr);
    bool copy(::cl::CommandQueue &commandQueue, ::cl::Buffer &srcImage, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);

    friend Format format(const OpenCLImageBuffer &image);
    friend Depth depth(const OpenCLImageBuffer &image);
    friend Location location(const OpenCLImageBuffer &image);
    friend size_t width(const OpenCLImageBuffer &image);
    friend size_t height(const OpenCLImageBuffer &image);
    friend size_t stride(const OpenCLImageBuffer &image);
    friend size_t nativeId(const OpenCLImageBuffer &image);

private:
    Format m_format;
    Depth m_depth;

    size_t m_width;
    size_t m_height;
    size_t m_stride;

    ::cl::Context m_openGLContext;
    ::cl::Buffer m_image;
};

}//namespace imglib


namespace imglib
{

inline Format format(const OpenCLImageBuffer &image) { return image.m_format; }
inline Depth depth(const OpenCLImageBuffer &image) { return image.m_depth; }
inline Location location(const OpenCLImageBuffer &image) { return Location::OpenCL; } //allways gpu for this guy

inline size_t width(const OpenCLImageBuffer &image) { return image.m_width; }
inline size_t height(const OpenCLImageBuffer &image) { return image.m_height; }
inline size_t stride(const OpenCLImageBuffer &image) { return image.m_stride; }

inline size_t nativeId(const OpenCLImageBuffer &image) { return (size_t)image.m_image(); }
//not system memory, so not directly accessible
inline uint8_t *data(const OpenCLImageBuffer &image) { return nullptr; }
inline size_t dataSize(const OpenCLImageBuffer &image) { return 0; }

inline bool resize(OpenCLImageBuffer &image, Format format, Depth depth, size_t width, size_t height)
{
    image.allocData(format, depth, width, height);
    return true;
}

}//namespace imglib

#endif //_imglib_openCLImageBuffer_h_