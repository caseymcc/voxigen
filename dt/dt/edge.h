#ifndef H_EDGE
#define H_EDGE

#include "vector2.h"

#include <ostream>
#include <vector>

struct Edge
{
    using VertexType = Vector2;

	Edge() = default;
	Edge(const Edge&) = default;
	Edge(Edge&&) = default;
//	Edge(const VertexType &v1, const VertexType &v2);
    Edge(size_t v1, size_t v2);

	Edge &operator=(const Edge&) = default;
	Edge &operator=(Edge&&) = default;
//	bool operator ==(const Edge &e) const;
	friend std::ostream &operator <<(std::ostream &str, const Edge &e);

//	const VertexType *v;
//	const VertexType *w;
    size_t v;
    size_t w;
	bool isBad = false;
};

bool equal(const std::vector<Vector2> &vertexes, const Edge &e1, const Edge &e2);
bool almost_equal(const std::vector<Vector2> &vertexes, const Edge &e1, const Edge &e2);

#endif

