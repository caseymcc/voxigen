#include "dt/delaunay.h"


void delaunayTriangulate(std::vector<Vector2> &vertexes, std::vector<Triangle> &triangles, std::vector<Edge> &edges, std::vector<Vector2> &triVertexes)
{
	// Determinate the super triangle
	float minX = vertexes[0].x;
	float minY = vertexes[0].y;
	float maxX = minX;
	float maxY = minY;

	for(std::size_t i = 0; i < vertexes.size(); ++i)
	{
		if (vertexes[i].x < minX) minX = vertexes[i].x;
		if (vertexes[i].y < minY) minY = vertexes[i].y;
		if (vertexes[i].x > maxX) maxX = vertexes[i].x;
		if (vertexes[i].y > maxY) maxY = vertexes[i].y;
	}

	const float dx = maxX - minX;
	const float dy = maxY - minY;
	const float deltaMax = std::max(dx, dy);
	const float midx = half(minX + maxX);
	const float midy = half(minY + maxY);

    std::vector<int> triVertexesRef;
    std::vector<size_t> triVertexesEmpty;

	triVertexes.emplace_back(midx - 20 * deltaMax, midy - deltaMax);
	triVertexes.emplace_back(midx, midy + 20 * deltaMax);
	triVertexes.emplace_back(midx + 20 * deltaMax, midy - deltaMax);
    
	// Create a list of triangles, and add the supertriangle in it
	triangles.emplace_back(0, 1, 2);

    triVertexesRef.push_back(0);
    triVertexesRef.push_back(0);
    triVertexesRef.push_back(0);
    
    std::vector<Edge> polygon;

	for(auto &p:vertexes)
	{
        polygon.clear();

		for(auto &t:triangles)
		{
			if(t.circumCircleContains(triVertexes, p))
			{
				t.isBad = true;
				polygon.push_back(Edge{t.a, t.b});
				polygon.push_back(Edge{t.b, t.c});
				polygon.push_back(Edge{t.c, t.a});

                triVertexesRef[t.a]+=2;
                triVertexesRef[t.b]+=2;
                triVertexesRef[t.c]+=2;
			}
		}

		triangles.erase(std::remove_if(begin(triangles), end(triangles), [](Triangle &t){
			return t.isBad;
		}), end(triangles));

		for(auto e1 = begin(polygon); e1 != end(polygon); ++e1)
		{
			for(auto e2 = e1 + 1; e2 != end(polygon); ++e2)
			{
				if(almost_equal(triVertexes, *e1, *e2))
				{
					e1->isBad = true;
					e2->isBad = true;
				}
			}
		}

        for(auto iter=polygon.begin(); iter!=polygon.end();)
        {
            if(iter->isBad)
            {
                triVertexesRef[iter->v]--;
                if(triVertexesRef[iter->v]<=0)
                    triVertexesEmpty.push_back(iter->v);
                triVertexesRef[iter->w]--;
                if(triVertexesRef[iter->w]<=0)
                    triVertexesEmpty.push_back(iter->w);

                iter=polygon.erase(iter);
            }
            else
                ++iter;
        }
//        polygon.erase(std::remove_if(begin(polygon), end(polygon), [](EdgeType &e){
//			return e.isBad;
//		}), end(polygon));

        size_t pointIndex;
        
        if(triVertexesEmpty.empty())
        {
            pointIndex=triVertexes.size();
            triVertexes.emplace_back(p);
            triVertexesRef.push_back(0);
        }
        else
        {
            pointIndex=triVertexesEmpty.back();
            triVertexesEmpty.pop_back();
            triVertexes[pointIndex]=p;
        }
                

		for(const auto e : polygon)
			triangles.emplace_back(e.v, e.w, pointIndex);

	}

//	triangles.erase(std::remove_if(begin(triangles), end(triangles), [p1, p2, p3](TriangleType &t){
//		return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
//	}), end(triangles));

	for(const auto t : triangles)
	{
		edges.push_back(Edge{t.a, t.b});
		edges.push_back(Edge{t.b, t.c});
		edges.push_back(Edge{t.c, t.a});
	}

}
