#ifndef H_VECTOR2
#define H_VECTOR2

#include "numeric.h"

#include <iostream>
#include <cmath>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

using Vector2=glm::vec2;
//struct Vector2
//{
//	Vector2() = default;
//	Vector2(const Vector2 &v) = default;
//	Vector2(Vector2&&) = default;
//	Vector2(const double vx, const double vy);
//
//	double dist2(const Vector2 &v) const;
//	double dist(const Vector2 &v) const;
//	double norm2() const;
//
//	Vector2 &operator=(const Vector2&) = default;
//	Vector2 &operator=(Vector2&&) = default;
//	bool operator ==(const Vector2 &v) const;
//	friend std::ostream &operator <<(std::ostream &str, const Vector2 &v);
//
//	double x;
//	double y;
//};

inline std::ostream &operator<<(std::ostream &out, const glm::vec2 &g)
{
    return out<<glm::to_string(g);
}

bool equal(const Vector2 &v1, const Vector2 &v2);
bool almost_equal(const Vector2 &v1, const Vector2 &v2, int ulp=2);

#endif
