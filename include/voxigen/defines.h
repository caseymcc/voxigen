#ifndef _voxigen_defines_h_
#define _voxigen_defines_h_

#include <set>

namespace voxigen
{

typedef unsigned __int32 ChunkHash;
typedef unsigned __int32 RegionHash;

struct Key
{
    typedef __int64 Type;
    typedef ChunkHash ChunkHashType;
    typedef RegionHash RegionHashType;

    Key() {}
    Key(__int64 hash):hash(hash){}
    Key(RegionHash regionHash, ChunkHash chunkHash):regionHash(regionHash), chunkHash(chunkHash){}

    union
    {
        struct
        {
            RegionHash regionHash;
            ChunkHash chunkHash;
        };
        unsigned __int64 hash;
    };
};

typedef std::set<Key::ChunkHashType> ChunkHashSet;

}//namespace voxigen

#endif //_voxigen_worldRegion_h_