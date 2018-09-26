#include "voxigen/blendMethod.h"

#include <unordered_map>

namespace voxigen
{

typedef std::unordered_map<std::string, BlendMethod> BlendMethodMap;

BlendMethodMap makeBlendMethodMap()
{
    BlendMethodMap map;

    map.emplace("unknown", BlendMethod::unknown);
    map.emplace("alpha", BlendMethod::alpha);
    map.emplace("add", BlendMethod::add);
    map.emplace("subtract", BlendMethod::subtract);
    map.emplace("multiply", BlendMethod::multiply);

    return map;
}
BlendMethodMap g_blendMethodMap=makeBlendMethodMap();

BlendMethod getBlendMethod(const std::string &method)
{
    auto iter=g_blendMethodMap.find(method);

    if(iter != g_blendMethodMap.end())
        return iter->second;
    return BlendMethod::unknown;
}

std::string getBlendMethodName(BlendMethod method)
{
    for(auto &iter:g_blendMethodMap)
    {
        if(iter.second==method)
            return iter.first;
    }
    return "unknown";
}

}//namespace voxigen
