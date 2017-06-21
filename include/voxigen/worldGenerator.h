#ifndef _voxigen_worldGenerator_h_
#define _voxigen_worldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"

#include <noise/noise.h>

namespace voxigen
{

template<typename _Block>
class World;

template<typename _Block>
class WorldGenerator
{
public:
    WorldGenerator(World<_Block> *world);
    ~WorldGenerator();

//    void setWorld(WorldDescriptors descriptors);
//    void setWorldDiscriptors(WorldDescriptors descriptors);
    
    void generateWorldOverview();
    UniqueChunk<_Block> generateChunk(glm::ivec3 chunkIndex);

private:
    World<_Block> *m_world;
    WorldDescriptors *m_descriptors;

    noise::module::Perlin m_perlin;
    noise::module::Perlin m_continentPerlin;
    noise::module::Curve m_continentCurve;

    noise::module::Perlin m_layersPerlin;
};

template<typename _Block>
WorldGenerator<_Block>::WorldGenerator(World<_Block> *world):
m_world(world),
m_descriptors(&world->getDescriptors())
{
    int seed=m_descriptors->seed;

    m_perlin.SetSeed(seed+0);

    m_continentPerlin.SetSeed(seed+0);
    m_continentPerlin.SetFrequency(m_descriptors->contientFrequency);
    m_continentPerlin.SetPersistence(0.5);
    m_continentPerlin.SetLacunarity(m_descriptors->contientLacunarity);
    m_continentPerlin.SetOctaveCount(14);
    m_continentPerlin.SetNoiseQuality(noise::QUALITY_STD);

    double seaLevel=m_descriptors->seaLevel;

    m_continentCurve.SetSourceModule(0, m_continentPerlin);
    m_continentCurve.AddControlPoint(-2.0000+seaLevel, -1.625+seaLevel);
    m_continentCurve.AddControlPoint(-1.0000+seaLevel, -1.375+seaLevel);
    m_continentCurve.AddControlPoint(0.0000+seaLevel, -0.375+seaLevel);
    m_continentCurve.AddControlPoint(0.0625+seaLevel, 0.125+seaLevel);
    m_continentCurve.AddControlPoint(0.1250+seaLevel, 0.250+seaLevel);
    m_continentCurve.AddControlPoint(0.2500+seaLevel, 1.000+seaLevel);
    m_continentCurve.AddControlPoint(0.5000+seaLevel, 0.250+seaLevel);
    m_continentCurve.AddControlPoint(0.7500+seaLevel, 0.250+seaLevel);
    m_continentCurve.AddControlPoint(1.0000+seaLevel, 0.500+seaLevel);
    m_continentCurve.AddControlPoint(2.0000+seaLevel, 0.500+seaLevel);

    m_layersPerlin.SetSeed(seed+1);
    //    m_layersPerlin.SetFrequency(m_contientFrequency);
    //    m_layersPerlin.SetPersistence(0.5);
    //    m_layersPerlin.SetLacunarity(m_contientLacunarity);
    //    m_layersPerlin.SetOctaveCount(14);
    //    m_layersPerlin.SetNoiseQuality(noise::QUALITY_STD);

}

template<typename _Block>
WorldGenerator<_Block>::~WorldGenerator()
{}

//template<typename _Block>
//void WorldGenerator<_Block>::setWorldDiscriptors(WorldDescriptors descriptors)
//{
//
//}

template<typename _Block>
void WorldGenerator<_Block>::generateWorldOverview()
{

}

template<typename _Block>
UniqueChunk<_Block> WorldGenerator<_Block>::generateChunk(glm::ivec3 chunkIndex)
{
    glm::vec3 offset=glm::vec3(m_descriptors->chunkSize*chunkIndex);
    glm::vec3 scaledOffset=offset*m_descriptors->noiseScale;
    glm::vec3 position=scaledOffset;

    UniqueChunk<_Block> chunk=std::make_unique<Chunk<_Block>>(m_descriptors, chunkIndex, offset);
    Chunk<_Block>::Blocks &blocks=chunk->getBlocks();

    std::vector<float> heightMap(m_descriptors->chunkSize.x*m_descriptors->chunkSize.y);

    int heightIndex=0;
    position.z=0.0f;
    for(int y=0; y<m_descriptors->chunkSize.y; ++y)
    {
        position.x=scaledOffset.x;
        for(int x=0; x<m_descriptors->chunkSize.x; ++x)
        {
            double  value=(m_continentPerlin.GetValue(position.x, position.y, m_descriptors->noiseScale))+1.0;
            double blockHeight=value*(m_descriptors->size.z/2);

            heightMap[heightIndex]=blockHeight*m_descriptors->noiseScale;

//            heightMap[heightIndex]=(m_continentPerlin.GetValue(position.x, position.y, m_descriptors->noiseScale))*(m_descriptors->size.z/2)*m_descriptors->noiseScale;
            heightIndex++;
            position.x+=m_descriptors->noiseScale;
        }
        position.y+=m_descriptors->noiseScale;
    }

    int index=0;
    position.z=scaledOffset.z;
    for(int z=0; z<m_descriptors->chunkSize.z; ++z)
    {
        heightIndex=0;
        position.y=scaledOffset.y;
        for(int y=0; y<m_descriptors->chunkSize.y; ++y)
        {
            position.x=scaledOffset.x;
            for(int x=0; x<m_descriptors->chunkSize.x; ++x)
            {
                float blockType;

                if(position.z > heightMap[heightIndex]) //larger than height map, air
                    blockType=0;
                else
                    blockType=(floor(m_layersPerlin.GetValue(position.x, position.y, heightMap[heightIndex]-position.z)+1.0f)*5)+1;

                blocks[index].type=blockType;
                index++;
                heightIndex++;
                position.x+=m_descriptors->noiseScale;
            }
            position.y+=m_descriptors->noiseScale;
        }
        position.z+=m_descriptors->noiseScale;
    }

    return chunk;
}

}//namespace voxigen

#endif //_voxigen_worldGenerator_h_