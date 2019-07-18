#include "dt/edge.h"

Edge::Edge(size_t v1, size_t v2) :
	v(v1), w(v2)
{}

std::ostream&
operator <<(std::ostream &str, const Edge &e)
{
	return str << "Edge " << e.v << ", " << e.w;
}

bool equal(const std::vector<Vector2> &vetexes, const Edge &e1, const Edge &e2)
{
    const Vector2 &e1v=vetexes[e1.v];
    const Vector2 &e1w=vetexes[e1.w];
    const Vector2 &e2v=vetexes[e2.v];
    const Vector2 &e2w=vetexes[e2.w];

    return (equal(e1v, e2v)&&equal(e1w, e2w))||
        (equal(e1v, e2w)&&equal(e1w, e2v));
}

bool almost_equal(const std::vector<Vector2> &vetexes, const Edge &e1, const Edge &e2)
{
    const Vector2 &e1v=vetexes[e1.v];
    const Vector2 &e1w=vetexes[e1.w];
    const Vector2 &e2v=vetexes[e2.v];
    const Vector2 &e2w=vetexes[e2.w];

	return	(almost_equal(e1v, e2v) && almost_equal(e1w, e2w)) ||
			(almost_equal(e1v, e2w) && almost_equal(e1w, e2v));
}
