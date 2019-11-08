#ifndef _voxigen_weather_h_
#define _voxigen_weather_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/sortedVector.h"
#include "voxigen/math_helpers.h"
#include "voxigen/noise.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>

namespace voxigen
{

struct WeatherBand
{
    std::string name;
    float latitude;
    float size;
    float moistureUpper;
    float moistureMiddle;
    float moistureLower;
};

struct WeatherCell
{
    friend bool operator<(const WeatherCell &left, const WeatherCell &right)
    {   return (left.latitude<right.latitude);}

    std::string name;

    float latitude;
    float size;
    float moisture;
    
    glm::vec2 windDirectionLower;
    glm::vec2 windDirectionUpper;
};

class WeatherBands
{
public:
    WeatherBands(int seed, const glm::ivec2 &size, std::vector<WeatherCell> cells)
    {
        if(cells.empty())
        {
            assert(false);
            return;
        }

        for(size_t i=0; i<cells.size(); ++i)
        {
            insert_sorted(m_cells, cells[i]);
        }
        
        float prevFrontSize=0.0f;
        float prevMoisture=m_cells[0].moisture;
//        float prevFrontUpper=current.latitude+(current.size*0.5f)

        for(size_t i=0; i<m_cells.size()-1; ++i)
        {
            WeatherBand cellBand;
            WeatherBand front;

            WeatherCell &current=m_cells[i];
            WeatherCell &next=m_cells[i+1];

            cellBand.name=current.name;

            float upperLatitude=current.latitude+(current.size*0.5f);
            float frontSize=(M_PI_2-abs(upperLatitude))*(20.0f/90.0f);

            cellBand.size=current.size-(prevFrontSize*0.5f)-(frontSize*0.5f);
            cellBand.latitude=current.latitude+(prevFrontSize*0.25f)-(frontSize*0.25f);
            cellBand.moistureLower=(current.moisture+prevMoisture)*0.5f;
            cellBand.moistureMiddle=current.moisture;

            glm::vec2 wind=current.windDirectionUpper-next.windDirectionLower;
            float frontMoisture=abs(glm::dot(current.windDirectionUpper, next.windDirectionLower));

            if(wind.y < 0.0f) //wind moving away from each other
                frontMoisture=0.2f;

            front.name=current.name+"/"+next.name+" front";
            front.latitude=upperLatitude;
            front.size=frontSize;
            front.moistureLower=(current.moisture+frontMoisture)*0.5f;
            front.moistureMiddle=frontMoisture;
            front.moistureUpper=(next.moisture+frontMoisture)*0.5f;

            cellBand.moistureUpper=(current.moisture+frontMoisture)*0.5f;
            m_bands.push_back(cellBand);

            m_bands.push_back(front);

            prevFrontSize=frontSize;
            prevMoisture=frontMoisture;
        }

        //add last cell band
        {
            WeatherCell &current=m_cells[m_cells.size()-1];
            WeatherBand cellBand;

            cellBand.name=current.name;
            cellBand.size=current.size-(prevFrontSize*0.5f);
            cellBand.latitude=current.latitude+(prevFrontSize*0.25f);
            cellBand.moistureLower=(current.moisture+prevMoisture)*0.5f;
            cellBand.moistureMiddle=current.moisture;
            cellBand.moistureUpper=current.moisture;

            m_bands.push_back(cellBand);
        }

		size_t simdLevel=HastyNoise::GetFastestSIMD();
		std::unique_ptr<HastyNoise::NoiseSIMD> noise=HastyNoise::CreateNoise(seed, simdLevel);

		glm::ivec2 noiseSize(size.x, m_bands.size());
		int noiseVectorSize=HastyNoise::AlignedSize(noiseSize.x*noiseSize.y, simdLevel);

		std::unique_ptr<HastyNoise::VectorSet> noiseSet=std::make_unique<HastyNoise::VectorSet>(simdLevel);
		m_noiseMap.resize(noiseVectorSize);
		noiseSet->SetSize(noiseVectorSize);

		glm::vec3 mapPos;
		size_t index=0;
		mapPos.z=(float)(noiseSize.x/2.0f);
		for(int y=0; y<noiseSize.y; y++)
		{
			mapPos.y=m_bands[y].latitude;
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

		noise->SetSeed(seed);
		noise->SetNoiseType(HastyNoise::NoiseType::PerlinFractal);
		noise->SetFrequency(0.01f);
		noise->SetFractalLacunarity(2.0f);
		noise->SetFractalOctaves(4);
		noise->FillSet(m_noiseMap.data(), noiseSet.get());
    }

    size_t getCellIndex(const glm::vec2 &coords)
    {
        for(size_t i=0; i<m_cells.size(); ++i)
        {
            float upperLatitude=m_cells[i].latitude+(m_cells[i].size*0.5f);
            
            if(coords.y<upperLatitude)
                return i;
        }
        return m_cells.size()-1;
    }
	size_t getCellIndex(float latitude)
	{
		return getCellIndex({0.0f, latitude});
	}

    const WeatherCell &getCell(const glm::vec2 &coords)
    {
        return m_cells[getCellIndex(coords.y)];
    }
	const WeatherCell &getCell(float latitude)
	{
		return getCell({0.0f, latitude});
	}
	

    glm::vec2 getWindDirection(const glm::vec2 &coords)
    {
        const WeatherCell cell=getCell(coords);

		float latitude=coords.y;
        if(latitude<-1.396)
            latitude=latitude;

        float lower=cell.latitude-(cell.size*0.5f);
        float value=(latitude-lower)/cell.size;

        glm::vec2 direction;

        return (cell.windDirectionUpper-cell.windDirectionLower)*value+cell.windDirectionLower;
    }
	glm::vec2 getWindDirection(float latitude)
	{
		return getWindDirection({0.0f, latitude});
	}

    size_t getBandIndex(const glm::vec2 &coords)
    {
		float latitude=coords.y;

        for(size_t i=0; i<m_bands.size(); ++i)
        {
            float upperLatitude=m_bands[i].latitude+(m_bands[i].size*0.5f);

            if(latitude<upperLatitude)
                return i;
        }
        return m_bands.size()-1;
    }
	size_t getBandIndex(float latitude)
	{
		return getBandIndex({0.0f, latitude});
	}

    const WeatherBand &getBand(const glm::vec2 &coords)
    {
        return m_bands[getBandIndex(coords)];
    }
	const WeatherBand &getBand(float latitude)
	{
		return getBand({0.0f, latitude});
	}

    float getMoisture(const glm::vec2 &coords)
    {
        const WeatherBand band=getBand(coords);

		float latitude=coords.y;
        float lower=band.latitude-(band.size*0.5f);
        float value=(latitude-lower)/band.size;

        //assert(value<=1.0f);
        value=clamp(value, 0.0f, 1.0f);

        if(value<0.5f)
        {
            value=2*value;
            return (band.moistureMiddle-band.moistureLower)*(1.0f-pow(value-1.0f, 2))+band.moistureLower;
        }
        else
        {
            value=2*(value-0.5);
            return (band.moistureUpper-band.moistureMiddle)*(pow(value, 2))+band.moistureMiddle;
        }
    }
	float getMoisture(float latitude)
	{
		return getMoisture({0.0f, latitude});
	}

private:
    std::vector<WeatherBand> m_bands;
    std::vector<WeatherCell> m_cells;

//	std::unique_ptr<HastyNoise::NoiseSIMD> m_noise;
//	std::unique_ptr<HastyNoise::VectorSet> m_noiseSet;
	std::vector<float> m_noiseMap;
};

constexpr float rads(float degrees)
{
    return M_PI/180.0f*degrees;
}

//const CellBand g_cellBands[]=
//{
//    {"Equator", rads(0.0f) , rads(10.0f), 0.7f, false},
//    {"Hadley Cell" , rads(17.5f), rads(25.0f), 1.0f, true},
//    {"Horse Latitude"  , rads(35.0f), rads(10.0f), 0.2f, false},
//    {"Ferrell Cell", rads(55.0f), rads(20.0f), 0.6f, true},
//    {"Polar Front" , rads(68.5f), rads(5.0f) , 0.8f, false},
//    {"Polar Cell"  , rads(80.0f), rads(20.0f), 0.45f, true}
//};
//
//
//inline size_t getCellIndex(float latitude, bool onlyCells=false)
//{
//    static bool init=false;
//    static std::vector<float> limits;
//
//    if(!init)
//    {
//        limits.resize(6);
//        for(size_t i=0; i<6; ++i)
//            limits[i]=g_cellBands[i].latitude+(g_cellBands[i].size*0.5f);
//        init=true;
//    }
//
//    size_t index=0;
//
//    for(size_t i=0; i<6; ++i)
//    {
//        if(latitude<limits[i])
//        {
//            if(onlyCells)
//                return index;
//            else
//                return i;
//        }
//
//        if(g_cellBands[i].cell)
//            index++;
//    }
//    return g_cellBands.size()-1;
//}
//
//inline size_t gellCellBoundaryCount
//
//inline const CellBand &getCellBand(float latitude)
//{
//    size_t index=getCellIndex(latitude);
//
//    return g_cellBands[index];
//}

inline float getTemperature(float latitude)
{
    float magnitude=abs(latitude/M_PI_2);

    magnitude=magnitude*magnitude;
    return 60.0f-(150.0f*magnitude);
}

}//namespace voxigen

#endif //_voxigen_weather_h_