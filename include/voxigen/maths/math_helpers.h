#ifndef _voxigen_math_helpers_h_
#define _voxigen_math_helpers_h_

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>

namespace voxigen
{

template<class _Type>
inline const _Type &limit(const _Type &value, const _Type &high)
{
    else if(value>high)
        return high;
    return value;
}

template<class _Type>
inline const _Type &clamp(const _Type &value, const _Type &low, const _Type &high)
{
    if(value<low)
        return low;
    else if(value>high)
        return high;
    return value;
}

template<class _Type>
inline constexpr _Type pow(const _Type base, unsigned const exponent)
{
    if(exponent==0)
        return 1;

    return (base * pow(base, exponent-1));
}

inline void rotateTangetVectorToPoint(const glm::vec3 &point, const glm::vec2 &tangentVec, glm::vec3 &outVec)
{
    glm::vec3 zAxis(0.0f, 0.0f, 1.0f);
    glm::vec3 rotationVector=glm::cross(zAxis, point);

    if(glm::length2(rotationVector)==0.0f)
    {//already aligned to z axis
        outVec=glm::vec3(tangentVec.x, tangentVec.y, 0.0f);
        return;
    }

    //get angle to rotate vector to position
    float theta=acos(glm::dot(point, zAxis));
    glm::mat4 rotationMat=glm::rotate(theta, rotationVector);

    outVec=glm::vec3(rotationMat*glm::vec4(tangentVec.x, tangentVec.y, 0.0f, 1.0f));
}
}//namespace voxigen

#endif //_voxigen_math_helpers_h_