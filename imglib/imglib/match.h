#ifndef _imglib_match_h_
#define _imglib_match_h_

#include "imglib_export.h"
#include "imglib/image.h"

#include <algorithm>
#include <cassert>

namespace imglib
{

template<typename _Image, Format _PixelType, Depth _Depth>
bool matchByTypeDepth(const _Image &src1, const _Image &src2, _Image &dest)
{
    size_t height=std::min(height(src1), height(src2));
    size_t width=std::min(width(src1), width(src2));

    uint8_t *src1Data=data(src1);
    uint8_t *src2Data=data(src2);
    uint8_t *dstData=data(dest);
    
    size_t src1Stride=stride(src1)-(width*traits::sizeOf<_PixelType, _Depth>());
    size_t src2Stride=stride(src2)-(width*traits::sizeOf<_PixelType, _Depth>());
    size_t dstStride=stride(dest)-(width*traits::sizeOf<Format::Binary, Depth::Bit8>());
    
    for(size_t y=0; y<height; ++y)
    {
        for(size_t x=0; x<width; ++x)
        {
            (*dstData)=utils::comparePixel<_PixelType, _Depth>(src1Data, src2Data)?1:0;

            src1Data+=traits::sizeOf<_PixelType, _Depth>();
            src2Data+=traits::sizeOf<_PixelType, _Depth>();
            dstData+=traits::sizeOf<Format::Binary, Depth::Bit8>();
        }
        src1Data+=src1Stride;
        src2Data+=src2Stride;
        dstData+=dstStride;
    }

    return true;
}

template<typename _Image, Format _PixelType>
inline bool matchByType(_Image &src1, _Image &src2, _Image &dest)
{
    if(depth(src1)!=depth(src2))
        return false;

    switch(depth(src1))
    {
    case Depth::Bit1:
        return matchByTypeDepth<_PixelType, Depth::Bit1>(src1, src2, dest);
        break;
    case Depth::Bit8:
        return matchByTypeDepth<_PixelType, Depth::Bit8>(src1, src2, dest);
        break;
    case Depth::Bit10:
        return matchByTypeDepth<_PixelType, Depth::Bit10>(src1, src2, dest);
        break;
    case Depth::Bit12:
        return matchByTypeDepth<_PixelType, Depth::Bit12>(src1, src2, dest);
        break;
    case Depth::Bit14:
        return matchByTypeDepth<_PixelType, Depth::Bit14>(src1, src2, dest);
        break;
    case Depth::Bit16:
        return matchByTypeDepth<_PixelType, Depth::Bit16>(src1, src2, dest);
        break;
    }

    assert(false);//no compare function provided
    return false;
}

template<typename _Image>
inline bool match(_Image &src1, _Image &src2, _Image &dest)
{
    if(format(src1)!=format(src2))
        return false;

    switch(format(src1))
    {
    case Format::Binary:
        return matchByType<Format::Binary>(src1, src2, dest);
        break;
    case Format::GreyScale:
        return matchByType<Format::GreyScale>(src1, src2, dest);
        break;
    case Format::RA:
        return matchByType<Format::RA>(src1, src2, dest);
        break;
    case Format::RGB:
        return matchByType<Format::RGB>(src1, src2, dest);
        break;
    case Format::RGBA:
        return matchByType<Format::RGBA>(src1, src2, dest);
        break;
    }

    assert(false);//no compare function provided
    return false;
}

}//namespace imglib

#endif //_imglib_match_h_