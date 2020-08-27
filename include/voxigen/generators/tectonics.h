#ifndef _voxigen_tectonics_h_
#define _voxigen_tectonics_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

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
    float plateDistanceValueNorm;
    float continentValue;

    float collision;
    float terrainScale;
    
    size_t weatherCell;
    size_t weatherBand;

    glm::vec2 direction;
    glm::vec2 airDirection;
    float airCurrent;
    
    float temperature;
    float moistureCapacity;
    float moisture;
};

struct PlateInfo
{
    size_t index;
    float value;

    glm::vec2 driftDirection;
    glm::vec3 direction;

    float height;

    glm::ivec2 point;
    glm::vec3 point3d;
    std::vector<glm::ivec2> points;

    std::vector<size_t> neighbors;
    std::vector<float> neighborCollisions;
};

inline void calculateCurve(float distance, float &plate1, float &plate2, float cutoff=0.07f)
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
    if(distance>0.9)
    {
        distance=(distance-0.9)*10.0f;
        return (1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)))*0.5f+0.5;
    }
    else if(distance>0.8)
    {
        distance=(distance-0.8)*10.0f;
        return (1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)))*0.3+0.2;
    }
    if(distance>0.6)
    {
        distance=(distance-0.6)*5.0f;
        return (distance*distance)*0.2;
    }
    return 0.0f;
}

inline float orogenicCurve(float distance)
{
    if(distance>0.6)
    {
        distance=(distance-0.6)*2.5f;
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

}//namespace voxigen

#endif //_voxigen_tectonics_h_