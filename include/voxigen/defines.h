#ifndef _voxigen_defines_h_
#define _voxigen_defines_h_

namespace voxigen
{

typedef unsigned __int32 ChunkHash;
typedef unsigned __int32 SegmentHash;

struct SegmentChunkHash
{
    SegmentChunkHash() {}
    SegmentChunkHash(SegmentHash segmentHash, ChunkHash chunkHash):segmentHash(segmentHash), chunkHash(chunkHash){}

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