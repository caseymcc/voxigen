#ifndef _imglib_simpleImage_h_
#define _imglib_simpleImage_h_

#include "imglib/image.h"
#include "imglib_export.h"

namespace imglib
{

/// A simple image class provided by the library for quick use. The library can accept many different kinds of images
/// as long as certain functions are declared, see image.h
struct IMGLIB_EXPORT SimpleImage
{
    SimpleImage():width(0), stride(0), height(0), owned(false), data(nullptr), dataSize(0) {};
    SimpleImage(const SimpleImage &image);
    SimpleImage(SimpleImage &&image);
    SimpleImage(Format format, Depth depth, size_t width, size_t height);
    SimpleImage(Format format, Depth depth, size_t width, size_t height, uint8_t *data, size_t dataSize):
        format(format), depth(depth), width(width), stride(width), height(height), owned(false), data(data), dataSize(dataSize)
    {}
    SimpleImage(Format format, Depth depth, size_t width, size_t stride, size_t height, uint8_t *data, size_t dataSize):
        format(format), depth(depth), width(width), stride(stride), height(height), owned(false), data(data), dataSize(dataSize)
    {}
    ~SimpleImage();

    SimpleImage &operator=(const SimpleImage &image);
    

    void allocData(Format format, Depth depth, size_t width, size_t height);
    void freeData();

    Format format;
    Depth depth;

    size_t width;
    size_t height;
    size_t stride;

    bool owned;
    uint8_t *data;
    size_t dataSize;
};

}//namespace imglib


//This is the functionality that you need to define for any custom image class you have to operate
//in this library. Make sure to define it in the imglib namespace as that is where the functions
//in this library will expect them. Make sure you declare (not just define) the functions in the 
//header, as there is a high likelyhood that the compilier will inline all these removing any function 
//call overhead.
namespace imglib
{

template<>
inline Format format<SimpleImage>(const SimpleImage &image) { return image.format; }
template<>
inline Depth depth<SimpleImage>(const SimpleImage &image) { return image.depth; }
template<>
inline Location location<SimpleImage>(const SimpleImage &image) { return Location::System; } //allways system for this guy

template<>
inline size_t width<SimpleImage>(const SimpleImage &image) { return image.width; }
template<>
inline size_t height<SimpleImage>(const SimpleImage &image) { return image.height; }
template<>
inline size_t stride<SimpleImage>(const SimpleImage &image) { return image.stride; }

template<>
inline size_t nativeId<SimpleImage>(const SimpleImage &image) { return (size_t)image.data; }
template<>
inline uint8_t *data<SimpleImage>(SimpleImage &image) { return image.data; }
template<>
inline size_t dataSize<SimpleImage>(const SimpleImage &image) { return image.dataSize; }

template<>
inline bool resize<SimpleImage>(SimpleImage &image, Format format, Depth depth, size_t width, size_t height)
{
    image.freeData();

    image.allocData(format, depth, width, height);
    return true;
}

}//namespace imglib

#endif //_imglib_simpleImage_h_