#ifndef _voxigen_equiRectWorldGenerator_h_
#define _voxigen_equiRectWorldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/coords.h"
#include "voxigen/heightMap.h"
#include "voxigen/noise.h"

#undef None

#include <random>

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
        [](const T &l, const T &r){ return l < r; });
    return iter!=container.end()&&*iter==item;
}

template<class T>
size_t index_sorted(const std::vector<T> &container, const T &item)
{
    auto iter=std::lower_bound(
        container.begin(),
        container.end(),
        item,
        [](const T &l, const T &r){ return l < r; });
    
    if(iter!=container.end()&&*iter==item)
        return iter-container.begin();
    return std::numeric_limits<size_t>::max();
}

struct VOXIGEN_EXPORT InfluenceCell
{
    float heightBase;
    float heightRangs;

    int tectonicPlate;
    float plateValue;
    float plateDistanceValue;
    float continentValue;

    float airCurrent;
};

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

        m_plateCountMin=8;
        m_plateCountMax=24;

        m_plateFrequency=0.0005f;
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

        m_plateFrequency=5.12f/m_influenceSize.x;
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

    static const char *typeName(){return "EquiRectWorldGenerator";}

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

private:
    void buildHeightMap(const glm::vec3 &startPos, const glm::ivec3 &lodSize, size_t stride);

    void generatePlates();
    void generateContinents();

    GridDescriptors<_Grid> *m_descriptors;
    EquiRectDescriptors m_descriptorValues;
    
    size_t m_simdLevel;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentPerlin;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_layersPerlin;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_cellularNoise;
    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentCellular;

    int m_plateSeed;
    int m_continentSeed;

    int m_plateCount;
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
    initNoise();
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

    m_plateCount=plateDistribution(generator);

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
    generatePlates();
    generateContinents();
}

template<typename _Grid>
void EquiRectWorldGenerator<_Grid>::generatePlates()
{
    glm::ivec2 influenceSize=m_descriptorValues.m_influenceSize;
    int influenceMapSize=HastyNoise::AlignedSize(influenceSize.x*influenceSize.y, m_simdLevel);
    glm::ivec2 size=m_descriptors->getSize();

    std::vector<float> plateMap;
    std::vector<float> plateDistanceMap;
    std::vector<float> continentMap;

    plateMap.resize(influenceMapSize);
    plateDistanceMap.resize(influenceMapSize);
    continentMap.resize(influenceMapSize);
    m_influenceMap.resize(influenceMapSize);

    m_influenceVectorSet=std::make_unique<HastyNoise::VectorSet>(m_simdLevel);
    m_influenceVectorSet->SetSize(influenceMapSize);

    glm::vec3 mapPos;
    size_t index=0;
    mapPos.z=(float)(influenceSize.x/2.0f);
    for(int y=0; y<influenceSize.y; y++)
    {
        mapPos.y=y;
        for(int x=0; x<influenceSize.x; x++)
        {
            mapPos.x=x;
            glm::vec3 pos=getSphericalcalCoords(influenceSize.x, influenceSize.y, mapPos);

            //hasty treats x and y in reverse, need to change
            m_influenceVectorSet->xSet[index]=pos.y;
            m_influenceVectorSet->ySet[index]=pos.x;
            m_influenceVectorSet->zSet[index]=pos.z;
            index++;
        }
    }

    m_cellularNoise->SetCellularReturnType(HastyNoise::CellularReturnType::Value);
    m_cellularNoise->SetCellularDistanceFunction(HastyNoise::CellularDistance::Natural);
    m_cellularNoise->SetSeed(m_plateSeed);
    m_cellularNoise->SetFrequency(m_descriptorValues.m_plateFrequency);
    m_cellularNoise->SetFractalLacunarity(m_descriptorValues.m_plateLacunarity);
    m_cellularNoise->SetFractalOctaves(m_descriptorValues.m_plateOctaves);
    m_cellularNoise->FillSet(plateMap.data(), m_influenceVectorSet.get());
    
    //going to generate twice as I want the distance value as well, will mod HastyNoise later to produce both (as it has already done the work)
    m_cellularNoise->SetCellularReturnType(HastyNoise::CellularReturnType::Distance);
    m_cellularNoise->FillSet(plateDistanceMap.data(), m_influenceVectorSet.get());

    //continent map
    m_cellularNoise->SetCellularReturnType(HastyNoise::CellularReturnType::Value);
    m_cellularNoise->SetSeed(m_continentSeed);
    m_cellularNoise->SetFrequency(m_descriptorValues.m_continentFrequency);
    m_cellularNoise->FillSet(continentMap.data(), m_influenceVectorSet.get());

    float last=-2.0f;
    std::vector<float> plates;

    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(plateMap[i]==last)
            continue;
        if(contains_sorted(plates, plateMap[i]))
            continue;

        insert_sorted(plates, plateMap[i]);
        last=plateMap[i];
    }

    last=-2.0f;
    size_t lastIndex=0;
    
    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(plateMap[i]==last)
            m_influenceMap[i].tectonicPlate=lastIndex;
        else
        {
            size_t index=index_sorted(plates, plateMap[i]);
            assert(index!=std::numeric_limits<size_t>::max());
            m_influenceMap[i].tectonicPlate=index;

            last=plateMap[i];
            lastIndex=index;
        }
        m_influenceMap[i].plateValue=plateMap[i];
        m_influenceMap[i].plateDistanceValue=plateDistanceMap[i];
        m_influenceMap[i].continentValue=continentMap[i];
    }

    m_plateCount=plates.size();
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
    size_t stride=glm::pow(2, lod);
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
    glm::ivec3 blockIndex;

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
    size_t stride=glm::pow(2, lod);
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

    m_continentPerlin->FillSet(regionHeightMap.data(), regionVectorSet.get());

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
//    size_t stride=glm::pow(2, lod);
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

    m_continentPerlin->FillSet(heightMap.data(), vectorSet.get());
//    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), lodSize.x, lodSize.y, 1);
}


}//namespace voxigen

#endif //_voxigen_equiRectEquiRectWorldGenerator_h_