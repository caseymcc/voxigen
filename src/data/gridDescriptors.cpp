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

void GridDescriptors::create(std::string name, int seed, const glm::ivec3 &size, const glm::ivec3 &segmentSize, const glm::ivec3 &chunkSize)
{
    m_name=name;
    m_seed=seed;
    m_size=size;
    m_segmentSize=segmentSize;
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

    const rapidjson::Value &sizeArray=document["size"];

    m_size.x=sizeArray[0].GetInt();
    m_size.y=sizeArray[1].GetInt();
    m_size.z=sizeArray[2].GetInt();

    const rapidjson::Value &segmentSizeArray=document["segmentSize"];

    m_segmentSize.x=segmentSizeArray[0].GetInt();
    m_segmentSize.y=segmentSizeArray[1].GetInt();
    m_segmentSize.z=segmentSizeArray[2].GetInt();

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

    writer.Key("size");
    writer.StartArray();
    writer.Int(m_size.x);
    writer.Int(m_size.y);
    writer.Int(m_size.z);
    writer.EndArray();

    writer.Key("segmentSize");
    writer.StartArray();
    writer.Int(m_segmentSize.x);
    writer.Int(m_segmentSize.y);
    writer.Int(m_segmentSize.z);
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
    m_chunkCount=m_size/m_chunkSize;
    m_segmentCount=m_chunkCount/m_segmentSize;
    m_chunkStride=glm::ivec3(1, m_chunkCount.x, m_chunkCount.x*m_chunkCount.y);
}

unsigned int GridDescriptors::chunkHash(const glm::ivec3 &chunkIndex) const
{
    return (m_chunkStride.z*chunkIndex.z)+(m_chunkStride.y*chunkIndex.y)+chunkIndex.x;
//    return (chunkStride.x*index.x)+(chunkStride.y*index.y)+index.z;
}

glm::ivec3 GridDescriptors::chunkIndex(unsigned int chunkHash) const
{
    glm::ivec3 index;

    index.z=chunkHash/m_chunkStride.z;
    chunkHash=chunkHash-(m_chunkStride.z*index.z);
    index.y=chunkHash/m_chunkStride.y;
    index.x=chunkHash-(m_chunkStride.y*index.y);
    
    return index;
}

glm::vec3 GridDescriptors::chunkOffset(unsigned int chunkHash) const
{
    glm::ivec3 index=chunkIndex(chunkHash);
    glm::vec3 offset=m_chunkSize*index;

    return offset;
}

}//namespace voxigen
