#ifndef _voxigen_defines_h_
#define _voxigen_defines_h_

#include <set>

namespace voxigen
{

typedef unsigned __int32 ChunkHash;
typedef unsigned __int32 SegmentHash;

struct Key
{
    typedef __int64 Type;
    typedef ChunkHash ChunkHashType;
    typedef SegmentHash SegmentHashType;

    Key() {}
    Key(__int64 hash):hash(hash){}
    Key(SegmentHash segmentHash, ChunkHash chunkHash):segmentHash(segmentHash), chunkHash(chunkHash){}

    union
    {
        struct
        {
            SegmentHash segmentHash;
            ChunkHash chunkHash;
        };
        unsigned __int64 hash;
    };
};

typedef std::set<Key::ChunkHashType> ChunkHashSet;

}//namespace voxigen

#endif //_voxigen_worldSegment_h_