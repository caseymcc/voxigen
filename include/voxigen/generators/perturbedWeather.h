#ifndef _voxigen_weatherPerturbed_h_
#define _voxigen_weatherPerturbed_h_

#include "voxigen/generators/weather.h"
#include "voxigen/noise.h"

#include <glm/glm.hpp>

namespace voxigen
{

struct WeatherBandInfo
{
    float upperLatitude;
    float lowerLatitude;
    float size;
};

struct WeatherCellInfo
{
    float upperLatitude;
    float lowerLatitude;
    float size;
};

class PerturbedWeatherBands:public WeatherBands
{
public:
    PerturbedWeatherBands(int seed, const glm::ivec2 &size, std::vector<WeatherCell> cells):
        WeatherBands(cells),
        m_size(size)
    {
        size_t simdLevel=HastyNoise::GetFastestSIMD();
        std::unique_ptr<HastyNoise::NoiseSIMD> noise=HastyNoise::CreateNoise(seed, simdLevel);

        glm::ivec2 noiseSize(size.x, m_cells.size()-1);
        int noiseVectorSize=HastyNoise::AlignedSize(noiseSize.x*noiseSize.y, simdLevel);

        std::unique_ptr<HastyNoise::VectorSet> noiseSet=std::make_unique<HastyNoise::VectorSet>(simdLevel);
        std::vector<float> noiseMap;
        noiseMap.resize(noiseVectorSize);
        noiseSet->SetSize(noiseVectorSize);

        size_t mapSize=size.x*size.y;

        glm::vec3 mapPos;
        size_t index=0;
        mapPos.z=(float)(noiseSize.x/2.0f);
        for(int y=0; y<noiseSize.y; y++)
        {
            mapPos.y=(size.y/2)-floor(m_cells[y].upperLatitude*((float)size.y/glm::pi<float>()));
            for(int x=0; x<noiseSize.x; x++)
            {
                mapPos.x=x;
                glm::vec3 pos=getSphericalCoords(size.x, size.y, mapPos);

                //hasty treats x and y in reverse, need to change
                noiseSet->xSet[index]=pos.y;
                noiseSet->ySet[index]=pos.x;
                noiseSet->zSet[index]=pos.z;
                index++;
            }
        }


        noise->SetNoiseType(HastyNoise::NoiseType::Perlin);
        noise->SetFrequency(0.003f);
//        noise->SetFractalLacunarity(2.0f);
//        noise->SetFractalOctaves(4);
        noise->FillSet(noiseMap.data(), noiseSet.get());

        float prevCellSize=0.0f;
        float nextCellSize=0.0f;
        
        m_cellsInfo.resize(m_cells.size());
        for(int i=0; i<m_cells.size()-1; i++)
        {
            WeatherCell &cell=m_cells[i];
            std::vector<WeatherCellInfo> &infos=m_cellsInfo[i];

            if(i+1<m_cells.size())
                nextCellSize=m_cells[i+1].upperLatitude-m_cells[i+1].lowerLatitude;
            else
                nextCellSize=0.0f;
            float cellSize=cell.upperLatitude-cell.lowerLatitude;

            infos.resize(size.x);
            for(int x=0; x<size.x; x++)
            {
                WeatherCellInfo &info=infos[x];
                
                if(i>0)
                {
                    float lowerNoise=noiseMap[((i-1)*m_size.x)+x];
                    if(lowerNoise>0.0f)
                        info.lowerLatitude=cell.lowerLatitude+lowerNoise*(cellSize*0.5f);
                    else
                        info.lowerLatitude=cell.lowerLatitude+lowerNoise*(prevCellSize*0.5f);
                }
                else
                    info.lowerLatitude=cell.lowerLatitude;

                float upperNoise=noiseMap[(i*m_size.x)+x];
                if(upperNoise>0.0f)
                    info.upperLatitude=cell.upperLatitude+upperNoise*(nextCellSize*0.5f);
                else
                    info.upperLatitude=cell.upperLatitude+upperNoise*(cellSize*0.5f);

                

                info.size=info.upperLatitude-info.lowerLatitude;
            }
            prevCellSize=cellSize;
        }

        //handle last
        {
            int i=m_cells.size()-1;

            WeatherCell &cell=m_cells[i];
            std::vector<WeatherCellInfo> &infos=m_cellsInfo[i];
            float cellSize=cell.upperLatitude-cell.lowerLatitude;

            infos.resize(size.x);
            for(int x=0; x<size.x; x++)
            {
                WeatherCellInfo &info=infos[x];
                float lowerNoise=noiseMap[((i-1)*m_size.x)+x];

                info.upperLatitude=cell.upperLatitude;

                if(lowerNoise>0.0f)
                    info.lowerLatitude=cell.lowerLatitude+lowerNoise*(cellSize*0.5f);
                else
                    info.lowerLatitude=cell.lowerLatitude+lowerNoise*(prevCellSize*0.5f);

                info.size=info.upperLatitude-info.lowerLatitude;
            }
        }

        std::vector<WeatherCell> tempCells(m_cells.begin(), m_cells.end());
        std::vector<WeatherBand> tempBands;

        //get noise to modify the cell fronts
//        noise->SetSeed(seed+1);
//        noise->FillSet(noiseMap.data(), noiseSet.get());

        m_bandsInfo.resize(m_bands.size());
        for(int i=0; i<m_bands.size(); i++) 
            m_bandsInfo[i].resize(size.x);

        for(int x=0; x<size.x; x++)
        {
            for(int i=0; i<m_cells.size(); i++)
            {
                WeatherCell &cell=tempCells[i];
                WeatherCellInfo &info=m_cellsInfo[i][x];

                cell.upperLatitude=info.upperLatitude;
                cell.lowerLatitude=info.lowerLatitude;
            }

            generateWeatherBands(tempCells, tempBands);

            for(int i=0; i<m_bands.size(); i++)
            {
                WeatherBand &band=tempBands[i];
                WeatherBandInfo &info=m_bandsInfo[i][x];

                info.upperLatitude=band.upperLatitude;
                info.lowerLatitude=band.lowerLatitude;
                info.size=band.size;
            }

            tempBands.resize(0);
        }
    }

