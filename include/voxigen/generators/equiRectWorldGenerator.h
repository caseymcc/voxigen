#ifndef _voxigen_equiRectWorldGenerator_h_
#define _voxigen_equiRectWorldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/volume/chunk.h"
#include "voxigen/volume/gridDescriptors.h"
#include "voxigen/maths/coords.h"
#include "voxigen/meshes/heightMap.h"
#include "voxigen/noise.h"

#include "voxigen/maths/glm_point.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/integer.hpp>

//#include "dt/delaunay.h"

#undef None

#include <cassert>
#include <random>
#include <chrono>
namespace chrono=std::chrono;

namespace voxigen
{

template< typename T >
typename std::vector<T>::iterator
insert_sorted(std::vector<T> &container, T const &item)
{
    return container.insert
    (
        std::upper_bound(container.begin(), container.end(), item),
        item
    );
}

template<class T>
bool contains_sorted(const std::vector<T> &container, const T &item)
{
    auto iter=std::lower_bound(
        container.begin(),
        container.end(),
        item,
        [](const T &l, const T &r){ return l<r; });
    return iter!=container.end()&&*iter==item;
}

template<class T>
size_t index_sorted(const std::vector<T> &container, const T &item)
{
    auto iter=std::lower_bound(
        container.begin(),
        container.end(),
        item,
        [](const T &l, const T &r){ return l<r; });

    if(iter!=container.end()&&*iter==item)
        return iter-container.begin();
    return std::numeric_limits<size_t>::max();
}

struct VOXIGEN_EXPORT InfluenceCell
{
    InfluenceCell():
        point(false),
        heightBase(0.0f),
        heightRange(0.0f),
        tectonicPlate(0),
        borderPlate(0),
        plateValue(0.0f),
        plateDistanceValue(0.0f),
        continentValue(0.0f),
        airCurrent(0.0f)
    {

    }

    bool point;
    float heightBase;
    float heightRange;

    size_t tectonicPlate;
    size_t borderPlate;
    float plateHeight;
    float plateValue;
    float plateDistanceValue;
    float continentValue;

    float collision;
    float terrainScale;

    float airCurrent;
};

struct VOXIGEN_EXPORT EquiRectDescriptors
{
    EquiRectDescriptors()
    {
        m_noiseScale=0.001f;

        m_heightmapFrequency=0.005f;
        m_heightmapOctaves=2;
        m_heightmapLacunarity=2.2f;
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
    
    float m_heightmapFrequency;
    int m_heightmapOctaves;
    float m_heightmapLacunarity;

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

    void initialize(IGridDescriptors *descriptors);
    void save(IGridDescriptors *descriptors);
    void save(std::string &descriptors);
    //    void initialize(IGridDescriptors *descriptors);
    //    void setWorld(GridDescriptors descriptors);
    //    void setWorldDiscriptors(GridDescriptors descriptors);

    void generateWorldOverview();

    //    UniqueChunkType generateChunk(unsigned int hash, void *buffer, size_t bufferSize);
    //    UniqueChunkType generateChunk(glm::ivec3 chunkIndex, void *buffer, size_t bufferSize);
    //    UniqueChunkType generateChunk(unsigned int hash, glm::ivec3 &chunkIndex, void *buffer, size_t bufferSize);
    unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod);
    unsigned int generateRegion(const glm::vec3 &startPos, const glm::ivec3 &regionSize, void *buffer, size_t bufferSize, size_t lod);

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
    void buildHeightMap(const glm::vec3 &startPos, const glm::ivec3 &lodSize, size_t stride);

    void generatePlates();
    void generateContinents();

    GridDescriptors<_Grid> *m_descriptors;
    EquiRectDescriptors m_descriptorValues;

    size_t m_simdLevel;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_heightmapNoise;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentPerlin;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_layersPerlin;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_cellularNoise;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentCellular;

    int m_continentSeed;

    InfluenceMap m_influenceMap;
    std::unique_ptr<HastyNoise::VectorSet> m_influenceVectorSet;

    //    Regular2DGrid<InfluenceCell> m_influence;
    //    noise::module::Perlin m_perlin;
    //    noise::module::Perlin m_continentPerlin;
    //    noise::module::Curve m_continentCurve;
    //
    //    noise::module::Perlin m_layersPerlin;

