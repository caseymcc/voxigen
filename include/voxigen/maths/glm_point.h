#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

inline const float &x(const glm::vec2 &point)
{
    return point.x;
}

inline const float &y(const glm::vec2 &point)
{
    return point.y;
}

inline float &x(glm::vec2 &point)
{
    return point.x;
}

inline float &y(glm::vec2 &point)
{
    return point.y;
}

inline const float &x(const glm::vec3 &point)
{
    return point.x;
}

inline const float &y(const glm::vec3 &point)
{
    return point.y;
}

inline const float &z(const glm::vec3 &point)
{
    return point.z;
}

inline float &x(glm::vec3 &point)
{
    return point.x;
}

inline float &y(glm::vec3 &point)
{
    return point.y;
}

inline float &z(glm::vec3 &point)
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

inline glm::vec2 normalize(const glm::vec2 &point)
{
    return glm::normalize(point);
}

inline glm::vec3 normalize(const glm::vec3 &point)
{
    return glm::normalize(point);
}

inline glm::vec4 normalize(const glm::vec4 &point)
{
    return glm::normalize(point);
}

inline float length(const glm::vec2 &point)
{
    return glm::length(point);
}

inline float length(const glm::vec3 &point)
{
    return glm::length(point);
}

inline float length(const glm::vec4 &point)
{
    return glm::length(point);
}