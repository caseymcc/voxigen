#ifndef _imglib_draw_h_
#define _imglib_draw_h_

#include <cassert>

#include "imglib_export.h"
#include "imglib/image.h"

namespace imglib
{

set

template<typename _Image, typename _Point, typename _Color>
void drawPoint(const _Point &point, const _Color &color, _Image &image)
{
//    size_t height=height(image);
//    size_t width=width(image);
    uint8_t *imgData=data(image);
    Format imgFormat=format(image);

    size_t pixelSize=sizeOfPixel(imgFormat, depth(image));
    if(sizeof(color)==pixelSize)
        ((_Color *)imgData)=color;
    else
        assert(false);
}

}
#endif //_imglib_draw_h_