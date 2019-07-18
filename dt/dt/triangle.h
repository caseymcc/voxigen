#ifndef H_TRIANGLE
#define H_TRIANGLE

#include "vector2.h"
#include "edge.h"
#include "numeric.h"

struct Triangle
{
	using EdgeType = Edge;
	using VertexType = Vector2;

	Triangle() = default;
	Triangle(const Triangle&) = default;
	Triangle(Triangle&&) = default;
//	Triangle(const VertexType &v1, const VertexType &v2, const VertexType &v3);
    Triangle(size_t v1, size_t v2, size_t v3);

	bool containsVertex(size_t v) const;
	bool circumCircleContains(const std::vector<Vector2> &vertexes, const VertexType &v) const;

	Triangle &operator=(const Triangle&) = default;
	Triangle &operator=(Triangle&&) = default;
//	bool operator ==(const Triangle &t) const;
	friend std::ostream &operator <<(std::ostream &str, const Triangle &t);

    size_t a;
    size_t b;
    size_t c;
	bool isBad = false;
};

bool equal(const std::vector<Vector2> &vertexes, const Triangle &t1, const Triangle &t2);
bool almost_equal(const std::vector<Vector2> &vertexes, const Triangle &t1, const Triangle &t2);

#endif
