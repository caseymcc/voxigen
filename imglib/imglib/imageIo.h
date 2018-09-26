#ifndef _imglib_imageIo_h_
#define _imglib_imageIo_h_

#include "imglib/png.h"
#include "imglib/ppm.h"

#ifdef IMGLIB_USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#endif//IMGLIB_USE_BOOST_FILESYSTEM

namespace imglib
{

#ifdef IMGLIB_USE_BOOST_FILESYSTEM
namespace fs=boost::filesystem;
#else//IMGLIB_USE_BOOST_FILESYSTEM
namespace fs=std::filesystem;
#endif//IMGLIB_USE_BOOST_FILESYSTEM

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