    size_t getCellIndex(const glm::vec2 &coord)
    {
        int x=floor(coord.x/glm::two_pi<float>()*m_size.x);

        for(size_t i=0; i<m_cells.size(); ++i)
        {
            WeatherCellInfo &info=m_cellsInfo[i][x];

            if(coord.y<info.upperLatitude)
                return i;
        }
        return m_cells.size()-1;
    }

    glm::vec2 getWindDirection(const glm::vec2 &coord)
    {
        size_t index=getCellIndex(coord);
        int x=floor(coord.x/glm::two_pi<float>()*m_size.x);

        WeatherCell &cell=m_cells[index];
        WeatherCellInfo &info=m_cellsInfo[index][x];

        float value=(coord.y-info.lowerLatitude)/info.size;

//        glm::vec2 direction;

        return (cell.windDirectionUpper-cell.windDirectionLower)*value+cell.windDirectionLower;
    }

	size_t getBandIndex(const glm::vec2 &coord)
    {
        int x=floor(coord.x/glm::two_pi<float>()*m_size.x);

        for(size_t i=0; i<m_bandsInfo.size(); ++i)
        {
            WeatherBandInfo &info=m_bandsInfo[i][x];

            if(coord.y < info.upperLatitude)
                return i;
        }
        return m_bandsInfo.size()-1;
    }

    float getMoisture(const glm::vec2 &coord)
    {
        size_t index=getBandIndex(coord);
        int x=(int)floor(coord.x/glm::two_pi<float>()*m_size.x);

        WeatherBand &band=m_bands[index];
        WeatherBandInfo &info=m_bandsInfo[index][x];

        float value=(coord.y-info.lowerLatitude)/info.size;

        //assert(value<=1.0f);
        value=clamp(value, 0.0f, 1.0f);

        if(value<0.5f)
        {
            value=2.0f*value;
            return (band.moistureMiddle-band.moistureLower)*(1.0f-pow(value-1.0f, 2))+band.moistureLower;
        }
        else
        {
            value=2.0f*(value-0.5f);
            return (band.moistureUpper-band.moistureMiddle)*(pow(value, 2))+band.moistureMiddle;
        }
    }

private:
    glm::ivec2 m_size;

    std::vector<std::vector<WeatherBandInfo>> m_bandsInfo;
    std::vector<std::vector<WeatherCellInfo>> m_cellsInfo;
};

}//namespace voxigen

#endif //_voxigen_weather_h_