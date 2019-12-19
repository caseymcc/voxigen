#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

inline float x(const glm::vec2 &point)
{
    return point.x;
}

inline float y(const glm::vec2 &point)
{
    return point.y;
}

inline float x(const glm::vec3 &point)
{
    return point.x;
}

inline float y(const glm::vec3 &point)
{
    return point.y;
}

inline float z(const glm::vec3 &point)
{
    return point.z;
}

inline float red(const glm::vec4 &point)
{
    return point.r*255;
}

inline float green(const glm::vec4 &point)
{
    return point.g*255;
}

inline float blue(const glm::vec4 &point)
{
    return point.b*255;
}

inline float alpha(const glm::vec4 &point)
{
    return point.a*255;
}

inline unsigned char red(const glm::tvec4<unsigned char> &point)
{
    return point.r;
}

inline unsigned char green(const glm::tvec4<unsigned char> &point)
{
    return point.g;
}

inline unsigned char blue(const glm::tvec4<unsigned char> &point)
{
    return point.b;
}

inline unsigned char alpha(const glm::tvec4<unsigned char> &point)
{
    return point.a;
}
