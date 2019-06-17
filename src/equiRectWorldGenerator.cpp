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
    if(document.HasMember("contientFrequency"))
        m_contientFrequency=document["contientFrequency"].GetDouble();
    else
        retValue=false;
    if(document.HasMember("contientOctaves"))
        m_contientOctaves=document["contientOctaves"].GetInt();
    else
        retValue=false;
    if(document.HasMember("contientLacunarity"))
        m_contientLacunarity=document["contientLacunarity"].GetDouble();
    else
        retValue=false;
    if(document.HasMember("seaLevel"))
        m_seaLevel=document["seaLevel"].GetDouble();
    else
        retValue=false;
    if(document.HasMember("continentaShelf"))
        m_continentaShelf=document["continentaShelf"].GetDouble();
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
    document.AddMember("contientFrequency", rapidjson::Value(m_contientFrequency).Move(), document.GetAllocator());
    document.AddMember("contientOctaves", rapidjson::Value(m_contientOctaves).Move(), document.GetAllocator());
    document.AddMember("contientLacunarity", rapidjson::Value(m_contientLacunarity).Move(), document.GetAllocator());
    document.AddMember("seaLevel", rapidjson::Value(m_seaLevel).Move(), document.GetAllocator());
    document.AddMember("continentaShelf", rapidjson::Value(m_continentaShelf).Move(), document.GetAllocator());

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
