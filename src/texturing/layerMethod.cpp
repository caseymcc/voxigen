#include "voxigen/texturing/layerMethod.h"

#include <unordered_map>

namespace voxigen
{

typedef std::unordered_map<std::string, LayerMethod> LayerMethodMap;

LayerMethodMap makeLayerMethodMap()
{
    LayerMethodMap map;

    map.emplace("unknown", LayerMethod::unknown);
    map.emplace("connect", LayerMethod::connect);
    map.emplace("horizontal", LayerMethod::horizontal);
    map.emplace("vertical", LayerMethod::vertical);
    map.emplace("grass", LayerMethod::grass);
    map.emplace("repeat", LayerMethod::repeat);
    map.emplace("random", LayerMethod::random);
    map.emplace("flora", LayerMethod::flora);

    return map;
}
LayerMethodMap l_textureMethodMap=makeLayerMethodMap();

LayerMethod getLayerMethod(const std::string &method)
{
    auto iter=l_textureMethodMap.find(method);

    if(iter != l_textureMethodMap.end())
        return iter->second;
    return LayerMethod::unknown;
}

std::string getLayerMethodName(LayerMethod method)
{
    for(auto &iter:l_textureMethodMap)
    {
        if(iter.second==method)
            return iter.first;
    }
    return "unknown";
}

}//namespace voxigen
