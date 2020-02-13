#include "voxigen/generators/equiRectWorldGenerator.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace voxigen
{

bool EquiRectDescriptors::load(const char *json)
{
    if(strlen(json)<=0)
        return false;

    rapidjson::Document document;

    document.Parse(json);
    bool retValue=true;
    
    if(document.HasMember("noiseScale"))
        m_noiseScale=document["noiseScale"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentFrequency"))
        m_continentFrequency=document["continentFrequency"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentOctaves"))
        m_continentOctaves=document["continentOctaves"].GetInt();
    else
        retValue=false;
    if(document.HasMember("continentLacunarity"))
        m_continentLacunarity=document["continentLacunarity"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("seaLevel"))
        m_seaLevel=document["seaLevel"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentalShelf"))
        m_continentalShelf=document["continentalShelf"].GetFloat();
    else
        retValue=false;

    return retValue;
}

bool EquiRectDescriptors::save(char *json, size_t &size)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    rapidjson::Document document;

    document.SetObject();

    document.AddMember("noiseScale", rapidjson::Value(m_noiseScale).Move(), document.GetAllocator());
    document.AddMember("continentFrequency", rapidjson::Value(m_continentFrequency).Move(), document.GetAllocator());
    document.AddMember("continentOctaves", rapidjson::Value(m_continentOctaves).Move(), document.GetAllocator());
    document.AddMember("continentLacunarity", rapidjson::Value(m_continentLacunarity).Move(), document.GetAllocator());
    document.AddMember("seaLevel", rapidjson::Value(m_seaLevel).Move(), document.GetAllocator());
    document.AddMember("continentalShelf", rapidjson::Value(m_continentalShelf).Move(), document.GetAllocator());

    document.Accept(writer);

    if(sb.GetSize()<size-1)
    {
        strncpy(json, sb.GetString(), sb.GetSize());
        size=sb.GetSize();
        return true;
    }
    
    size=sb.GetSize();
    return false;
}

void EquiRectDescriptors::init(IGridDescriptors *gridDescriptors)
{
    calculateInfluenceSize(gridDescriptors);
}

//this is expecting cylindrical wrap
std::vector<size_t> get2DCellNeighbors_eq(const glm::ivec2 &index, const glm::ivec2 &size)
{
    glm::ivec2 neighborIndex(index.x-1, index.y-1);
    std::vector<size_t> neighbors(9);

    size_t i=0;
    size_t nIndex;

    for(size_t y=0; y<3; ++y)
    {
        if(neighborIndex.y<0)
            neighborIndex.y=size.y+neighborIndex.y;
        else if(neighborIndex.y>=size.y)
            neighborIndex.y=neighborIndex.y-size.y;

        nIndex=size.x*(size_t)neighborIndex.y;

        neighborIndex.x=index.x-1;
        for(size_t x=0; x<3; ++x)
        {
            if(neighborIndex.x<0)
                neighborIndex.x=size.x+neighborIndex.x;
            else if(neighborIndex.x>=size.x)
                neighborIndex.x=neighborIndex.x-size.x;

            neighbors[i]=nIndex+(size_t)neighborIndex.x;
//            assert(neighbors[i]<size.x*size.y);
            i++;
            neighborIndex.x++;
        }

        neighborIndex.y++;
    }

    return neighbors;
}

}//namespace voxigen
