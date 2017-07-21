#ifndef _voxigen_worldGenerator_h_
#define _voxigen_worldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"
#include "voxigen/coords.h"

#include <noise/noise.h>
#include <voxigen/noise/FastNoiseSIMD.h>

namespace voxigen
{

template<typename _Block, size_t _ChuckSizeX, size_t _ChuckSizeY, size_t _ChuckSizeZ>
class World;

template<typename _Chunk>
class WorldGenerator
{
public:
    typedef World<typename _Chunk::BlockType, _Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value> WorldType;
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<_Chunk> UniqueChunkType;

    WorldGenerator(WorldDescriptors *descriptors);
    ~WorldGenerator();

//    void setWorld(WorldDescriptors descriptors);
//    void setWorldDiscriptors(WorldDescriptors descriptors);
    
    void generateWorldOverview();

    UniqueChunkType generateChunk(unsigned int hash);
    UniqueChunkType generateChunk(glm::ivec3 chunkIndex);
    UniqueChunkType generateChunk(unsigned int hash, const glm::ivec3 &chunkIndex);

private:
    WorldType *m_world;
    WorldDescriptors *m_descriptors;

    std::unique_ptr<FastNoiseSIMD> m_continentPerlin;
    std::unique_ptr<FastNoiseSIMD> m_layersPerlin;

//    noise::module::Perlin m_perlin;
//    noise::module::Perlin m_continentPerlin;
//    noise::module::Curve m_continentCurve;
//
//    noise::module::Perlin m_layersPerlin;
};

template<typename _Chunk>
WorldGenerator<_Chunk>::WorldGenerator(WorldDescriptors *descriptors):
m_descriptors(descriptors)
{
    int seed=m_descriptors->m_seed;

    m_continentPerlin.reset(FastNoiseSIMD::NewFastNoiseSIMD(seed));
    
    m_continentPerlin->SetNoiseType(FastNoiseSIMD::PerlinFractal);
    m_continentPerlin->SetFrequency(m_descriptors->m_contientFrequency);
    m_continentPerlin->SetFractalLacunarity(m_descriptors->m_contientLacunarity);
    m_continentPerlin->SetFractalOctaves(m_descriptors->m_contientOctaves);


    m_layersPerlin.reset(FastNoiseSIMD::NewFastNoiseSIMD(seed+1));

    m_layersPerlin->SetNoiseType(FastNoiseSIMD::PerlinFractal);
    m_layersPerlin->SetFrequency(m_descriptors->m_contientFrequency);
    m_layersPerlin->SetFractalLacunarity(m_descriptors->m_contientLacunarity);
    m_layersPerlin->SetFractalOctaves(m_descriptors->m_contientOctaves);
}

template<typename _Chunk>
WorldGenerator<_Chunk>::~WorldGenerator()
{}

//template<typename _Chunk>
//void WorldGenerator<_Chunk>::setWorldDiscriptors(WorldDescriptors descriptors)
//{
//
//}

template<typename _Chunk>
void WorldGenerator<_Chunk>::generateWorldOverview()
{

}

template<typename _Chunk>
typename WorldGenerator<_Chunk>::UniqueChunkType WorldGenerator<_Chunk>::generateChunk(unsigned int hash)
{
    glm::ivec3 chunkIndex=m_descriptors->chunkIndex(hash);

    return generateChunk(hash, chunkIndex);
}

template<typename _Chunk>
typename WorldGenerator<_Chunk>::UniqueChunkType WorldGenerator<_Chunk>::generateChunk(glm::ivec3 chunkIndex)
{
    unsigned int hash=m_descriptors->chunkHash(chunkIndex);

    return generateChunk(hash, chunkIndex);
}

template<typename _Chunk>
typename WorldGenerator<_Chunk>::UniqueChunkType WorldGenerator<_Chunk>::generateChunk(unsigned int hash, const glm::ivec3 &chunkIndex)
{
    glm::vec3 offset=glm::ivec3(_Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value)*chunkIndex;
    glm::vec3 scaledOffset=offset*m_descriptors->m_noiseScale;
    glm::vec3 position=scaledOffset;
    float noiseScale=m_descriptors->m_noiseScale;
    
   
    UniqueChunkType chunk=std::make_unique<ChunkType>(hash, 0, chunkIndex, offset);
    ChunkType::Blocks &blocks=chunk->getBlocks();

    int heightMapSize=FastNoiseSIMD::AlignedSize(_Chunk::sizeX::value*_Chunk::sizeY::value);
    std::vector<float> heightMap(heightMapSize);
    std::vector<float> xMap(heightMapSize);
    std::vector<float> yMap(heightMapSize);
    std::vector<float> zMap(heightMapSize);

    size_t index=0;
    glm::vec3 mapPos;
    glm::vec3 size=m_descriptors->m_size;

    mapPos.z=size.x/2;
    for(int y=0; y<_Chunk::sizeY::value; ++y)
    {
        mapPos.y=offset.y+y;
        for(int x=0; x<_Chunk::sizeX::value; ++x)
        {
            mapPos.x=offset.x+x;

            glm::vec3 pos=getCylindricalCoords(size.x, size.y, mapPos);

//            pos*=m_descriptors->noiseScale;
            xMap[index]=pos.x;
            yMap[index]=pos.y;
            zMap[index]=pos.z;
            index++;
        }
    }

    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), _Chunk::sizeX::value, _Chunk::sizeY::value, 1);

    int chunkMapSize=FastNoiseSIMD::AlignedSize(_Chunk::sizeX::value*_Chunk::sizeY::value*_Chunk::sizeZ::value);

    std::vector<float> layerMap(chunkMapSize);

