#ifndef _imglib_imageIo_h_
#define _imglib_imageIo_h_

#include "imglib/png.h"
#include "imglib/ppm.h"

#include <generic/fileIO.h>

namespace imglib
{

template<typename _ImageType>
bool load(_ImageType &image, const std::string &location)
{
    std::string ext=generic::getExtension(location);

    if(ext==".png")
        return loadPng(image, location.c_str());
    else if(ext==".ppm")
        return loadPpm(image, location.c_str());

    assert(false); //not supported yet
    return false;
}

//Template based load, allows functions to use IO provided by caller
template<typename _FileIO, typename _ImageType>
bool load(_ImageType &image, const std::string &location, void *userData=nullptr)
{
    std::string ext=generic::getExtension(location);

    _FileIO::Type *fileType=generic::open<_FileIO>(location, "rb", userData);

    size_t bufferSize=generic::size<_FileIO>(fileType);
    uint8_t *buffer=(uint8_t *)malloc(sizeof(uint8_t)*bufferSize);

    generic::read<_FileIO>(buffer, sizeof(uint8_t), bufferSize, fileType);

    if(ext==".png")
        return loadPngBuffer(image, buffer, bufferSize);
 
    assert(false); //not supported yet
    return false;
}

template<typename _ImageType>
bool loadBuffer(_ImageType &image, const uint8_t *buffer, size_t size, std::string &format)
{
    if(format=="png")
        return loadPngBuffer(image, buffer, size);

    assert(false); //not supported yet
    return false;
}

template<typename _ImageType>
bool save(_ImageType &image, const std::string &location)
{
    std::string ext=generic::getExtension(location);

    if(ext==".png")
        return savePng(image, location.c_str());
    else if(ext==".ppm")
        return savePpm(image, location.c_str());

    assert(false); //not supported yet
    return false;
}

}//namespace imglib

#endif //_imglib_imageIo_h_