        //convert these to scratch buffer request as this is not thread safe
    static std::vector<float> heightMap;
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

template<typename _Grid>
std::vector<float> EquiRectWorldGenerator<_Grid>::heightMap;
template<typename _Grid>
std::vector<float> EquiRectWorldGenerator<_Grid>::xMap;
template<typename _Grid>
std::vector<float> EquiRectWorldGenerator<_Grid>::yMap;
template<typename _Grid>
std::vector<float> EquiRectWorldGenerator<_Grid>::zMap;
template<typename _Grid>
std::vector<float> EquiRectWorldGenerator<_Grid>::layerMap;
template<typename _Grid>
std::unique_ptr<HastyNoise::VectorSet> EquiRectWorldGenerator<_Grid>::vectorSet;

template<typename _Grid>
std::vector<float> EquiRectWorldGenerator<_Grid>::regionHeightMap;
template<typename _Grid>
std::unique_ptr<HastyNoise::VectorSet> EquiRectWorldGenerator<_Grid>::regionVectorSet;

template<typename _Grid>
EquiRectWorldGenerator<_Grid>::EquiRectWorldGenerator()
{
    m_plateCount=16;
    initNoise();//make sure noise dlls are loaded
}

template<typename _Grid>
EquiRectWorldGenerator<_Grid>::~EquiRectWorldGenerator()
{}

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::initialize(IGridDescriptors *descriptors)
{
    m_descriptors=(GridDescriptors<_Grid> *)(descriptors);

    bool loaded=m_descriptorValues.load(m_descriptors->m_generatorDescriptors.c_str());

    if(!loaded)
        save(m_descriptors->m_generatorDescriptors);

    m_descriptorValues.init(m_descriptors);

    //    m_descriptors=descriptors;
    //    assert(m_descriptors!=nullptr);

    assert(m_descriptors->m_chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
    int seed=m_descriptors->m_seed;
    m_simdLevel=HastyNoise::GetFastestSIMD();

    m_heightmapNoise=HastyNoise::CreateNoise(seed, m_simdLevel);
    m_heightmapNoise->SetNoiseType(HastyNoise::NoiseType::PerlinFractal);
    m_heightmapNoise->SetFrequency(m_descriptorValues.m_heightmapFrequency);
    m_heightmapNoise->SetFractalLacunarity(m_descriptorValues.m_heightmapLacunarity);
    m_heightmapNoise->SetFractalOctaves(m_descriptorValues.m_heightmapOctaves);

    m_continentPerlin=HastyNoise::CreateNoise(seed, m_simdLevel);

    m_continentSeed=seed;
    m_continentPerlin->SetNoiseType(HastyNoise::NoiseType::PerlinFractal);
    m_continentPerlin->SetFrequency(m_descriptorValues.m_continentFrequency);
    m_continentPerlin->SetFractalLacunarity(m_descriptorValues.m_continentLacunarity);
    m_continentPerlin->SetFractalOctaves(m_descriptorValues.m_continentOctaves);
    

    m_layersPerlin=HastyNoise::CreateNoise(seed+1, m_simdLevel);

    m_layersPerlin->SetNoiseType(HastyNoise::NoiseType::PerlinFractal);
    m_layersPerlin->SetFrequency(m_descriptorValues.m_continentFrequency);
    m_layersPerlin->SetFractalLacunarity(m_descriptorValues.m_continentLacunarity);
    m_layersPerlin->SetFractalOctaves(m_descriptorValues.m_continentOctaves);

    m_plateSeed=seed+2;
    m_cellularNoise=HastyNoise::CreateNoise(seed+2, m_simdLevel);

    m_cellularNoise->SetNoiseType(HastyNoise::NoiseType::Cellular);

    vectorSet=std::make_unique<HastyNoise::VectorSet>(m_simdLevel);
    regionVectorSet=std::make_unique<HastyNoise::VectorSet>(m_simdLevel);

    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> plateDistribution(m_descriptorValues.m_plateCountMin, m_descriptorValues.m_plateCountMax);

    //    m_plateCount=plateDistribution(generator);

    generateWorldOverview();
}

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::save(IGridDescriptors *descriptors)
{
    std::string descriptorsString;

    save(descriptorsString);
    descriptors->setGeneratorDescriptors(descriptorsString.c_str());
}

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::save(std::string &descriptors)
{
    size_t size=1024;

    descriptors.resize(size);

    //removing const as we plan to stay inside the memory range from the resize above
    if(!m_descriptorValues.save((char *)descriptors.data(), size))
    {
        descriptors.resize(size);
        m_descriptorValues.save((char *)descriptors.data(), size);
    }
    descriptors.resize(size);
}

//template<typename _Grid>
//void EquiRectWorldGenerator<_Grid>::setWorldDiscriptors(GridDescriptors descriptors)
//{
//
//}

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::generateWorldOverview()
{

//    generatePlates();
    //    generateContinents();
}

//#define REDBLOG
#ifdef REDBLOG
template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::generatePlates()
{
    glm::ivec2 influenceSize=m_descriptorValues.m_influenceSize;
    int influenceMapSize=HastyNoise::AlignedSize(influenceSize.x*influenceSize.y, m_simdLevel);

    m_influenceMap.resize(influenceMapSize);

    std::vector<glm::vec3> sphere;
    std::vector<glm::vec3> cartSphere;
    std::vector<glm::vec2> stereographicSphere;
    std::vector<glm::vec2> points;

    std::vector<float> jitter;
    std::default_random_engine generator(m_descriptors->m_seed);
    std::uniform_real_distribution<float> jitterDistribution(-1.0f, 1.0f);
    
    jitter.resize(m_plateCount);
    for(size_t i=0; i<m_plateCount; ++i)
    {
        jitter[i]=jitterDistribution(generator);
    }

    generateFibonacciSphere(m_plateCount, sphere, jitter);
    sphericalToCartesian(sphere, cartSphere);

    sphericalToEquirectangular(sphere, m_influencePoints);
    for(auto &point:m_influencePoints)
    {
        if(point.x>1.0f)
            point.x=1.0f;

        point.x=point.x*influenceSize.x;
        point.y=point.y*influenceSize.y;
    }

    cartesianToStereographic(cartSphere, stereographicSphere);

    std::vector<Triangle> triangles;// =delaunay.triangulate(stereographicSphere);
    std::vector<Edge> edges;// =delaunay.getEdges();
    delaunayTriangulate(stereographicSphere, triangles, edges, points);

    std::vector<glm::vec3> cartPoints;
    //    std::vector<glm::vec3> sphereicalPoints;

    stereographicToCartesian(points, cartPoints);
    //    insertPoint(glm::vec3(0.0f, 0.0f, 1.0f), cartPoints, triangles, edges);

    //    sphere.resize(2);
    //    sphere[0]={1.0f, 0.1f, 0.0f};
    //    sphere[1]={1.0f, -0.1f, 0.0f};
    //
    //    sphericalToCartesian(sphere, cartPoints);
    //    edges.emplace_back(0,1);

    std::vector<glm::vec3> cartPolyLine;

    cartPolyLine.resize(2);
    m_influenceLines.resize(edges.size());

    //calculate pixel size in normalized coords
    float distanceThresh=(2.0f*glm::pi<float>())/influenceSize.x;

    //distance is measure as squared distance
    distanceThresh=distanceThresh*distanceThresh;

    for(size_t i=0; i<edges.size(); ++i)
    {
        cartPolyLine[0]=cartPoints[edges[i].v];
        cartPolyLine[1]=cartPoints[edges[i].w];

        projectPolyLine<Projections::Cartesian, Projections::Equirectangular>(cartPolyLine, m_influenceLines[i], distanceThresh);
    }
    for(size_t i=0; i<m_influenceLines.size(); ++i)
    {
        for(auto &point:m_influenceLines[i])
        {
            point.x=point.x*(influenceSize.x-1);
            point.y=point.y*(influenceSize.y-1);
        }
    }

    //    std::vector<glm::vec2> stereographicPolyLine;
    //    float max=0.0f;
    //    float min=0.0f;
    //
    //    m_influenceLines.resize(edges.size());
    //    for(size_t i=0; i<edges.size(); ++i)
    //    {
    //        glm::vec2 &pt1=points[edges[i].v];
    //        glm::vec2 &pt2=points[edges[i].w];
    //
    //        max=std::max(pt1.x, max);
    //        max=std::max(pt1.y, max);
    //        max=std::max(pt2.x, max);
    //        max=std::max(pt2.y, max);
    //
    //        min=std::min(pt1.x, min);
    //        min=std::min(pt1.y, min);
    //        min=std::min(pt2.x, min);
    //        min=std::min(pt2.y, min);
    //
    //        m_influenceLines[i].push_back(pt1);
    //        m_influenceLines[i].push_back(pt2);
    //    }
    //
    //    float delta=max-min;
    //    //set points based on size
    //    for(size_t i=0; i<m_influenceLines.size(); ++i)
    //    {
    //        for(auto &point:m_influenceLines[i])
    //        {
    //            point.x=((point.x-min)/delta)*influenceSize.x;
    //            point.y=((point.y-min)/delta)*influenceSize.y;
    //        }
    //    }
}

#else

struct PlateInfo
{
    size_t index;
    float value;
    glm::vec3 direction;

    float height;

    glm::ivec2 point;
    glm::vec3 point3d;
    std::vector<glm::ivec2> points;

    std::vector<size_t> neighbors;
    std::vector<float> neighborCollisions;
};

void calculateCurve(float distance, float &plate1, float &plate2, float cutoff=0.7f);
float calculateConvergentCurve(float distance, bool plate1Ocean, bool plate2Ocean);
float calculateDivergentCurve(float distance, bool plate1Ocean, bool plate2Ocean);

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::generatePlates()
{
    chrono::high_resolution_clock::time_point time1;
    chrono::high_resolution_clock::time_point time2;

    glm::ivec2 influenceSize=m_descriptorValues.m_influenceSize;
    int influenceMapSize=HastyNoise::AlignedSize(influenceSize.x*influenceSize.y, m_simdLevel);
    glm::ivec2 size=m_descriptors->getSize();

    std::vector<float> plateMap;
    std::vector<float> plate2Map;
    std::vector<float> plateDistanceMap;
    std::vector<float> continentMap;
    std::vector<float> heightMap;
    std::vector<float> terrainScaleMap;
    std::vector<PlateInfo> plateDetails;

    time1=chrono::high_resolution_clock::now();

    plateMap.resize(influenceMapSize);
    plate2Map.resize(influenceMapSize);
    plateDistanceMap.resize(influenceMapSize);
    continentMap.resize(influenceMapSize);
    m_influenceMap.resize(influenceMapSize);
    heightMap.resize(influenceMapSize);
    terrainScaleMap.resize(influenceMapSize);

    m_influenceVectorSet=std::make_unique<HastyNoise::VectorSet>(m_simdLevel);
    m_influenceVectorSet->SetSize(influenceMapSize);

    time2=chrono::high_resolution_clock::now();
    allocationTime=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

    glm::vec3 mapPos;
    size_t index=0;
    mapPos.z=(float)(influenceSize.x/2.0f);
    for(int y=0; y<influenceSize.y; y++)
    {
        mapPos.y=y;
        for(int x=0; x<influenceSize.x; x++)
        {
            mapPos.x=x;
            glm::vec3 pos=getSphericalCoords(influenceSize.x, influenceSize.y, mapPos);

            //hasty treats x and y in reverse, need to change
            m_influenceVectorSet->xSet[index]=pos.y;
            m_influenceVectorSet->ySet[index]=pos.x;
            m_influenceVectorSet->zSet[index]=pos.z;
            index++;
        }
    }

    time1=chrono::high_resolution_clock::now();
    coordsTime=chrono::duration_cast<chrono::milliseconds>(time1-time2).count();

//height map
    m_continentPerlin->SetNoiseType(HastyNoise::NoiseType::PerlinFractal);
//    m_continentPerlin->SetFrequency(m_descriptorValues.m_continentFrequency);
//    m_continentPerlin->SetFractalLacunarity(m_descriptorValues.m_continentLacunarity);
//    m_continentPerlin->SetFractalOctaves(m_descriptorValues.m_continentOctaves);
    m_continentPerlin->SetFrequency(0.01f);
    m_continentPerlin->SetFractalLacunarity(2.0f);
    m_continentPerlin->SetFractalOctaves(4);
    m_continentPerlin->FillSet(heightMap.data(), m_influenceVectorSet.get());

    m_continentPerlin->SetFrequency(0.05f);
    m_continentPerlin->SetFractalLacunarity(2.0f);
    m_continentPerlin->SetFractalOctaves(4);
    m_continentPerlin->FillSet(terrainScaleMap.data(), m_influenceVectorSet.get());

//plate map
    m_cellularNoise->SetCellularReturnType(HastyNoise::CellularReturnType::Value);
    m_cellularNoise->SetCellularDistanceFunction(HastyNoise::CellularDistance::Natural);
    m_cellularNoise->SetSeed(m_plateSeed);
    m_cellularNoise->SetFrequency(m_descriptorValues.m_plateFrequency);
    m_cellularNoise->SetFractalLacunarity(m_descriptorValues.m_plateLacunarity);
    m_cellularNoise->SetFractalOctaves(m_descriptorValues.m_plateOctaves);

    m_cellularNoise->SetPerturbType(HastyNoise::PerturbType::GradientFractal);
    m_cellularNoise->SetPerturbAmp(0.5f);
    m_cellularNoise->SetPerturbFrequency(1.0f);
    m_cellularNoise->SetPerturbFractalOctaves(5);
    m_cellularNoise->SetPerturbFractalLacunarity(2.0f);
    m_cellularNoise->SetPerturbFractalGain(0.5f);
    m_cellularNoise->SetPerturbNormaliseLength(1.0f);

    m_cellularNoise->FillSet(plateMap.data(), m_influenceVectorSet.get());

    time2=chrono::high_resolution_clock::now();
    cellularPlateTime=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

//Distance Map
    //going to generate twice as I want the distance value as well, will mod HastyNoise later to produce both (as it has already done the work)
    m_cellularNoise->SetCellularReturnType(HastyNoise::CellularReturnType::Distance2Div);//gives distance to border
    m_cellularNoise->FillSet(plateDistanceMap.data(), m_influenceVectorSet.get());

    time1=chrono::high_resolution_clock::now();
    cellularDistanceTime=chrono::duration_cast<chrono::milliseconds>(time1-time2).count();

//Boarder plate Map
    m_cellularNoise->SetCellularReturnType(HastyNoise::CellularReturnType::ValueDistance2);//get what plate the div was tested against
    m_cellularNoise->FillSet(plate2Map.data(), m_influenceVectorSet.get());

    time2=chrono::high_resolution_clock::now();
    cellularPlate2Time=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

//continent map
    m_cellularNoise->SetCellularReturnType(HastyNoise::CellularReturnType::Value);
    m_cellularNoise->SetSeed(m_continentSeed);
    m_cellularNoise->SetFrequency(m_descriptorValues.m_continentFrequency);
    m_cellularNoise->FillSet(continentMap.data(), m_influenceVectorSet.get());

    time1=chrono::high_resolution_clock::now();

    float last=-2.0f;
    std::vector<float> plates;

    //find and index all plates
    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(plateMap[i]==last)
            continue;
        else if(contains_sorted(plates, plateMap[i]))
        {
            last=plateMap[i];
            continue;
        }

        insert_sorted(plates, plateMap[i]);
        last=plateMap[i];
    }

    last=-2.0f;
    size_t lastIndex=0;
    float last2=-2.0f;
    size_t last2Index=0;
    glm::ivec2 point={0, 0};

    //setup plates
    std::default_random_engine generator(m_descriptors->m_seed);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distributionNorm(0.0f, 1.0f);
    std::vector<float> jitter;
    std::vector<float> plateMinDistance;
    std::vector<float> plateMaxDistance;
    std::vector<glm::ivec2> plateMinPoint;
    std::vector<int> plateNeighbors;
    std::vector<int> plateCollisions;

    plateDetails.resize(plates.size());
    plateNeighbors.resize(plates.size()*plates.size(), 0);
    plateMinDistance.resize(plates.size(), 2.0f);
    plateMaxDistance.resize(plates.size(), -2.0f);
    plateMinPoint.resize(plates.size());

    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(plateMap[i]!=last)
        {
            lastIndex=index_sorted(plates, plateMap[i]);
            last=plateMap[i];
        }

        if(plate2Map[i]!=last2)
        {
            last2Index=index_sorted(plates, plate2Map[i]);

            if(last2Index==std::numeric_limits<size_t>::max())
            {
                //plate we haven't seen, setting to current plate
                last2Index=lastIndex;
                last2=plateMap[i];
            }
            else
                last2=plate2Map[i];
        }

        if(lastIndex!=last2Index)
        {
            size_t neighborIndex=lastIndex*plates.size()+last2Index;

            if(!plateNeighbors[neighborIndex])
                plateNeighbors[neighborIndex]=1;
        }

        if(plateDistanceMap[i]<plateMinDistance[lastIndex])
        {
            plateMinDistance[lastIndex]=plateDistanceMap[i];
            plateMinPoint[lastIndex]=point;
        }

        if(plateDistanceMap[i]>plateMaxDistance[lastIndex])
            plateMaxDistance[lastIndex]=plateDistanceMap[i];

        assert(last2Index<plates.size());

        m_influenceMap[i].tectonicPlate=lastIndex;
        m_influenceMap[i].borderPlate=last2Index;
        m_influenceMap[i].plateHeight=(last)*0.1f+0.5f;
        m_influenceMap[i].plateValue=last;
        m_influenceMap[i].plateDistanceValue=plateDistanceMap[i];
        m_influenceMap[i].continentValue=continentMap[i];

        plateDetails[lastIndex].points.emplace_back(point.x, point.y);

        point.x++;
        if(point.x>=influenceSize.x)
        {
            point.x=0;
            point.y++;
        }
    }

    glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

    for(size_t i=0; i<plateDetails.size(); i++)
    {
        PlateInfo &details=plateDetails[i];

        details.value=plates[i];
        details.point=plateMinPoint[i];

        glm::vec2 mapPoint;
        glm::vec3 point;

        //we want plate heights to be 0.5 to -0.5
//        details.height=(distributionNorm(generator))*0.3f+0.3f;
        details.height=(plates[i])*0.1f+0.5f;
        if(details.height<0.5f)
            details.height=details.height-0.05f;

        //normalize map axis
        mapPoint.x=(float)details.point.x/(float)influenceSize.x;
        mapPoint.y=(float)details.point.y/(float)influenceSize.y;

        projectPoint<Projections::Equirectangular, Projections::Cartesian>(mapPoint, point);
        details.point3d=point;

        glm::vec3 direction;
        glm::vec2 tangentPlaneCoords;

        //generate drift vector, building on x/y plane that will be rotate to the point
        //tanget to the sphere
        tangentPlaneCoords.x=distribution(generator);
        tangentPlaneCoords.y=distribution(generator);
        tangentPlaneCoords=glm::normalize(tangentPlaneCoords);

        //cross with z axis for a rotation vector to align with the z axis
        glm::vec3 rotationVector=glm::cross(zAxis, point);

        if(glm::length2(rotationVector)==0.0f)
        {//already aligned to z axis
            details.direction=glm::vec3(tangentPlaneCoords.x, tangentPlaneCoords.y, 0.0f);
        }
        else
        {
            //get angle to rotate vector to position
            float theta=acos(glm::dot(point, zAxis));
            glm::mat4 rotationMat=glm::rotate(theta, rotationVector);

            details.direction=rotationMat*glm::vec4(tangentPlaneCoords.x, tangentPlaneCoords.y, 0.0f, 1.0f);
        }

        size_t neighborIndex=i*plateDetails.size();
        for(size_t j=0; j<plateDetails.size(); j++)
        {
            if(plateNeighbors[neighborIndex+j])
                details.neighbors.push_back(j);
        }
    }

