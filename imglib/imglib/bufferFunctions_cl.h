#ifndef _cvlib_bufferFunctions_cl_h_
#define _cvlib_bufferFunctions_cl_h_

#include "cvlib/cvlibDefine.h"
#include "CL/cl.hpp"

#include <vector>

namespace cvlib{namespace cl
{

///
/// Copies buffer by rotating pos to 0
///
cvlib_EXPORT void rotateCopy(::cl::Context &context, ::cl::CommandQueue &commandQueue, ::cl::Buffer &src, ::cl::Buffer &dst, int size, int pos, std::vector<::cl::Event> *events, ::cl::Event *event);

}}//namespace cvlib::cl

#endif //_cvlib_bufferFunctions_cl_h_