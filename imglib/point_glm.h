#include <glm/glm.hpp>

namespace imglib
{

inline int &x(glm::ivec2 &point)
{
	return point.x;
}

inline int &y(glm::ivec2 &point)
{
	return point.y;
}

namespace traits
{

template<>
struct Value<glm::ivec2>
{
	using Type=int;
};

}
}//namespace imglib