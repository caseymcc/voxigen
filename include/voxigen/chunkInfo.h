#ifndef _voxigen_chunkInfo_h_
#define _voxigen_chunkInfo_h_

#include <type_traits>

namespace voxigen
{

template<typename _Block, int _SizeX, int _SizeY, int _SizeZ>
struct ChunkInfo
{
    typedef _Block BlockType;
    typedef std::integral_constant<int, _SizeX> sizeX;
    typedef std::integral_constant<int, _SizeY> sizeY;
    typedef std::integral_constant<int, _SizeZ> sizeZ;
};

} //namespace voxigen

#endif //_voxigen_chunkInfo_h_