    const float invsqrt2=0.5f/sqrt(2.0f);

    for(size_t i=0; i<plateDetails.size(); i++)
    {
        PlateInfo &details=plateDetails[i];
        size_t neighborIndex=plateDetails.size()*i;

        for(size_t j=0; j<details.neighbors.size(); j++)
        {
            //this is working on the vector in between the center points, it is actually be better to rotate the 
            //vector to the point being worked on and solve from there. Skipping shear for the moment.
            PlateInfo &details2=plateDetails[details.neighbors[j]];

            glm::vec3 collisionVector=details2.point3d-details.point3d;

            glm::vec3 direction=glm::proj(details.direction, collisionVector);
            glm::vec3 direction2=glm::proj(details2.direction, collisionVector);
            
            float pointDistance=glm::distance(details2.point3d, details.point3d);
            float directionDistance=glm::distance((details2.point3d+direction2), (details.point3d+direction));

            float collision=(pointDistance-directionDistance)*invsqrt2; //scale to 0.0 to 1.0

            assert(collision<=1.0f);
            collision=std::min(collision, 1.0f);//make sure between 0.0 and 1.0
            details.neighborCollisions.push_back(collision);
        }
    }

    for(size_t i=0; i<influenceMapSize; i++)
    {
        size_t &index=m_influenceMap[i].tectonicPlate;
        size_t &borderIndex=m_influenceMap[i].borderPlate;

        PlateInfo &details=plateDetails[index];
        PlateInfo &details2=plateDetails[borderIndex];

        size_t neighborIndex=0;

        for(size_t j=0; j<details.neighbors.size(); ++j)
        {
            if(details.neighbors[j]==borderIndex)
                neighborIndex=j;
        }

        float collision=details.neighborCollisions[neighborIndex];

        //normalize distance
        m_influenceMap[i].plateDistanceValue=(plateDistanceMap[i]-plateMinDistance[index])/(plateMaxDistance[index]-plateMinDistance[index]);
//        m_influenceMap[i].heightBase=0.0f;

        bool oceanPlate=(details.height<0.5f);
        bool oceanPlate2=(details2.height<0.5f);

        float plateScale;
        float plate2Scale;
        float terrainScale=0.0f;

		if((oceanPlate && !oceanPlate2) || (!oceanPlate&&oceanPlate2))
			calculateCurve(m_influenceMap[i].plateDistanceValue, plateScale, plate2Scale, 0.7f);
		else
			calculateCurve(m_influenceMap[i].plateDistanceValue, plateScale, plate2Scale, 0.5f);
        
//        if(i>51450)
//            i=i;

        if(index != borderIndex)
        {
            m_influenceMap[i].collision=collision;

            if(collision<0.0f) //divergent boundary
            {
                collision=-(collision);//reverse negative as following is expecting collision to be a magnitude
                terrainScale=calculateDivergentCurve(m_influenceMap[i].plateDistanceValue, oceanPlate, oceanPlate2);
            }
            else if(collision>0.0f) //convergent boundary
                terrainScale=calculateConvergentCurve(m_influenceMap[i].plateDistanceValue, oceanPlate, oceanPlate2);
        }
        else
            m_influenceMap[i].collision=0.0f;

        float genHeight=(heightMap[i]+1.0f)*0.05f;
        float genTerrainScale=(terrainScaleMap[i]+1.0f)*0.2f+0.2f;

        m_influenceMap[i].heightBase=((details.height+genHeight)*plateScale)+(details2.height*plate2Scale)+(terrainScale*collision*genTerrainScale);// *0.4f);
        
        m_influenceMap[i].terrainScale=terrainScale;

		if(m_influenceMap[i].heightBase>1.0f)
			m_influenceMap[i].heightBase=1.0f;
		if(m_influenceMap[i].heightBase<0.0f)
			m_influenceMap[i].heightBase=0.0f;
    }

//	for(size_t y=1; i<influenceSize.y; y++)
//	{
//
//	}

