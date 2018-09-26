#ifndef _imglib_png_h_
#define _imglib_png_h

#include "imglib_export.h"
#include "imglib/image.h"

namespace imglib
{

IMGLIB_EXPORT bool loadPng(ImageWrapper image, const char *filename);
IMGLIB_EXPORT bool savePng(ImageWrapper image, const char *filename);

}//namespace imglib

#endif //_imglib_png_h