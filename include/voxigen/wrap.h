#ifndef _voxigen_wrap_h_
#define _voxigen_wrap_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

namespace details
{
template<typename _Type> constexpr size_t dimensions() { return 1; }

template<> constexpr size_t dimensions<glm::ivec2>() { return 2; }
template<> constexpr size_t dimensions<glm::ivec3>() { return 3; }
template<> constexpr size_t dimensions<glm::ivec4>() { return 4; }
template<> constexpr size_t dimensions<glm::vec2>() { return 2; }
template<> constexpr size_t dimensions<glm::vec3>() { return 3; }
template<> constexpr size_t dimensions<glm::vec4>() { return 4; }

template<typename _Type, size_t dims>
struct WrapHelper
{
    static _Type _(_Type value, _Type lowerBound, _Type upperBound);
};

template<typename _Type, size_t dims>
_Type WrapHelper<_Type, dims>::_(_Type value, _Type lowerBound, _Type upperBound)
{
    for(size_t i=0; i<dims; ++i)
    {
        if(value[i]<lowerBound[i])
            value[i]=upperBound[i]-(lowerBound[i]-value[i]);
        else if(value[i]>upperBound[i])
            value[i]=lowerBound[i]+(value[i]-upperBound[i]);
    }
    return value;
}

template<typename _Type>
struct WrapHelper<_Type, 1>
{
    static _Type _(_Type value, _Type lowerBound, _Type upperBound)
    {
        if(value<lowerBound)
            value=upperBound-(lowerBound-value);
        else if(value>upperBound)
            value=lowerBound+(value-upperBound);

        return value;
    }
};

}//namespace details

template<typename _Type, size_t dims=details::dimensions<_Type>()>
_Type wrap(_Type value, _Type lowerBound, _Type upperBound)
{
    return details::WrapHelper<_Type, dims>::_(value, lowerBound, upperBound);
}

}//namespace voxigen

#endif //_voxigen_wrap_h_