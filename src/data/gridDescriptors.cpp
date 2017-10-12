#include "voxigen/gridDescriptors.h"

#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>

namespace voxigen
{

GridDescriptors::GridDescriptors()
{
    m_seed=0;
    m_size=glm::ivec3(1024, 1024, 256);
//    m_chunkSize=glm::ivec3(16, 16, 16);

    m_noiseScale=0.001;

//    contientFrequency=1.0;
    m_contientFrequency=0.005;
    m_contientOctaves=2;
    m_contientLacunarity=2.2;

    m_seaLevel=0.0f;
}

void GridDescriptors::create(std::string name, int seed, const glm::ivec3 &size, const glm::ivec3 &regionSize, const glm::ivec3 &chunkSize)
{
    m_name=name;
    m_seed=seed;
    m_size=size;
    m_regionSize=regionSize;
    m_chunkSize=chunkSize;

    init();
}

void GridDescriptors::load(std::string fileName)
{
    FILE *filePtr=fopen(fileName.c_str(), "rb");
    char readBuffer[65536];

    rapidjson::FileReadStream readStream(filePtr, readBuffer, sizeof(readBuffer));

    rapidjson::Document document;

    document.ParseStream(readStream);

    assert(document.IsObject());

    m_name=document["name"].GetString();
    m_seed=document["seed"].GetUint();
    m_generator=document["generator"].GetString();

    const rapidjson::Value &sizeArray=document["size"];

    m_size.x=sizeArray[0].GetInt();
    m_size.y=sizeArray[1].GetInt();
    m_size.z=sizeArray[2].GetInt();

    const rapidjson::Value &regionSizeArray=document["regionSize"];

    m_regionSize.x=regionSizeArray[0].GetInt();
    m_regionSize.y=regionSizeArray[1].GetInt();
    m_regionSize.z=regionSizeArray[2].GetInt();

    const rapidjson::Value &chunkSizeArray=document["chunkSize"];

    m_chunkSize.x=chunkSizeArray[0].GetInt();
    m_chunkSize.y=chunkSizeArray[1].GetInt();
    m_chunkSize.z=chunkSizeArray[2].GetInt();
    

    m_noiseScale=document["noiseScale"].GetDouble();
    m_contientFrequency=document["contientFrequency"].GetDouble();
    m_contientOctaves=document["contientOctaves"].GetInt();
    m_contientLacunarity=document["contientLacunarity"].GetDouble();
    m_seaLevel=document["seaLevel"].GetDouble();
    m_continentaShelf=document["continentaShelf"].GetDouble();

    fclose(filePtr);
    init();
}

void GridDescriptors::save(std::string fileName)
{
    FILE *filePtr=fopen(fileName.c_str(), "wb");
    char writeBuffer[65536];

    rapidjson::FileWriteStream fileStream(filePtr, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fileStream);

    writer.StartObject();

    writer.Key("name");
    writer.String(m_name.c_str());
    writer.Key("seed");
    writer.Uint(m_seed);
    writer.Key("generator");
    writer.String(m_generator.c_str());

    writer.Key("size");
    writer.StartArray();
    writer.Int(m_size.x);
    writer.Int(m_size.y);
    writer.Int(m_size.z);
    writer.EndArray();

    writer.Key("regionSize");
    writer.StartArray();
    writer.Int(m_regionSize.x);
    writer.Int(m_regionSize.y);
    writer.Int(m_regionSize.z);
    writer.EndArray();

    writer.Key("chunkSize");
    writer.StartArray();
    writer.Int(m_chunkSize.x);
    writer.Int(m_chunkSize.y);
    writer.Int(m_chunkSize.z);
    writer.EndArray();

    writer.Key("noiseScale");
    writer.Double(m_noiseScale);
    writer.Key("contientFrequency");
    writer.Double(m_contientFrequency);
    writer.Key("contientOctaves");
    writer.Int(m_contientOctaves);
    writer.Key("contientLacunarity");
    writer.Double(m_contientLacunarity);
    writer.Key("seaLevel");
    writer.Double(m_seaLevel);
    writer.Key("continentaShelf");
    writer.Double(m_continentaShelf);

    writer.EndObject();

    fclose(filePtr);
}

void GridDescriptors::init()
{
    m_regionCellSize=m_regionSize*m_chunkSize;
    m_regionCount=m_size/(m_regionCellSize);
    m_regionStride=glm::ivec3(1, m_regionCount.x, m_regionCount.x*m_regionCount.y);

    m_chunkCount=m_regionCellSize/m_chunkSize;
    m_chunkStride=glm::ivec3(1, m_chunkCount.x, m_chunkCount.x*m_chunkCount.y);
}

RegionHash GridDescriptors::regionHash(const glm::ivec3 &index) const
{
    return m_regionStride.x*index.x+m_regionStride.y*index.y+m_regionStride.z*index.z;
}

glm::ivec3 GridDescriptors::regionIndex(RegionHash hash) const
{
    glm::ivec3 index;

    index.z=hash/m_regionStride.z;
    hash=hash-(m_regionStride.z*index.z);
    index.y=hash/m_regionStride.y;
    index.x=hash-(m_regionStride.y*index.y);

    return index;
}

glm::ivec3 GridDescriptors::regionIndex(const glm::vec3 &pos) const
{
    glm::ivec3 position=glm::floor(pos);

    return position/m_regionCellSize;
}

glm::vec3 GridDescriptors::regionOffset(RegionHash hash) const
{
    glm::ivec3 index=regionIndex(hash);
    glm::vec3 offset=m_regionCellSize*index;

    return offset;
}

glm::vec3 GridDescriptors::adjustRegion(glm::ivec3 &regionIndex, glm::ivec3 &chunkIndex) const
{
    glm::ivec3 offset(0, 0 ,0);

    if(chunkIndex.x<0)
    {
        offset.x--;
        chunkIndex.x=m_regionSize.x+chunkIndex.x;
    }
    else if(chunkIndex.x>=m_regionSize.x)
    {
        offset.x++;
        chunkIndex.x=chunkIndex.x-m_regionSize.x;
    }
    if(chunkIndex.y<0)
    {
        offset.y--;
        chunkIndex.y=m_regionSize.y+chunkIndex.y;
    }
    else if(chunkIndex.y>=m_regionSize.y)
    {
        offset.y++;
        chunkIndex.y=chunkIndex.y-m_regionSize.y;
    }
    if(chunkIndex.z<0)
    {
        offset.z--;
        chunkIndex.z=m_regionSize.z+chunkIndex.z;
    }
    else if(chunkIndex.z>=m_regionSize.z)
    {
        offset.z++;
        chunkIndex.z=chunkIndex.z-m_regionSize.z;
    }

    regionIndex+=offset;
    return glm::vec3(m_regionCellSize*offset);
}

float GridDescriptors::distance(glm::ivec3 &regionIndex1, glm::ivec3 &chunkIndex1, glm::ivec3 &regionIndex2, glm::ivec3 &chunkIndex2) const
{
    glm::ivec3 offset(0.0f, 0.0f, 0.0f);

    if(regionIndex1!=regionIndex2)
        offset=(regionIndex2-regionIndex1)*m_regionSize;

    glm::ivec3 chunkPos1=chunkIndex1*m_chunkSize;
    glm::ivec3 chunkPos2=(chunkIndex2+offset)*m_chunkSize;
    return glm::length(glm::vec3(chunkPos1-chunkPos2));
}

ChunkHash GridDescriptors::chunkHash(const glm::ivec3 &chunkIndex) const
{
    return (m_chunkStride.z*chunkIndex.z)+(m_chunkStride.y*chunkIndex.y)+chunkIndex.x;
//    return (chunkStride.x*index.x)+(chunkStride.y*index.y)+index.z;
}

glm::ivec3 GridDescriptors::chunkIndex(ChunkHash chunkHash) const
{
    glm::ivec3 index;

    index.z=chunkHash/m_chunkStride.z;
    chunkHash=chunkHash-(m_chunkStride.z*index.z);
    index.y=chunkHash/m_chunkStride.y;
    index.x=chunkHash-(m_chunkStride.y*index.y);
    
    return index;
}

glm::vec3 GridDescriptors::chunkOffset(ChunkHash chunkHash) const
{
    glm::ivec3 index=chunkIndex(chunkHash);
    glm::vec3 offset=m_chunkSize*index;

    return offset;
}

}//namespace voxigen
