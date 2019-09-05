#ifndef _imglib_location_h_
#define _imglib_location_h_

#include <stdint.h>
#include <memory>

namespace imglib
{

enum class Location
{
    System,
    Cuda,
    OpenCL,
    OpenGL,
    Vulkan
};

}//namespace imglib

#endif //_imglib_location_h_