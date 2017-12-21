#include "voxigen/gridDescriptors.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>

namespace voxigen
{

bool loadDescriptors(IGridDescriptors *descriptors, const char *fileName)
{
    FILE *filePtr=fopen(fileName, "rb");
    char readBuffer[65536];

    rapidjson::FileReadStream readStream(filePtr, readBuffer, sizeof(readBuffer));

    rapidjson::Document document;

    document.ParseStream(readStream);

    assert(document.IsObject());

    descriptors->setName(document["name"].GetString());
    descriptors->setSeed(document["seed"].GetUint());
    descriptors->setGenerator(document["generator"].GetString());
    if(document.HasMember("generatorDescriptors"))
    {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

        document["generatorDescriptors"].Accept(writer);
        descriptors->setGeneratorDescriptors(sb.GetString());
    }

    const rapidjson::Value &sizeArray=document["size"];

    glm::ivec3 size;

    size.x=sizeArray[0].GetInt();
    size.y=sizeArray[1].GetInt();
    size.z=sizeArray[2].GetInt();

    descriptors->setSize(size);

    const rapidjson::Value &regionSizeArray=document["regionSize"];

    size.x=regionSizeArray[0].GetInt();
    size.y=regionSizeArray[1].GetInt();
    size.z=regionSizeArray[2].GetInt();

    descriptors->setRegionSize(size);

    const rapidjson::Value &chunkSizeArray=document["chunkSize"];
    
    size.x=chunkSizeArray[0].GetInt();
    size.y=chunkSizeArray[1].GetInt();
    size.z=chunkSizeArray[2].GetInt();

    descriptors->setChunkSize(size);
//    m_noiseScale=document["noiseScale"].GetDouble();
//    m_contientFrequency=document["contientFrequency"].GetDouble();
//    m_contientOctaves=document["contientOctaves"].GetInt();
//    m_contientLacunarity=document["contientLacunarity"].GetDouble();
//    m_seaLevel=document["seaLevel"].GetDouble();
//    m_continentaShelf=document["continentaShelf"].GetDouble();
    fclose(filePtr);
    return true;
}

bool saveDescriptors(IGridDescriptors *descriptors, const char *fileName)
{
    FILE *filePtr=fopen(fileName, "wb");
    char writeBuffer[65536];

    rapidjson::FileWriteStream fileStream(filePtr, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fileStream);

    writer.StartObject();

    writer.Key("name");
    writer.String(descriptors->getName());
    writer.Key("seed");
    writer.Uint(descriptors->getSeed());
    writer.Key("generator");
    writer.String(descriptors->getGenerator());
    writer.Key("generatorDescriptors");
//    writer.String(descriptors->getGeneratorDescriptors());

    size_t descriptorLength=strlen(descriptors->getGeneratorDescriptors());

    if(descriptorLength>0)
        writer.RawValue(descriptors->getGeneratorDescriptors(), strlen(descriptors->getGeneratorDescriptors()), rapidjson::kObjectType);
    else
        writer.RawValue("{}", 2, rapidjson::kObjectType);

    glm::ivec3 size=descriptors->getSize();
    
    writer.Key("size");
    writer.StartArray();
    writer.Int(size.x);
    writer.Int(size.y);
    writer.Int(size.z);
    writer.EndArray();

    size=descriptors->getRegionSize();

    writer.Key("regionSize");
    writer.StartArray();
    writer.Int(size.x);
    writer.Int(size.y);
    writer.Int(size.z);
    writer.EndArray();

    size=descriptors->getChunkSize();

    writer.Key("chunkSize");
    writer.StartArray();
    writer.Int(size.x);
    writer.Int(size.y);
    writer.Int(size.z);
    writer.EndArray();
//    writer.Key("noiseScale");
//    writer.Double(m_noiseScale);
//    writer.Key("contientFrequency");
//    writer.Double(m_contientFrequency);
//    writer.Key("contientOctaves");
//    writer.Int(m_contientOctaves);
//    writer.Key("contientLacunarity");
//    writer.Double(m_contientLacunarity);
//    writer.Key("seaLevel");
//    writer.Double(m_seaLevel);
//    writer.Key("continentaShelf");
//    writer.Double(m_continentaShelf);

    writer.EndObject();
    fclose(filePtr);
    return true;
}

}//namespace voxigen
