#ifndef _voxigen_layerMethod_h_
#define _voxigen_layerMethod_h_

#include "voxigen/voxigen_export.h"

#include <string>

namespace voxigen
{

enum class LayerMethod
{
    unknown,
    connect,
    horizontal,
    vertical,
    grass,
    repeat,
    random,
    flora
};

VOXIGEN_EXPORT LayerMethod getLayerMethod(const std::string &method);
VOXIGEN_EXPORT std::string getLayerMethodName(LayerMethod method);

}//namespace voxigen

#endif //_voxigen_layerMethod_h_