//    m_layersPerlin->FillNoiseSet(layerMap.data(), offset.x, offset.y, offset.z, _Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value);

    int seaLevel=(size.z/2);
    float heightScale=(size.z/2);
    unsigned int validBlocks=0;

    index=0;
    size_t heightIndex=0;
    position.z=scaledOffset.z;
    for(int z=0; z<_Chunk::sizeZ::value; ++z)
    {
        heightIndex=0;
        position.y=scaledOffset.y;

        int blockZ=offset.z+z;
        for(int y=0; y<_Chunk::sizeY::value; ++y)
        {
            position.x=scaledOffset.x;
            for(int x=0; x<_Chunk::sizeX::value; ++x)
            {
                unsigned int blockType;
                int blockHeight=heightMap[heightIndex]*heightScale+seaLevel;

//                if(position.z > heightMap[heightIndex]) //larger than height map, air
                if(blockZ>blockHeight)
                    blockType=0;
                else
                {
                    //                    blockType=(floor(m_layersPerlin.GetValue(position.x, position.y, heightMap[heightIndex]-position.z)+1.0f)*5)+1;
                    //                    blockType=floor((layerMap[index]+1.0f)*5.0f)+1;
                    blockType=(blockHeight-blockZ)/13;
                    validBlocks++;
                }

                blocks[index].type=blockType;
                index++;
                heightIndex++;
                position.x+=noiseScale;
            }
            position.y+=noiseScale;
        }
        position.z+=noiseScale;
    }

    chunk->setValidBlockCount(validBlocks);

//    int heightIndex=0;
//    position.z=0.0f;
//    for(int y=0; y<_ChuckSizeY; ++y)
//    {
//        position.x=scaledOffset.x;
//        for(int x=0; x<_ChuckSizeX; ++x)
//        {
//            double value=(m_continentPerlin.GetValue(position.x, position.y, m_descriptors->noiseScale))+1.0;
//            double blockHeight=value*(m_descriptors->size.z/2);
//
//            heightMap[heightIndex]=blockHeight*m_descriptors->noiseScale;
//
////            heightMap[heightIndex]=(m_continentPerlin.GetValue(position.x, position.y, m_descriptors->noiseScale))*(m_descriptors->size.z/2)*m_descriptors->noiseScale;
//            heightIndex++;
//            position.x+=m_descriptors->noiseScale;
//        }
//        position.y+=m_descriptors->noiseScale;
//    }
//
//    int index=0;
//    position.z=scaledOffset.z;
//    for(int z=0; z<_ChuckSizeZ; ++z)
//    {
//        heightIndex=0;
//        position.y=scaledOffset.y;
//        for(int y=0; y<_ChuckSizeY; ++y)
//        {
//            position.x=scaledOffset.x;
//            for(int x=0; x<_ChuckSizeX; ++x)
//            {
//                float blockType;
//
//                if(position.z > heightMap[heightIndex]) //larger than height map, air
//                    blockType=0;
//                else
//                    blockType=(floor(m_layersPerlin.GetValue(position.x, position.y, heightMap[heightIndex]-position.z)+1.0f)*5)+1;
//
//                blocks[index].type=blockType;
//                index++;
//                heightIndex++;
//                position.x+=m_descriptors->noiseScale;
//            }
//            position.y+=m_descriptors->noiseScale;
//        }
//        position.z+=m_descriptors->noiseScale;
//    }

    return chunk;
}

}//namespace voxigen

#endif //_voxigen_worldGenerator_h_