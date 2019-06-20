#include "voxigen/equiRectWorldGenerator.h"

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
        m_noiseScale=document["noiseScale"].GetDouble();
    else
        retValue=false;
    if(document.HasMember("continentFrequency"))
        m_continentFrequency=document["continentFrequency"].GetDouble();
    else
        retValue=false;
    if(document.HasMember("continentOctaves"))
        m_continentOctaves=document["continentOctaves"].GetInt();
    else
        retValue=false;
    if(document.HasMember("continentLacunarity"))
        m_continentLacunarity=document["continentLacunarity"].GetDouble();
    else
        retValue=false;
    if(document.HasMember("seaLevel"))
        m_seaLevel=document["seaLevel"].GetDouble();
    else
        retValue=false;
    if(document.HasMember("continentalShelf"))
        m_continentalShelf=document["continentalShelf"].GetDouble();
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

}//namespace voxigen
