#include "dt/triangle.h"
#include <glm/gtx/norm.hpp>

Triangle::Triangle(size_t v1, size_t v2, size_t v3) :
	a(v1), b(v2), c(v3), isBad(false)
{}

bool
Triangle::containsVertex(size_t v) const
{
	// return p1 == v || p2 == v || p3 == v;
	return (a == v) || (b == v) || (c == v);
}

bool
Triangle::circumCircleContains(const std::vector<Vector2> &vertexes, const VertexType &v) const
{
    const VertexType &va=vertexes[a];
    const VertexType &vb=vertexes[b];
    const VertexType &vc=vertexes[c];

//	const float ab = a->norm2();
//	const float cd = b->norm2();
//	const float ef = c->norm2();
    const float ab=glm::length2(va);
    const float cd=glm::length2(vb);
    const float ef=glm::length2(vc);

	const float &ax = va.x;
	const float &ay = va.y;
	const float &bx = vb.x;
	const float &by = vb.y;
	const float &cx = vc.x;
	const float &cy = vc.y;

	const float circum_x = (ab * (cy - by) + cd * (ay - cy) + ef * (by - ay)) / (ax * (cy - by) + bx * (ay - cy) + cx * (by - ay));
	const float circum_y = (ab * (cx - bx) + cd * (ax - cx) + ef * (bx - ax)) / (ay * (cx - bx) + by * (ax - cx) + cy * (bx - ax));

	const VertexType circum(half(circum_x), half(circum_y));
//	const float circum_radius = a->dist2(circum);
    const float circum_radius=glm::distance2(va, circum);
//	const float dist = v.dist2(circum);
    const float dist=glm::distance2(v, circum);
	return dist <= circum_radius;
}

//bool
//Triangle::operator ==(const Triangle &t) const
//{
//	return	(*this->a == *t.a || *this->a == *t.b || *this->a == *t.c) &&
//			(*this->b == *t.a || *this->b == *t.b || *this->b == *t.c) &&
//			(*this->c == *t.a || *this->c == *t.b || *this->c == *t.c);
//}

std::ostream&
operator <<(std::ostream &str, const Triangle &t)
{
	return str << "Triangle:" << "\n\t" <<
			t.a << "\n\t" <<
			t.b << "\n\t" <<
			t.c << '\n';
}

bool almost_equal(const std::vector<Vector2> &vertexes, const Triangle &t1, const Triangle &t2)
{
    const Vector2 &t1a=vertexes[t1.a];
    const Vector2 &t1b=vertexes[t1.b];
    const Vector2 &t1c=vertexes[t1.c];
    const Vector2 &t2a=vertexes[t2.a];
    const Vector2 &t2b=vertexes[t2.b];
    const Vector2 &t2c=vertexes[t2.c];

	return	(almost_equal(t1a , t2a) || almost_equal(t1a , t2b) || almost_equal(t1a , t2c)) &&
			(almost_equal(t1b , t2a) || almost_equal(t1b , t2b) || almost_equal(t1b , t2c)) &&
			(almost_equal(t1c , t2a) || almost_equal(t1c , t2b) || almost_equal(t1c , t2c));
}
