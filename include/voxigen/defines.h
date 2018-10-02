#ifndef _voxigen_defines_h_
#define _voxigen_defines_h_

#include <set>
#include <cstdint>
//#define DEBUG_ALLOCATION
//#define LOG_PROCESS_QUEUE
//#define DEBUG_DRAW_CALLS
//#define DEBUG_RENDERERS
//#define DEBUG_MESH

namespace voxigen
{

typedef uint32_t ChunkHash;
typedef uint32_t RegionHash;

struct Key
{
    typedef uint64_t Type;
    typedef ChunkHash ChunkHashType;
    typedef RegionHash RegionHashType;

    Key() {}
    Key(uint64_t hash):hash(hash){}
    Key(RegionHash regionHash, ChunkHash chunkHash):regionHash(regionHash), chunkHash(chunkHash){}

    union
    {
        struct
        {
            RegionHash regionHash;
            ChunkHash chunkHash;
        };
        uint64_t hash;
    };
};

typedef std::set<Key::ChunkHashType> ChunkHashSet;

}//namespace voxigen

#endif //_voxigen_worldRegion_h_