    time2=chrono::high_resolution_clock::now();
    processingTime=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

    m_plateCount=plates.size();
}
#endif 

inline void calculateCurve(float distance, float &plate1, float &plate2, float cutoff)
{
    if(distance>cutoff)
    {
        float d=(distance-cutoff)*(1.0f/(1.0f-cutoff));
        plate2=d*0.5f;
//        plate2=(d*d)*0.5f;
        plate1=1.0f-plate2;
    }
    else
    {
        plate1=1.0f;
        plate2=0.0f;
    }
}

inline float subductionCurve(float distance)
{
    if(distance>0.9f)
    {
        distance=(distance-0.9f)*10.0f;
        return (1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)))*0.5f+0.5f;
    }
    else if(distance>0.8f)
    {
        distance=(distance-0.8f)*10.0f;
        return (1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)))*0.3f+0.2f;
    }
    if(distance>0.6f)
    {
        distance=(distance-0.6f)*5.0f;
        return (distance*distance)*0.2f;
    }
    return 0.0f;
}

inline float orogenicCurve(float distance)
{
    if(distance>0.6f)
    {
        distance=(distance-0.6f)*2.5f;
		return 1.0f/(1.0f+pow(3.0f*(1.0f-distance)/distance, 2.0f));
//        return (distance*distance);
    }
    return 0.0f;
}

