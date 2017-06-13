#ifndef _voxigen_worldGenerator_h_
#define _voxigen_worldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/worldDescriptors.h"

#include <noise/noise.h>

namespace voxigen
{

template<typename _Block>
class VOXIGEN_EXPORT WorldGenerator
{
public:
    WorldGenerator(int seed);
    ~WorldGenerator();

    void setWorldDiscriptors(WorldDescriptors descriptors);
    
    void generateWorldOverview();
    UniqueChunk<_Block> generateChunk(glm::ivec3 chunkIndex);

private:
    WorldDescriptors descriptors;

    noise::module::Perlin m_perlin;
    noise::module::Perlin m_continentPerlin;
    noise::module::Curve m_continentCurve;
};

template<typename _Block>
WorldGenerator<_Block>::WorldGenerator(int seed)
{
    m_seed=seed;

    m_perlin.SetSeed(m_seed+0);

    m_continentPerlin.SetSeed(m_seed+0);
    m_continentPerlin.SetFrequency(m_contientFrequency);
    m_continentPerlin.SetPersistence(0.5);
    m_continentPerlin.SetLacunarity(m_contientLacunarity);
    m_continentPerlin.SetOctaveCount(14);
    m_continentPerlin.SetNoiseQuality(noise::QUALITY_STD);

    m_continentCurve.AddControlPoint(-2.0000+m_seaLevel, -1.625+m_seaLevel);
    m_continentCurve.AddControlPoint(-1.0000+m_seaLevel, -1.375+m_seaLevel);
    m_continentCurve.AddControlPoint(0.0000+m_seaLevel, -0.375+m_seaLevel);
    m_continentCurve.AddControlPoint(0.0625+m_seaLevel, 0.125+m_seaLevel);
    m_continentCurve.AddControlPoint(0.1250+m_seaLevel, 0.250+m_seaLevel);
    m_continentCurve.AddControlPoint(0.2500+m_seaLevel, 1.000+m_seaLevel);
    m_continentCurve.AddControlPoint(0.5000+m_seaLevel, 0.250+m_seaLevel);
    m_continentCurve.AddControlPoint(0.7500+m_seaLevel, 0.250+m_seaLevel);
    m_continentCurve.AddControlPoint(1.0000+m_seaLevel, 0.500+m_seaLevel);
    m_continentCurve.AddControlPoint(2.0000+m_seaLevel, 0.500+m_seaLevel);
}

template<typename _Block>
WorldGenerator<_Block>::~WorldGenerator()
{}

template<typename _Block>
void WorldGenerator<_Block>::setWorldDiscriptors(WorldDescriptors descriptors)
{

}

template<typename _Block>
void WorldGenerator<_Block>::generateWorldOverview()
{
    noise::utils::NoiseMap map;

    //    utils::NoiseMapBuilderSphere planet;
    //    utils::NoiseMap elevGrid;
    //
    //    // Pass in the boundaries of the elevation grid to extract.
    //    planet.SetBounds(-90, 90, -180, 180);
    //    planet.SetDestSize(m_gridWidth, m_gridHeight);
}

template<typename _Block>
UniqueChunk<_Block> WorldGenerator<_Block>::generateChunk(glm::ivec3 chunkIndex)
{

}

}//namespace voxigen

#endif //_voxigen_worldGenerator_h_