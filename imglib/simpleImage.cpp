#include "imglib/simpleImage.h"

#include <algorithm>
#include <cstring>

namespace imglib
{

SimpleImage::SimpleImage(const SimpleImage &image)
{
    freeData();
    allocData(image.format, image.depth, image.width, image.height);

    memcpy(data, image.data, std::min(dataSize, image.dataSize));
}

SimpleImage::SimpleImage(SimpleImage &&image)
{
    format=image.format;
    depth=image.depth;

    width=image.width;
    height=image.height;
    stride=image.stride;

    owned=image.owned;
    data=image.data;
    dataSize=image.dataSize;

    image.owned=false;
}

SimpleImage::SimpleImage(Format format, Depth depth, size_t width, size_t height):
    format(format), width(width), stride(width), height(height), owned(false)
{
    allocData(format, depth, width, height);
}

SimpleImage::~SimpleImage()
{
    freeData();
}

SimpleImage &SimpleImage::operator=(const SimpleImage &image)
{
    freeData();
    allocData(image.format, image.depth, image.width, image.height);

    memcpy(data, image.data, std::min(dataSize, image.dataSize));
    return *this;
}

void SimpleImage::allocData(Format format, Depth depth, size_t width, size_t height)
{
    this->format=format;
    this->depth=depth;
    this->width=width;
    this->stride=width;
    this->height=height;

    dataSize=sizeOfPixel(format, depth)*width*height;
    data=(uint8_t *)malloc(dataSize);
    owned=true;
}

void SimpleImage::freeData()
{
    if((owned)&&(data!=nullptr))
    {
        free(data);
        data=nullptr;
        owned=false;
    }
}

}//namespace imglib
