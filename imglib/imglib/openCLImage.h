#ifndef _imglib_openCLImageImage_h_
#define _imglib_openCLImageImage_h_

#include "imglib/image.h"
#include "imglib_export.h"

#include "CL/cl.hpp"

#include <vector>

namespace imglib
{

/// A gpu image class provided by the library for quick use.
class IMGLIB_EXPORT OpenCLImage
{
public:
    OpenCLImage(::cl::Context &openGLContext):m_width(0), m_stride(0), m_height(0){};
    OpenCLImage(::cl::Context &openGLContext, Format format, Depth depth, size_t width, size_t height);
    OpenCLImage(::cl::Context &openGLContext, ::cl::CommandQueue &commandQueue, Format format, Depth depth, size_t width, size_t height, uint8_t *data, size_t dataSize);
    template<typename Image>
    OpenCLImage(::cl::Context &openGLContext, ::cl::CommandQueue &commandQueue, Image &image, bool copyImage=false):
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
    ~OpenCLImage();

    ::cl::Image2D &clImage() { return m_image; }

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
            ::cl::Image2D clImage((cl_mem)nativeId(image));

            copy(commandQueue, clImage, format(image), depth(image), width(image), height(image), events, event);
        }
        else
            return false;
        return true;
    }
    bool copy(::cl::CommandQueue &commandQueue, OpenCLImage &srcImage, std::vector<::cl::Event> *waitEvents=nullptr, ::cl::Event *event=nullptr);
    bool copy(::cl::CommandQueue &commandQueue, ::cl::Image2D &srcImage, Format format, Depth depth, unsigned int width, unsigned int height, std::vector<::cl::Event> *events=nullptr, ::cl::Event *event=nullptr);

    friend Format format(const OpenCLImage &image);
    friend Depth depth(const OpenCLImage &image);
    friend Location location(const OpenCLImage &image);
    friend size_t width(const OpenCLImage &image);
    friend size_t height(const OpenCLImage &image);
    friend size_t stride(const OpenCLImage &image);
    friend size_t nativeId(const OpenCLImage &image);

private:
    Format m_format;
    Depth m_depth;

    size_t m_width;
    size_t m_height;
    size_t m_stride;

    ::cl::Context m_openGLContext;
    ::cl::Image2D m_image;
};

}//namespace imglib


namespace imglib
{

inline Format format(const OpenCLImage &image) { return image.m_format; }
inline Depth depth(const OpenCLImage &image) { return image.m_depth; }
inline Location location(const OpenCLImage &image) { return Location::OpenCL; } //allways gpu for this guy

inline size_t width(const OpenCLImage &image) { return image.m_width; }
inline size_t height(const OpenCLImage &image) { return image.m_height; }
inline size_t stride(const OpenCLImage &image) { return image.m_stride; }

inline size_t nativeId(const OpenCLImage &image) { return (size_t)image.m_image(); }
//not system memory, so not directly accessible
inline uint8_t *data(const OpenCLImage &image) { return nullptr; }
inline size_t dataSize(const OpenCLImage &image) { return 0; }

inline bool resize(OpenCLImage &image, Format format, Depth depth, size_t width, size_t height)
{
    image.allocData(format, depth, width, height);
    return true;
}

}//namespace imglib

#endif //_imglib_openCLImageImage_h_