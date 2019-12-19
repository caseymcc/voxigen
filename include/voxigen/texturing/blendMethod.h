#ifndef _voxigen_blendMethod_h_
#define _voxigen_blendMethod_h_

#include "voxigen/voxigen_export.h"

#include <string>

namespace voxigen
{

enum class BlendMethod
{
    unknown,
    alpha,
    add,
    subtract,
    multiply
};

VOXIGEN_EXPORT BlendMethod getBlendMethod(const std::string &method);
VOXIGEN_EXPORT std::string getBlendMethodName(BlendMethod method);

}//namespace voxigen

#endif //_voxigen_blendMethod_h_