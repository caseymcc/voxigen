#ifndef _voxigen_defines_h_
#define _voxigen_defines_h_

namespace voxigen
{

typedef unsigned int ChunkHash;
typedef unsigned int SegmentHash;

struct SegmentChunkHash
{
    SegmentChunkHash() {}
    SegmentChunkHash(SegmentHash segmentHash, ChunkHash chunkHash):segmentHash(segmentHash), chunkHash(chunkHash){}

    SegmentHash segmentHash;
    ChunkHash chunkHash;
};

}//namespace voxigen

#endif //_voxigen_worldSegment_h_