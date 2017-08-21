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
    m_generator=document["generator"].GetString();

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
    writer.Key("generator");
    writer.String(m_generator.c_str());

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
    m_segmentCellSize=m_segmentSize*m_chunkSize;
    m_segmentCount=m_size/(m_segmentCellSize);
    m_segmentStride=glm::ivec3(1, m_segmentCount.x, m_segmentCount.x*m_segmentCount.y);

    m_chunkCount=m_segmentCellSize/m_chunkSize;
    m_chunkStride=glm::ivec3(1, m_chunkCount.x, m_chunkCount.x*m_chunkCount.y);
}

SegmentHash GridDescriptors::segmentHash(const glm::ivec3 &index) const
{
    return m_segmentStride.x*index.x+m_segmentStride.y*index.y+m_segmentStride.z*index.z;
}

glm::ivec3 GridDescriptors::segmentIndex(SegmentHash hash) const
{
    glm::ivec3 index;

    index.z=hash/m_segmentStride.z;
    hash=hash-(m_segmentStride.z*index.z);
    index.y=hash/m_segmentStride.y;
    index.x=hash-(m_segmentStride.y*index.y);

    return index;
}

glm::ivec3 GridDescriptors::segmentIndex(const glm::vec3 &pos) const
{
    glm::ivec3 position=glm::floor(pos);

    return position/m_segmentCellSize;
}

glm::vec3 GridDescriptors::segmentOffset(SegmentHash hash) const
{
    glm::ivec3 index=segmentIndex(hash);
    glm::vec3 offset=m_segmentCellSize*index;

    return offset;
}

glm::vec3 GridDescriptors::adjustSegment(glm::ivec3 &segmentIndex, glm::ivec3 &chunkIndex) const
{
    glm::ivec3 offset(0, 0 ,0);

    if(chunkIndex.x<0)
    {
        offset.x--;
        chunkIndex.x=m_segmentSize.x+chunkIndex.x;
    }
    else if(chunkIndex.x>=m_segmentSize.x)
    {
        offset.x++;
        chunkIndex.x=chunkIndex.x-m_segmentSize.x;
    }
    if(chunkIndex.y<0)
    {
        offset.y--;
        chunkIndex.y=m_segmentSize.y+chunkIndex.y;
    }
    else if(chunkIndex.y>=m_segmentSize.y)
    {
        offset.y++;
        chunkIndex.y=chunkIndex.y-m_segmentSize.y;
    }
    if(chunkIndex.z<0)
    {
        offset.z--;
        chunkIndex.z=m_segmentSize.z+chunkIndex.z;
    }
    else if(chunkIndex.z>=m_segmentSize.z)
    {
        offset.z++;
        chunkIndex.z=chunkIndex.z-m_segmentSize.z;
    }

    segmentIndex+=offset;
    return glm::vec3(m_segmentCellSize*offset);
}

float GridDescriptors::distance(glm::ivec3 &segmentIndex1, glm::ivec3 &chunkIndex1, glm::ivec3 &segmentIndex2, glm::ivec3 &chunkIndex2) const
{
    glm::ivec3 offset(0.0f, 0.0f, 0.0f);

    if(segmentIndex1!=segmentIndex2)
        offset=(segmentIndex1-segmentIndex2)*m_segmentSize;

    return glm::length((chunkIndex1-chunkIndex3)+offset);
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