inline float divergentCurve(float distance, float cutoff=0.9)
{
    if(distance>cutoff)
    {
        distance=(distance-cutoff)/(1.0f-cutoff);
        return -(1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)));
    }
    return 0.0f;
}

inline float calculateConvergentCurve(float distance, bool plate1Ocean, bool plate2Ocean)
{
	if(plate1Ocean && plate2Ocean)
		return orogenicCurve(distance)*0.5f;
    else if(plate1Ocean&&!plate2Ocean)
        return subductionCurve(distance)*0.7f;
    else if(!plate1Ocean&&plate2Ocean)
        return orogenicCurve(distance)*0.7f;
    return orogenicCurve(distance);
}

inline float calculateDivergentCurve(float distance, bool plate1Ocean, bool plate2Ocean)
{
    if(plate1Ocean && plate2Ocean)
        return divergentCurve(distance, 0.7f)*0.5f;
    else if(plate1Ocean&&!plate2Ocean)
        return divergentCurve(distance);
    else if(!plate1Ocean&&plate2Ocean)
        return divergentCurve(distance, 0.7f);
    return divergentCurve(distance);
}

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::generateContinents()
{
//    m_cellularNoise->SetFrequency(m_descriptorValues.m_continentFrequency);
//    m_cellularNoise->SetFractalLacunarity(m_descriptorValues.m_plateLacunarity);
//    m_cellularNoise->SetFractalOctaves(m_descriptorValues.m_plateOctaves);
}

template<bool useStride>
int getBlockType(int z, size_t columnHeight, size_t stride)
{
    int blockType=1;// =(columnHeight-z)/8+2;

    if(columnHeight>10)
        blockType=3;
    if(columnHeight>2)
        blockType=2;

    return blockType;
}

template<>
inline int getBlockType<true>(int z, size_t columnHeight, size_t stride)
{
    int blockType=1;

//    for(int z=0; z<stride; ++z)
//    {
//        for(int y=0; y<stride; ++y)
//        {
//            for(int y=0; y<stride; ++y)
//            {
//
//            }
//        }
//    }
    if(columnHeight>10)
        blockType=3;
    if(columnHeight>2)
        blockType=2;

    return blockType;
}

