#ifndef _voxigen_defines_h_
#define _voxigen_defines_h_

#include <set>
#include <cstdint>
//#define DEBUG_ALLOCATION
//#define LOG_PROCESS_QUEUE
//#define DEBUG_DRAW_CALLS
//#define DEBUG_RENDERERS
//#define DEBUG_MESH
//#define DEBUG_THREAD
//#define DEBUG_REQUESTS
//#define DEBUG_RENDERER_STATUS
//#define DEBUG_RENDERER_MESH
//#define VOXIGEN_DEBUG_ACTIVEVOLUME
//#define VOXIGEN_DEBUG_CONTAINERVOLUME
//#define VOXIGEN_MEMORY_DEBUG

#ifdef VOXIGEN_MEMORY_DEBUG
#define MEMORY_CHECK assert(_CrtCheckMemory());
#else
#define MEMORY_CHECK
#endif

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