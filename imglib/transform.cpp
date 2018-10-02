#include "imglib/transform.h"

#include <cassert>

namespace imglib
{

void reduceSimple_1(uint8_t *srcBuffer, size_t srcSize, Depth srcDepth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight)
{
}

void reduceSimple_4(uint8_t *srcBuffer, size_t srcSize, Depth srcDepth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight)
{
    if((srcWidth/2) != dstWidth)
    {
        assert(false);//not suported
        return;
    }

    if((srcHeight/2)!=dstHeight)
    {
        assert(false);//not suported
        return;
    }

    for(size_t dstY=0; dstY<dstWidth; dstY++)
    {
        for(size_t dstX=0; dstX<dstWidth; dstX++)
        {

        }
    }
}

void reduceSimple(uint8_t *srcBuffer, size_t srcSize, Format srcFormat, Depth srcDepth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight)
{
    if(srcFormat!=dstFormat)
    {
        assert(false);//not suported
        return;
    }

    if(srcDepth!=dstDepth)
    {
        assert(false);//not suported
        return;
    }

    switch(srcFormat)
    {
    case Format::Binary:
    case Format::GreyScale:
        break;
    case Format::RA:
        break;
    case Format::RGB:
        break;
    case Format::RGBA:
    case Format::BGRA:
//        reduceSimple_4(srcBuffer, srcSize, srcFormat, srcDepth, srcWidth, srcHeight, dstBuffer, dstSize, dstFormat, dstDepth, dstWidth, dstHeight);
        break;
    default:
        assert(false);//not implemented
        break;
    }
}

void expandSimple(uint8_t *srcBuffer, size_t srcSize, Format format, Depth depth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight)
{
    //not implemented
    assert(false);
}

void reduce(uint8_t *srcBuffer, size_t srcSize, Format format, Depth depth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight)
{
    //not implemented
    assert(false);
}

void expand(uint8_t *srcBuffer, size_t srcSize, Format format, Depth depth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight)
{
    //not implemented
    assert(false);
}

}//namespace imglib
