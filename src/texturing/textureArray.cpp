#include "voxigen/texturing/textureAtlas.h"

#include <generic/jsonSerializer.h>

#include <limits>
#include <algorithm>

namespace imglib
{
struct ReuseImage:public SimpleImage
{
};

inline bool resize(ReuseImage &image, Format format, Depth depth, size_t width, size_t height)
{
    image.format=format;
    image.depth=depth;
    image.width=width;
    image.stride=width;
    image.height=height;

    size_t neededSize=sizeOfPixel(format, depth)*width*height;

    if(image.dataSize<neededSize)
    {
        image.freeData();
        image.allocData(format, depth, width, height);
    }
    return true;
}

}//namespace imglib

namespace voxigen
{


TextureArray::TextureArray(size_t maxTextureWidth, size_t maxTextureHeight):
m_maxTextureWidth(maxTextureWidth),
m_maxTextureHeight(maxTextureHeight)
{}

TextureArray::~TextureArray()
{

}

void TextureArray::load(const std::string &path, const std::string &name)
{
}

void TextureArray::save(const std::string &path, const std::string &name)
{
    generic::JsonSerializer serializer;
    std::string configPath=path+"/"+name+".json";
    std::string imagePath=path+"/"+name+".png";

    if(!serializer.open(configPath))
        return;

    serializer.startObject();

    serializer.addKey("version");
    serializer.addUInt(1);

    serializer.addKey("blocks");
    serializer.startArray();

    for(auto &value:m_blockMap)
    {
        serializer.startObject();

        serializer.addKey("name");
        serializer.addString(value.first);

        serializer.addKey("index");
        serializer.addUInt((unsigned int)value.second);

        serializer.endObject();
    }

    serializer.endArray();

    serializer.addKey("entries");
    serializer.startArray();

    for(auto &entry:m_blockEntries)
    {
        serializer.startObject();

        for(size_t i=0; i<6; ++i)
        {
            TextureEntry &textureEntry=entry.faces[i];

            serializer.addKey(std::to_string(i));
            serializer.startObject();

            serializer.addKey("x");
            serializer.addUInt((unsigned int)textureEntry.x);

            serializer.addKey("y");
            serializer.addUInt((unsigned int)textureEntry.y);

            serializer.endObject();
        }

        serializer.endObject();
    }

    serializer.endArray();

    serializer.endObject();
//    serializer.close();

    imglib::save(m_image, imagePath);
}

const TextureArray::BlockEntry &TextureArray::getBlockEntry(size_t index) const
{
    assert(index<m_blockEntries.size());

    return m_blockEntries[index];
}


}//namespace voxigen
