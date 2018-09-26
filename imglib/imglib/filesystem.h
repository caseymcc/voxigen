#ifndef _imglib_image_h_
#define _imglib_image_h_

#ifdef IMGLIB_USE_PNG
#include "png.h"
#endif //IMGLIB_USE_PNG

namespace imglib
{

//used to hide boost::filesystem access
std::string getExt(char *filename);

template<typename _Image>
bool loadImage(_Image &image, char *filename)
{
    std::string ext=getExt(filename);

//try based on extention
#ifdef IMGLIB_USE_PNG
    if(ext=="png")
        return loadPng(image filename);
#endif //IMGLIB_USE_PNG
    if(ext=="ppm")
        return loadPpm(image filename);

//no one understood the extension, try all
#ifdef IMGLIB_USE_PNG
    if(loadPng(image filename))
        return true;
#endif //IMGLIB_USE_PNG
    if(loadPpm(image filename))
        return true;

    return false;
}

}//namespace imglib

#endif //_imglib_image_h_