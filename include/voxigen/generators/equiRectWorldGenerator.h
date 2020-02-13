#ifndef _voxigen_equiRectWorldGenerator_h_
#define _voxigen_equiRectWorldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/volume/chunk.h"
#include "voxigen/volume/gridDescriptors.h"
#include "voxigen/maths/coords.h"
#include "voxigen/meshes/heightMap.h"
#include "voxigen/noise.h"
#include "voxigen/generators/tectonics.h"
#include "voxigen/generators/weather.h"
#include "voxigen/generators/perturbedWeather.h"
#include "voxigen/wrap.h"
#include "voxigen/fill.h"
#include "voxigen/maths/math_helpers.h"
#include "voxigen/sortedVector.h"

#include "voxigen/maths/glm_point.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/integer.hpp>

#undef None

#include <cassert>
#include <random>
#include <chrono>
namespace chrono=std::chrono;

namespace voxigen
{

constexpr int NeighborCount=4;

//this is expecting cylindrical wrap
VOXIGEN_EXPORT std::vector<size_t> get2DCellNeighbors_eq(const glm::ivec2 &index, const glm::ivec2 &size);

struct VOXIGEN_EXPORT EquiRectDescriptors
{
    EquiRectDescriptors()
    {
        m_noiseScale=0.001f;

        //    contientFrequency=1.0;
        m_continentFrequency=0.005f;
        m_continentOctaves=2;
        m_continentLacunarity=2.2f;

        m_seaLevel=0.0f;

        m_plateCount=16;
        m_plateCountMin=8;
        m_plateCountMax=24;

        m_plateFrequency=0.00025f;
        m_plateOctaves=3;
        m_plateLacunarity=2.0f;

        m_influenceGridSize={4096, 4096};
    }

    void calculateInfluenceSize(IGridDescriptors *gridDescriptors)
    {
        glm::ivec3 size=gridDescriptors->getSize();

        m_influenceSize=glm::ivec2(size.x, size.y)/m_influenceGridSize;

        while(m_influenceSize.x*m_influenceGridSize.x<size.x)
            m_influenceSize.x++;
        while(m_influenceSize.y*m_influenceGridSize.y<size.y)
            m_influenceSize.y++;

        m_plateFrequency=2.56f/m_influenceSize.x;
        m_continentFrequency=10*m_plateFrequency;
    }

    bool load(const char *json);
    bool save(char *json, size_t &size);

    void init(IGridDescriptors *gridDescriptors);

    float m_noiseScale;
    float m_continentFrequency;
    int m_continentOctaves;
    float m_continentLacunarity;

    float m_seaLevel;
    float m_continentalShelf;

    int m_plateCount;
    int m_plateCountMin;
    int m_plateCountMax;

    float m_plateFrequency;
    int m_plateOctaves;
    float m_plateLacunarity;

    glm::ivec2 m_influenceSize;
    glm::ivec2 m_influenceGridSize;
};

constexpr unsigned int EquiRectWorldGeneratorHeader_Marker=0x0f0f0f0f;
struct EquiRectWorldGeneratorHeader
{
    unsigned int marker;
    unsigned int version;
    unsigned int x;
    unsigned int y;
    unsigned int cellSize;
    unsigned int size;
};

template<typename _Grid>
class EquiRectWorldGenerator
{
public:
    typedef typename _Grid::RegionType Region;

    typedef typename _Grid::ChunkType ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunkType;

    typedef std::vector<InfluenceCell> InfluenceMap;

    EquiRectWorldGenerator();
    ~EquiRectWorldGenerator();

    static const char *typeName() { return "EquiRectWorldGenerator"; }

    void create(IGridDescriptors *descriptors);
    template<typename _FileIO>
    bool load(IGridDescriptors *descriptors, const std::string &directory);
    template<typename _FileIO>
    void save(const std::string &directory);

    void loadDescriptors(IGridDescriptors *descriptors);
    void saveDescriptors(IGridDescriptors *descriptors);
    void saveDescriptors(std::string &descriptors);
    //    void initialize(IGridDescriptors *descriptors);
    //    void setWorld(GridDescriptors descriptors);
    //    void setWorldDiscriptors(GridDescriptors descriptors);

    void generateWorldOverview();

    //    UniqueChunkType generateChunk(unsigned int hash, void *buffer, size_t bufferSize);
    //    UniqueChunkType generateChunk(glm::ivec3 chunkIndex, void *buffer, size_t bufferSize);
    //    UniqueChunkType generateChunk(unsigned int hash, glm::ivec3 &chunkIndex, void *buffer, size_t bufferSize);
    unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod);
    unsigned int generateRegion(const glm::vec3 &startPos, const glm::ivec3 &regionSize, void *buffer, size_t bufferSize, size_t lod);

    int getBaseHeight(const glm::vec2 &pos);

    //for debugging
    int getPlateCount() { return m_plateCount; }
    const InfluenceMap &getInfluenceMap() { return m_influenceMap; }
    const glm::ivec2 &getInfluenceMapSize() { return m_descriptorValues.m_influenceSize; }

    EquiRectDescriptors &getDecriptors() { return m_descriptorValues; }

    int m_plateSeed;
    int m_plateCount;
    std::vector<glm::vec2> m_influencePoints;
    std::vector<std::vector<glm::vec2>> m_influenceLines;

private:
    void initialize(IGridDescriptors *descriptors);

    template<typename _FileIO>
    bool loadWorldOverview(const std::string &directory);
    template<typename _FileIO>
    void saveWorldOverview(const std::string &directory);

    void buildHeightMap(const glm::vec3 &startPos, const glm::ivec3 &lodSize, size_t stride);

    void generatePlates();
    void generateContinents();

    void updateInfluenceNeighbors();

    GridDescriptors<_Grid> *m_descriptors;
    EquiRectDescriptors m_descriptorValues;

    size_t m_simdLevel;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentPerlin;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_layersPerlin;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_cellularNoise;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentCellular;

    int m_continentSeed;

    InfluenceMap m_influenceMap;
    std::vector<float> m_influenceNeighborMap;

    std::unique_ptr<HastyNoise::VectorSet> m_influenceVectorSet;

    //    Regular2DGrid<InfluenceCell> m_influence;
    //    noise::module::Perlin m_perlin;
    //    noise::module::Perlin m_continentPerlin;
    //    noise::module::Curve m_continentCurve;
    //
    //    noise::module::Perlin m_layersPerlin;

        //convert these to scratch buffer request as this is not thread safe
    static std::vector<float> heightMap;
    static std::vector<float> blockHeightMap;
    static std::vector<float> blockScaleMap;
    static std::vector<float> xMap;
    static std::vector<float> yMap;
    static std::vector<float> zMap;
    static std::vector<float> layerMap;
    static std::unique_ptr<HastyNoise::VectorSet> vectorSet;

    static std::vector<float> regionHeightMap;
    static std::unique_ptr<HastyNoise::VectorSet> regionVectorSet;

    //timing
    double allocationTime=0.0;
    double coordsTime=0.0;
    double cellularPlateTime=0.0;
    double cellularPlate2Time=0.0;
    double cellularDistanceTime=0.0;
    double processingTime=0.0;
};

}//namespace voxigen

#include "voxigen/generators/equiRectWorldGenerator.inl"

#endif //_voxigen_equiRectEquiRectWorldGenerator_h_