#ifndef _imglib_transform_h_
#define _imglib_transform_h_

#include "imglib_export.h"
#include "imglib/image.h"

namespace imglib
{

template<typename _Image>
void reduceSimple(_Image &srcImage, _Image &dstImage);

template<typename _Image>
void expandSimple(_Image &srcImage, _Image &dstImage);

template<typename _Image>
void reduce(_Image &srcImage, _Image &dstImage);

template<typename _Image>
void expand(_Image &srcImage, _Image &dstImage);

IMGLIB_EXPORT void reduceSimple(uint8_t *srcBuffer, size_t srcSize, Format format, Depth depth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight);
IMGLIB_EXPORT void expandSimple(uint8_t *srcBuffer, size_t srcSize, Format format, Depth depth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight);

IMGLIB_EXPORT void reduce(uint8_t *srcBuffer, size_t srcSize, Format format, Depth depth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight);
IMGLIB_EXPORT void expand(uint8_t *srcBuffer, size_t srcSize, Format format, Depth depth, size_t srcWidth, size_t srcHeight, uint8_t *dstBuffer, size_t dstSize, Format dstFormat, Depth dstDepth, size_t dstWidth, size_t dstHeight);

}//namespace imglib

//source
#include "transform.inl"

#endif //_imglib_transform_h_