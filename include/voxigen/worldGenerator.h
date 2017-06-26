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

template<typename _Block, size_t _ChuckSizeX, size_t _ChuckSizeY, size_t _ChuckSizeZ>
class WorldGenerator
{
public:
    typedef World<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ> WorldType;
    typedef Chunk<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ> ChunkType;
    typedef UniqueChunk<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ> UniqueChunkType;

    WorldGenerator(WorldType *world);
    ~WorldGenerator();

//    void setWorld(WorldDescriptors descriptors);
//    void setWorldDiscriptors(WorldDescriptors descriptors);
    
    void generateWorldOverview();
    UniqueChunkType generateChunk(glm::ivec3 chunkIndex);

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

template<typename _Block, size_t _ChuckSizeX, size_t _ChuckSizeY, size_t _ChuckSizeZ>
WorldGenerator<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ>::WorldGenerator(WorldType *world):
m_world(world),
m_descriptors(&world->getDescriptors())
{
    int seed=m_descriptors->seed;

    m_continentPerlin.reset(FastNoiseSIMD::NewFastNoiseSIMD(seed));
    
    m_continentPerlin->SetNoiseType(FastNoiseSIMD::PerlinFractal);
    m_continentPerlin->SetFrequency(m_descriptors->contientFrequency);
    m_continentPerlin->SetFractalLacunarity(m_descriptors->contientLacunarity);
    m_continentPerlin->SetFractalOctaves(m_descriptors->contientOctaves);


    m_layersPerlin.reset(FastNoiseSIMD::NewFastNoiseSIMD(seed+1));

    m_layersPerlin->SetNoiseType(FastNoiseSIMD::PerlinFractal);
    m_layersPerlin->SetFrequency(m_descriptors->contientFrequency);
    m_layersPerlin->SetFractalLacunarity(m_descriptors->contientLacunarity);
    m_layersPerlin->SetFractalOctaves(m_descriptors->contientOctaves);

//    m_perlin.SetSeed(seed+0);
//
//    m_continentPerlin.SetSeed(seed+0);
//    m_continentPerlin.SetFrequency(m_descriptors->contientFrequency);
//    m_continentPerlin.SetPersistence(0.5);
//    m_continentPerlin.SetLacunarity(m_descriptors->contientLacunarity);
//    m_continentPerlin.SetOctaveCount(14);
//    m_continentPerlin.SetNoiseQuality(noise::QUALITY_STD);
//
//    double seaLevel=m_descriptors->seaLevel;
//
//    m_continentCurve.SetSourceModule(0, m_continentPerlin);
//    m_continentCurve.AddControlPoint(-2.0000+seaLevel, -1.625+seaLevel);
//    m_continentCurve.AddControlPoint(-1.0000+seaLevel, -1.375+seaLevel);
//    m_continentCurve.AddControlPoint(0.0000+seaLevel, -0.375+seaLevel);
//    m_continentCurve.AddControlPoint(0.0625+seaLevel, 0.125+seaLevel);
//    m_continentCurve.AddControlPoint(0.1250+seaLevel, 0.250+seaLevel);
//    m_continentCurve.AddControlPoint(0.2500+seaLevel, 1.000+seaLevel);
//    m_continentCurve.AddControlPoint(0.5000+seaLevel, 0.250+seaLevel);
//    m_continentCurve.AddControlPoint(0.7500+seaLevel, 0.250+seaLevel);
//    m_continentCurve.AddControlPoint(1.0000+seaLevel, 0.500+seaLevel);
//    m_continentCurve.AddControlPoint(2.0000+seaLevel, 0.500+seaLevel);
//
//    m_layersPerlin.SetSeed(seed+1);
//    //    m_layersPerlin.SetFrequency(m_contientFrequency);
//    //    m_layersPerlin.SetPersistence(0.5);
//    //    m_layersPerlin.SetLacunarity(m_contientLacunarity);
//    //    m_layersPerlin.SetOctaveCount(14);
//    //    m_layersPerlin.SetNoiseQuality(noise::QUALITY_STD);

}

template<typename _Block, size_t _ChuckSizeX, size_t _ChuckSizeY, size_t _ChuckSizeZ>
WorldGenerator<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ>::~WorldGenerator()
{}

//template<typename _Block, size_t _ChuckSizeX, size_t _ChuckSizeY, size_t _ChuckSizeZ>
//void WorldGenerator<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ>::setWorldDiscriptors(WorldDescriptors descriptors)
//{
//
//}

template<typename _Block, size_t _ChuckSizeX, size_t _ChuckSizeY, size_t _ChuckSizeZ>
void WorldGenerator<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ>::generateWorldOverview()
{

}

template<typename _Block, size_t _ChuckSizeX, size_t _ChuckSizeY, size_t _ChuckSizeZ>
typename WorldGenerator<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ>::UniqueChunkType WorldGenerator<_Block, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ>::generateChunk(glm::ivec3 chunkIndex)
{
    glm::vec3 offset=glm::ivec3(_ChuckSizeX, _ChuckSizeY, _ChuckSizeZ)*chunkIndex;
    glm::vec3 scaledOffset=offset*m_descriptors->noiseScale;
    glm::vec3 position=scaledOffset;
    unsigned int hash=m_descriptors->chunkHash(chunkIndex);
    
    UniqueChunkType chunk=std::make_unique<ChunkType>(hash, chunkIndex, offset);
    ChunkType::Blocks &blocks=chunk->getBlocks();

    int heightMapSize=FastNoiseSIMD::AlignedSize(_ChuckSizeX*_ChuckSizeY);
    std::vector<float> heightMap(heightMapSize);
    std::vector<float> xMap(heightMapSize);
    std::vector<float> yMap(heightMapSize);
    std::vector<float> zMap(heightMapSize);

    size_t index=0;
    glm::vec3 mapPos;

    mapPos.z=m_descriptors->size.x/2;
    for(int y=0; y<_ChuckSizeY; ++y)
    {
        mapPos.y=offset.y+y;
        for(int x=0; x<_ChuckSizeX; ++x)
        {
            mapPos.x=offset.x+x;

            glm::vec3 pos=getCylindricalCoords(m_descriptors->size.x, m_descriptors->size.y, mapPos);

            xMap[index]=pos.x;
            yMap[index]=pos.y;
            zMap[index]=pos.z;
            index++;
        }
    }

    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), _ChuckSizeX, _ChuckSizeY, 1);

    int chunkMapSize=FastNoiseSIMD::AlignedSize(_ChuckSizeX*_ChuckSizeY*_ChuckSizeZ);

    std::vector<float> layerMap(chunkMapSize);

    m_layersPerlin->FillNoiseSet(layerMap.data(), offset.x, offset.y, offset.z, _ChuckSizeX, _ChuckSizeY, _ChuckSizeZ);

    index=0;
    size_t heightIndex=0;
    position.z=scaledOffset.z;
    for(int z=0; z<_ChuckSizeZ; ++z)
    {
        heightIndex=0;
        position.y=scaledOffset.y;
        for(int y=0; y<_ChuckSizeY; ++y)
        {
            position.x=scaledOffset.x;
            for(int x=0; x<_ChuckSizeX; ++x)
            {
                float blockType;

                if(position.z > heightMap[heightIndex]) //larger than height map, air
                    blockType=0;
                else
//                    blockType=(floor(m_layersPerlin.GetValue(position.x, position.y, heightMap[heightIndex]-position.z)+1.0f)*5)+1;
                    blockType=floor((layerMap[index]+1.0f)*5.0f)+1;

                blocks[index].type=blockType;
                index++;
                heightIndex++;
                position.x+=m_descriptors->noiseScale;
            }
            position.y+=m_descriptors->noiseScale;
        }
        position.z+=m_descriptors->noiseScale;
    }

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