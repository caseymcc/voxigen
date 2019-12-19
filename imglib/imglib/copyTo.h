#ifndef _imglib_copyTo_h_
#define _imglib_copyTo_h_

#include <memory>

#include "imglib_export.h"


namespace imglib
{

template<typename _DstImage, typename _SrcImage>
bool copyTo(_DstImage &dstImage, size_t dstX, size_t dstY, _SrcImage &srcImage)
{
    if(location(srcImage)!=location(dstImage))
        return false;//not implemented yet

    if(format(srcImage)!=format(dstImage))
        return false;//not implemented yet

    if(depth(srcImage)!=depth(dstImage))
        return false;//not implemented yet

    //raw copy
    size_t copyWidth=width(srcImage);
    size_t copyHeight=height(srcImage);
    
    if(copyWidth+dstX>width(dstImage))
        copyWidth=width(dstImage)-dstX;
    copyWidth*=sizeOfPixel(format(srcImage), depth(srcImage));

    if(copyHeight+dstY>height(dstImage))
        copyHeight=height(dstImage)-dstY;

    size_t srcStride=sizeOfPixel(format(srcImage), depth(srcImage))*stride(srcImage);
    uint8_t *srcData=data(srcImage);
        
    size_t dstStride=sizeOfPixel(format(dstImage), depth(dstImage))*stride(dstImage);
    uint8_t *dstData=data(dstImage)+(dstStride*dstY)+(sizeOfPixel(format(dstImage), depth(dstImage))*dstX);
    
    for(size_t y=0; y<copyHeight; ++y)
    {
        memcpy(dstData, srcData, copyWidth);

        srcData+=srcStride;
        dstData+=dstStride;
    }

    return true;
}

}//namespace imglib

#endif //_imglib_copyTo_h_