template<typename _Grid>
unsigned int EquiRectWorldGenerator<_Grid>::generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod)
{
//    glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;
    glm::vec3 scaledOffset=startPos*m_descriptorValues.m_noiseScale;
    glm::vec3 position=scaledOffset;
        
//    UniqueChunkType chunk=std::make_unique<ChunkType>(hash, 0, chunkIndex, startPos);
//    ChunkType::Cells &cells=chunk->getCells();
    
    typename ChunkType::CellType *cells=(typename ChunkType::CellType *)buffer;
    size_t stride=glm::pow(2u, (unsigned int)lod);
    glm::ivec3 lodChunkSize=chunkSize/(int)stride;
    float noiseScale=m_descriptorValues.m_noiseScale/stride;

    //verify chunkSize matches template chunk size
    assert(chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
    //verify buffer is large enough for data
    //assert(bufferSize>=(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value)*sizeof(ChunkType::CellType));
    assert(bufferSize>=(lodChunkSize.x*lodChunkSize.y*lodChunkSize.z)*sizeof(typename ChunkType::CellType));

//    int heightMapSize=HastyAlignedSize(lodChunkSize.x*lodChunkSize.y, m_simdLevel);
//
////    std::vector<float> heightMap(heightMapSize);
////    std::vector<float> xMap(heightMapSize);
////    std::vector<float> yMap(heightMapSize);
////    std::vector<float> zMap(heightMapSize);
//    heightMap.resize(heightMapSize);
//    xMap.resize(heightMapSize);
//    yMap.resize(heightMapSize);
//    zMap.resize(heightMapSize);
//
//    size_t index=0;
//    glm::vec3 mapPos;
//    glm::ivec3 size=m_descriptors->m_size;
//
//    mapPos.z=(float)size.x/2.0f;
//    for(int y=0; y<ChunkType::sizeY::value; y+=stride)
//    {
//        mapPos.y=startPos.y+y;
//        for(int x=0; x<ChunkType::sizeX::value; x+=stride)
//        {
//            mapPos.x=startPos.x+x;
//
//            glm::vec3 pos=getCylindricalCoords(size.x, size.y, mapPos);
//
////            pos*=m_descriptors.noiseScale;
//            xMap[index]=pos.x;
//            yMap[index]=pos.y;
//            zMap[index]=pos.z;
//            index++;
//        }
//    }
//
//    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), lodChunkSize.x, lodChunkSize.y, 1);
    buildHeightMap(startPos, lodChunkSize, stride);

    int chunkMapSize=HastyNoise::AlignedSize(lodChunkSize.x*lodChunkSize.y*lodChunkSize.z, m_simdLevel);

//    std::vector<float> layerMap(chunkMapSize);
    layerMap.resize(chunkMapSize);

//    m_layersPerlin->FillNoiseSet(layerMap.data(), offset.x, offset.y, offset.z, _Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value);

    glm::ivec3 size=m_descriptors->m_size;
    int seaLevel=(size.z/2);
    float heightScale=((float)size.z/2.0f);
    unsigned int validCells=0;
//    glm::ivec3 blockIndex;

    size_t index=0;
    size_t heightIndex=0;
    position.z=scaledOffset.z;
    for(int z=0; z<ChunkType::sizeZ::value; z+=stride)
    {
        heightIndex=0;
        position.y=scaledOffset.y;

        int blockZ=(int)startPos.z+z;
        for(int y=0; y<ChunkType::sizeY::value; y+=stride)
        {
            position.x=scaledOffset.x;
            for(int x=0; x<ChunkType::sizeX::value; x+=stride)
            {
                unsigned int blockType;
                int blockHeight=(int)(heightMap[heightIndex]*heightScale)+seaLevel;

//                if(position.z > heightMap[heightIndex]) //larger than height map, air
                if(blockZ>blockHeight)
                    blockType=0;
                else
                {
                    if(stride == 1)
                        blockType=getBlockType<false>(blockZ, blockHeight-blockZ, stride);
                    else
                        blockType=getBlockType<true>(blockZ, blockHeight-blockZ, stride);

//                    if(blockZ<seaLevel)
//                        blockType=1;
//                    blockType=(blockHeight-blockZ)/13+2;
                    if(blockType!=0)
                        validCells++;
                }

                type(cells[index])=blockType;
                index++;
                heightIndex++;
                position.x+=noiseScale;
            }
            position.y+=noiseScale;
        }
        position.z+=noiseScale;
    }

    return validCells;
//    chunk->setValidCellCount(validCells);
//    return chunk;
}

template<typename _Grid>
unsigned int EquiRectWorldGenerator<_Grid>::generateRegion(const glm::vec3 &startPos, const glm::ivec3 &regionSize, void *buffer, size_t bufferSize, size_t lod)
{
    size_t stride=glm::pow(2u, (unsigned int)lod);
    glm::ivec2 lodSize=glm::ivec2(regionSize.x, regionSize.y)/(int)stride;

    typename _Grid::RegionHandleType::Cell *cells=(typename _Grid::RegionHandleType::Cell *)buffer;
    assert(bufferSize>=(lodSize.x*lodSize.y)*sizeof(typename _Grid::RegionHandleType::Cell));

    int heightMapSize=HastyNoise::AlignedSize(lodSize.x*lodSize.y, m_simdLevel);

    regionHeightMap.resize(heightMapSize);
    regionVectorSet->SetSize(heightMapSize);

    size_t index=0;
    glm::vec3 mapPos;
    glm::ivec3 size=m_descriptors->m_size;
    float heightScale=((float)size.z/2.0f);
    int seaLevel=(size.z/2);
    unsigned int validCells=0;

    mapPos.z=(float)size.x/2.0f;
    for(int y=0; y<regionSize.y; y+=stride)
    {
        mapPos.y=startPos.y+y;
        for(int x=0; x<regionSize.x; x+=stride)
        {
            mapPos.x=startPos.x+x;

            glm::vec3 pos=getCylindricalCoords(size.x, size.y, mapPos);

            regionVectorSet->xSet[index]=pos.x;
            regionVectorSet->ySet[index]=pos.y;
            regionVectorSet->zSet[index]=pos.z;
            index++;
        }
    }

    m_heightmapNoise->FillSet(regionHeightMap.data(), regionVectorSet.get());

    index=0;
    for(int y=0; y<regionSize.y; y+=stride)
    {
        for(int x=0; x<regionSize.x; x+=stride)
        {
            unsigned int blockType;
            int blockHeight=(int)(regionHeightMap[index]*heightScale)+seaLevel;

            if((blockHeight<startPos.z)||(blockHeight>startPos.z+regionSize.z))
                blockType=0;
            else
            {
                if(stride==1)
                    blockType=getBlockType<false>(blockHeight, 0, stride);
                else
                    blockType=getBlockType<true>(blockHeight, 0, stride);
            }

            if(blockType!=0)
                validCells++;

            type(cells[index])=blockType;
            height(cells[index])=blockHeight;

            index++;
        }
    }

    return validCells;
}

//template<typename _Grid>
//unsigned int EquiRectWorldGenerator<_Grid>::generateHeightMap(const glm::vec3 &startPos, const glm::ivec3 &regionSize, void *buffer, size_t bufferSize, size_t lod)
//{
//    glm::vec3 scaledOffset=startPos*m_descriptorValues.m_noiseScale;
//    glm::vec3 position=scaledOffset;
//
//    HeightMapCell *cells=(HeightMapCell *)buffer;
//
//    size_t stride=glm::pow(2u, (unsigned int)lod);
//    glm::ivec3 lodSize=regionSize/(int)stride;
//    float noiseScale=m_descriptorValues.m_noiseScale/stride;
//
//    //verify chunkSize matches template chunk size
//    assert(chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
//
//    //verify buffer is large enough for data
//    assert(bufferSize>=(lodSize.x*lodSize.y*lodSize.z)*sizeof(HeightMapCell));
//
//    buildHeightMap(startPos, lodSize, stride);
//
//    size_t index=0;
//    int heightIndex=0;
//    position.y=scaledOffset.y;
//
//    int blockZ=(int)startPos.z+z;
//    for(int y=0; y<ChunkType::sizeY::value; y+=stride)
//    {
//        position.x=scaledOffset.x;
//        for(int x=0; x<ChunkType::sizeX::value; x+=stride)
//        {
//            unsigned int blockType;
//            int blockHeight=(int)(heightMap[heightIndex]*heightScale)+seaLevel;
//
//            if(stride==1)
//                blockType=getBlockType<false>(blockHeight, 0, stride);
//            else
//                blockType=getBlockType<true>(blockHeight, 0, stride);
//
//            if(blockType!=0)
//                validCells++;
//
//            cells[index].type=blockType;
//            cells[index].height=blockHeight;
//
//            index++;
//            heightIndex++;
//            position.x+=noiseScale;
//        }
//        position.y+=noiseScale;
//    }
//}

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::buildHeightMap(const glm::vec3 &startPos, const glm::ivec3 &lodSize, size_t stride)
{
    int heightMapSize=HastyNoise::AlignedSize(lodSize.x*lodSize.y, m_simdLevel);

    heightMap.resize(heightMapSize);
    xMap.resize(heightMapSize);
    yMap.resize(heightMapSize);
    zMap.resize(heightMapSize);

    vectorSet->SetSize(heightMapSize);

    size_t index=0;
    glm::vec3 mapPos;
    glm::ivec3 size=m_descriptors->m_size;

    mapPos.z=(float)size.x/2.0f;
    for(int y=0; y<ChunkType::sizeY::value; y+=stride)
    {
        mapPos.y=startPos.y+y;
        for(int x=0; x<ChunkType::sizeX::value; x+=stride)
        {
            mapPos.x=startPos.x+x;

            glm::vec3 pos=getCylindricalCoords(size.x, size.y, mapPos);

            vectorSet->xSet[index]=pos.x;
            vectorSet->ySet[index]=pos.y;
            vectorSet->zSet[index]=pos.z;
            index++;
        }
    }

    m_heightmapNoise->FillSet(heightMap.data(), vectorSet.get());
//    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), lodSize.x, lodSize.y, 1);
}

//template<typename _Grid>
//void EquiRectWorldGenerator<_Grid>::generate()
//{
//
//}

}//namespace voxigen

#endif //_voxigen_equiRectEquiRectWorldGenerator_h_