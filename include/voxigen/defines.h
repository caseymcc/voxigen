#ifndef _voxigen_defines_h_
#define _voxigen_defines_h_

namespace voxigen
{

typedef unsigned __int32 ChunkHash;
typedef unsigned __int32 SegmentHash;

struct Key
{
    typedef __int64 Type;

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

}//namespace voxigen

#endif //_voxigen_worldSegment_h_