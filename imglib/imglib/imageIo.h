#ifndef _imglib_imageIo_h_
#define _imglib_imageIo_h_

#include "imglib/png.h"
#include "imglib/ppm.h"

#if IMGLIB_USE_FILESYSTEM == 1
#include <filesystem>
#elif IMGLIB_USE_FILESYSTEM == 2
#include <experimental/filesystem>
#else
#include <boost/filesystem.hpp>
#endif

namespace imglib
{

#if IMGLIB_USE_FILESYSTEM == 1
namespace fs=std::filesystem;
#elif IMGLIB_USE_FILESYSTEM == 2
#ifdef _MSC_VER
namespace fs=std::experimental::filesystem::v1;
#else
namespace fs=std::experimental::filesystem;
#endif
#else
namespace fs=boost::filesystem;
#endif

template<typename _ImageType>
bool load(_ImageType &image, const std::string &location)
{
    fs::path path(location);

    if(!path.has_extension())
    {
        assert(false); //can only sort by extension at the moment
        return false;
    }

    std::string ext=path.extension().string();

    if(ext==".png")
        return loadPng(image, location.c_str());
    else if(ext==".ppm")
        return loadPpm(image, location.c_str());

    assert(false); //not supported yet
    return false;
}

template<typename _ImageType>
bool save(_ImageType &image, const std::string &location)
{
    fs::path path(location);

    if(!path.has_extension())
    {
        assert(false); //can only sort by extension at the moment
        return false;
    }

    std::string ext=path.extension().string();

    if(ext==".png")
        return savePng(image, location.c_str());
    else if(ext==".ppm")
        return savePpm(image, location.c_str());

    assert(false); //not supported yet
    return false;
}

}//namespace imglib

#endif //_imglib_imageIo_h_