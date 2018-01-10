#ifndef _voxigen_equiRectWorldGenerator_h_
#define _voxigen_equiRectWorldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/coords.h"

//#include <noise/noise.h>
#include <FastNoiseSIMD.h>

namespace voxigen
{

struct VOXIGEN_EXPORT EquiRectDescriptors
{
    EquiRectDescriptors()
    {
        m_noiseScale=0.001;

        //    contientFrequency=1.0;
        m_contientFrequency=0.005;
        m_contientOctaves=2;
        m_contientLacunarity=2.2;

        m_seaLevel=0.0f;
    }

    bool load(const char *json);
    bool save(char *json, size_t &size);

    float m_noiseScale;
    float m_contientFrequency;
    int m_contientOctaves;
    float m_contientLacunarity;

    float m_seaLevel;
    float m_continentaShelf;
};

template<typename _Grid>
class EquiRectWorldGenerator
{
public:
    typedef typename _Grid::ChunkType ChunkType;
    typedef std::unique_ptr<ChunkType> UniqueChunkType;

    EquiRectWorldGenerator();
    ~EquiRectWorldGenerator();

    static char *typeName(){return "EquiRectWorldGenerator";}

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
    unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize);

private:
    GridDescriptors<_Grid> *m_descriptors;
    EquiRectDescriptors m_descriptorValues;

    std::unique_ptr<FastNoiseSIMD> m_continentPerlin;
    std::unique_ptr<FastNoiseSIMD> m_layersPerlin;

//    noise::module::Perlin m_perlin;
//    noise::module::Perlin m_continentPerlin;
//    noise::module::Curve m_continentCurve;
//
//    noise::module::Perlin m_layersPerlin;
};

template<typename _Grid>
EquiRectWorldGenerator<_Grid>::EquiRectWorldGenerator()
{
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

//    m_descriptors=descriptors;
//    assert(m_descriptors!=nullptr);

    assert(m_descriptors->m_chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
    int seed=m_descriptors->m_seed;

    m_continentPerlin.reset(FastNoiseSIMD::NewFastNoiseSIMD(seed));

    m_continentPerlin->SetNoiseType(FastNoiseSIMD::PerlinFractal);
    m_continentPerlin->SetFrequency(m_descriptorValues.m_contientFrequency);
    m_continentPerlin->SetFractalLacunarity(m_descriptorValues.m_contientLacunarity);
    m_continentPerlin->SetFractalOctaves(m_descriptorValues.m_contientOctaves);


    m_layersPerlin.reset(FastNoiseSIMD::NewFastNoiseSIMD(seed+1));

    m_layersPerlin->SetNoiseType(FastNoiseSIMD::PerlinFractal);
    m_layersPerlin->SetFrequency(m_descriptorValues.m_contientFrequency);
    m_layersPerlin->SetFractalLacunarity(m_descriptorValues.m_contientLacunarity);
    m_layersPerlin->SetFractalOctaves(m_descriptorValues.m_contientOctaves);
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

}

template<typename _Grid>
unsigned int EquiRectWorldGenerator<_Grid>::generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize)
{
//    glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;
    glm::vec3 scaledOffset=startPos*m_descriptorValues.m_noiseScale;
    glm::vec3 position=scaledOffset;
    float noiseScale=m_descriptorValues.m_noiseScale;
    
   
//    UniqueChunkType chunk=std::make_unique<ChunkType>(hash, 0, chunkIndex, startPos);
//    ChunkType::Cells &cells=chunk->getCells();
    
    ChunkType::CellType *cells=(ChunkType::CellType *)buffer;

    //verify chunkSize matches template chunk size
    assert(chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
    //verify buffer is large enough for data
    assert(bufferSize>=(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value)*sizeof(ChunkType::CellType));

    int heightMapSize=FastNoiseSIMD::AlignedSize(ChunkType::sizeX::value*ChunkType::sizeY::value);
    std::vector<float> heightMap(heightMapSize);
    std::vector<float> xMap(heightMapSize);
    std::vector<float> yMap(heightMapSize);
    std::vector<float> zMap(heightMapSize);

    size_t index=0;
    glm::vec3 mapPos;
    glm::ivec3 size=m_descriptors->m_size;

    mapPos.z=(float)size.x/2.0f;
    for(int y=0; y<chunkSize.y; ++y)
    {
        mapPos.y=startPos.y+y;
        for(int x=0; x<ChunkType::sizeX::value; ++x)
        {
            mapPos.x=startPos.x+x;

            glm::vec3 pos=getCylindricalCoords(size.x, size.y, mapPos);

//            pos*=m_descriptors.noiseScale;
            xMap[index]=pos.x;
            yMap[index]=pos.y;
            zMap[index]=pos.z;
            index++;
        }
    }

    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), ChunkType::sizeX::value, ChunkType::sizeY::value, 1);

    int chunkMapSize=FastNoiseSIMD::AlignedSize(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value);

    std::vector<float> layerMap(chunkMapSize);

//    m_layersPerlin->FillNoiseSet(layerMap.data(), offset.x, offset.y, offset.z, _Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value);

    int seaLevel=(size.z/2);
    float heightScale=((float)size.z/2.0f);
    unsigned int validCells=0;

    index=0;
    size_t heightIndex=0;
    position.z=scaledOffset.z;
    for(int z=0; z<ChunkType::sizeZ::value; ++z)
    {
        heightIndex=0;
        position.y=scaledOffset.y;

        int blockZ=(int)startPos.z+z;
        for(int y=0; y<ChunkType::sizeY::value; ++y)
        {
            position.x=scaledOffset.x;
            for(int x=0; x<ChunkType::sizeX::value; ++x)
            {
                unsigned int blockType;
                int blockHeight=(int)(heightMap[heightIndex]*heightScale)+seaLevel;

//                if(position.z > heightMap[heightIndex]) //larger than height map, air
                if(blockZ>blockHeight)
                    blockType=0;
                else
                {
                    blockType=(blockHeight-blockZ)/13;
                    validCells++;
                }

                cells[index].type=blockType;
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

}//namespace voxigen

#endif //_voxigen_equiRectEquiRectWorldGenerator_h_