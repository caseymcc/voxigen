#ifndef _voxigen_weather_h_
#define _voxigen_weather_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/sortedVector.h"

#define _USE_MATH_DEFINES
#include <math.h>

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
    WeatherBands(std::vector<WeatherCell> cells)
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
            float frontSize=(M_PI_2-abs(upperLatitude))*(15.0f/90.0f);

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
    }

    size_t getCellIndex(float latitude)
    {
        for(size_t i=0; i<m_cells.size(); ++i)
        {
            float upperLatitude=m_cells[i].latitude+(m_cells[i].size*0.5f);
            
            if(latitude<upperLatitude)
                return i;
        }
        return m_cells.size()-1;
    }

    const WeatherCell &getCell(float latitude)
    {
        return m_cells[getCellIndex(latitude)];
    }

    glm::vec2 getWindDirection(float latitude)
    {
        const WeatherCell cell=getCell(latitude);

        if(latitude<-1.396)
            latitude=latitude;

        float lower=cell.latitude-(cell.size*0.5f);
        float value=(latitude-lower)/cell.size;

        glm::vec2 direction;

        return (cell.windDirectionUpper-cell.windDirectionLower)*value+cell.windDirectionLower;
    }

    size_t getBandIndex(float latitude)
    {
        for(size_t i=0; i<m_bands.size(); ++i)
        {
            float upperLatitude=m_bands[i].latitude+(m_bands[i].size*0.5f);

            if(latitude<upperLatitude)
                return i;
        }
        return m_bands.size()-1;
    }

    const WeatherBand &getBand(float latitude)
    {
        return m_bands[getBandIndex(latitude)];
    }

    float getMoisture(float latitude)
    {
        const WeatherBand band=getBand(latitude);

        float lower=band.latitude-(band.size*0.5f);
        float value=(latitude-lower)/band.size;

        assert(value<=1.0f);

        if(value<0.5f)
            return (band.moistureMiddle-band.moistureLower)*(2.0f*value)+band.moistureLower;
        else
            return (band.moistureUpper-band.moistureMiddle)*(2.0f*(value-0.5f))+band.moistureMiddle;
    }

private:
    std::vector<WeatherBand> m_bands;
    std::vector<WeatherCell> m_cells;
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