#ifndef _voxigen_equiRectWorldGenerator_h_
#define _voxigen_equiRectWorldGenerator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/chunk.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/coords.h"

#include <noise/noise.h>
#include <voxigen/noise/FastNoiseSIMD.h>

namespace voxigen
{

template<typename _Chunk>
class EquiRectWorldGenerator
{
public:
    typedef _Chunk ChunkType;
    typedef std::unique_ptr<_Chunk> UniqueChunkType;

    EquiRectWorldGenerator();
    ~EquiRectWorldGenerator();

    void initialize(GridDescriptors *descriptors);
//    void setWorld(GridDescriptors descriptors);
//    void setWorldDiscriptors(GridDescriptors descriptors);
    
    void generateWorldOverview();

    UniqueChunkType generateChunk(unsigned int hash, void *buffer, size_t bufferSize);
    UniqueChunkType generateChunk(glm::ivec3 chunkIndex, void *buffer, size_t bufferSize);
    UniqueChunkType generateChunk(unsigned int hash, glm::ivec3 &chunkIndex, void *buffer, size_t bufferSize);

private:
    GridDescriptors *m_descriptors;

    std::unique_ptr<FastNoiseSIMD> m_continentPerlin;
    std::unique_ptr<FastNoiseSIMD> m_layersPerlin;

//    noise::module::Perlin m_perlin;
//    noise::module::Perlin m_continentPerlin;
//    noise::module::Curve m_continentCurve;
//
//    noise::module::Perlin m_layersPerlin;
};

template<typename _Chunk>
EquiRectWorldGenerator<_Chunk>::EquiRectWorldGenerator()
{
}

template<typename _Chunk>
EquiRectWorldGenerator<_Chunk>::~EquiRectWorldGenerator()
{}

template<typename _Chunk>
void EquiRectWorldGenerator<_Chunk>::initialize(GridDescriptors *descriptors)
{
    m_descriptors=descriptors;

    assert(descriptors->m_chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
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

//template<typename _Chunk>
//void EquiRectWorldGenerator<_Chunk>::setWorldDiscriptors(GridDescriptors descriptors)
//{
//
//}

template<typename _Chunk>
void EquiRectWorldGenerator<_Chunk>::generateWorldOverview()
{

}

template<typename _Chunk>
typename EquiRectWorldGenerator<_Chunk>::UniqueChunkType EquiRectWorldGenerator<_Chunk>::generateChunk(unsigned int hash, void *buffer, size_t bufferSize)
{
    glm::ivec3 chunkIndex=m_descriptors->chunkIndex(hash);

    return generateChunk(hash, chunkIndex, buffer, bufferSize);
}

template<typename _Chunk>
typename EquiRectWorldGenerator<_Chunk>::UniqueChunkType EquiRectWorldGenerator<_Chunk>::generateChunk(glm::ivec3 chunkIndex, void *buffer, size_t bufferSize)
{
    unsigned int hash=m_descriptors->chunkHash(chunkIndex);

    return generateChunk(hash, chunkIndex, buffer, bufferSize);
}

template<typename _Chunk>
typename EquiRectWorldGenerator<_Chunk>::UniqueChunkType EquiRectWorldGenerator<_Chunk>::generateChunk(unsigned int hash, glm::ivec3 &chunkIndex, void *buffer, size_t bufferSize)
{
    glm::vec3 offset=glm::ivec3(_Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value)*chunkIndex;
    glm::vec3 scaledOffset=offset*m_descriptors->m_noiseScale;
    glm::vec3 position=scaledOffset;
    float noiseScale=m_descriptors->m_noiseScale;
    
   
    UniqueChunkType chunk=std::make_unique<ChunkType>(hash, 0, chunkIndex, offset);
//    ChunkType::Cells &cells=chunk->getCells();
    
    ChunkType::CellType *cells=(ChunkType::CellType *)buffer;

    //verify buffer is large enough for data
    assert(bufferSize>=(_Chunk::sizeX::value*_Chunk::sizeY::value*_Chunk::sizeZ::value)*sizeof(ChunkType::CellType));

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
    unsigned int validCells=0;

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

    chunk->setValidCellCount(validCells);
    return chunk;
}

}//namespace voxigen

#endif //_voxigen_equiRectEquiRectWorldGenerator_h_