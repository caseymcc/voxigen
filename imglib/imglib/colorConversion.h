#ifndef _imglib_colorConversion_h_
#define _imglib_colorConversion_h_

#include "imglib_export.h"
#include "imglib/image.h"

namespace imglib
{

template<typename _Image>
bool convert(_Image &srcImage, _Image &dstImage);

template<typename _Image>
void rgbToRGBA(_Image &srcImage, _Image &dstImage);

IMGLIB_EXPORT void rgbToRGBA(__uint8_t *srcBuffer, size_t srcSize, Depth srcDepth, size_t srcWidth, size_t srcHeight, 
    __uint8_t *dstBuffer, size_t dstSize, Depth dstDepth, size_t dstWidth, size_t dstHeight);

}//namespace imglib

//source
#include "colorConversion.inl"

#endif //_imglib_colorConversion_h_