#include "voxigen/worldDescriptors.h"

namespace voxigen
{

WorldDescriptors::WorldDescriptors()
{
    seed=0;
    size=glm::ivec3(1024, 1024, 256);
    chunkSize=glm::ivec3(16, 16, 16);

    noiseScale=0.00001;

    contientFrequency=1.0;
    contientLacunarity=2.208984375;

    seaLevel=0.0f;
}

void WorldDescriptors::init()
{
    chunkCount=size/chunkSize;
    chunkStride=glm::ivec3(1, chunkCount.x, chunkCount.x*chunkCount.y);
}

unsigned int WorldDescriptors::chunkHash(const glm::ivec3 &chunkIndex) const
{
    return (chunkStride.z*chunkIndex.z)+(chunkStride.y*chunkIndex.y)+chunkIndex.x;
//    return (chunkStride.x*index.x)+(chunkStride.y*index.y)+index.z;
}

glm::ivec3 WorldDescriptors::chunkIndex(unsigned int chunkHash) const
{
    glm::ivec3 index;

    index.z=chunkHash/chunkStride.z;
    chunkHash=chunkHash-(chunkStride.z*index.z);
    index.y=chunkHash/chunkStride.y;
    index.x=chunkHash-(chunkStride.y*index.y);
    
    return index;
}

}//namespace voxigen
