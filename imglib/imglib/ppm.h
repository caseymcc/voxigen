#ifndef _imglib_ppm_h_
#define _imglib_ppm_h

#include "imglib_export.h"
#include "imglib/image.h"

namespace imglib
{

IMGLIB_EXPORT bool loadPpm(ImageWrapper image, const char *filename);
IMGLIB_EXPORT bool savePpm(ImageWrapper image, const char *filename);

}//namespace imglib

#endif //_imglib_ppm_h_