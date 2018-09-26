#include "voxigen/textureAtlas.h"
#include "voxigen/regular2DGrid.h"

#include <generic/jsonSerializer.h>

#include <imglib/imageIo.h>
#include <imglib/copyTo.h>
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


struct TextureEntry
{
    std::string name;
    std::string path;

    imglib::SimpleImage image;

    size_t x;
    size_t y;
};

TextureAtlas::TextureAtlas(size_t maxTextureWidth, size_t maxTextureHeight):
m_maxTextureWidth(maxTextureWidth),
m_maxTextureHeight(maxTextureHeight)
{}

TextureAtlas::~TextureAtlas()
{

}

void TextureAtlas::load(const std::string &path, const std::string &name)
{
}

void TextureAtlas::save(const std::string &path, const std::string &name)
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
        serializer.addUInt(value.second);

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
            serializer.addUInt(textureEntry.x);

            serializer.addKey("y");
            serializer.addUInt(textureEntry.y);

            serializer.endObject();
        }

        serializer.endObject();
    }

    serializer.endArray();

    serializer.endObject();
//    serializer.close();

    imglib::save(m_image, imagePath);
}

//need empty function for regular2DGrid
bool empty(bool value) { return !value; }

struct ImageEntry:public TextureAtlas::TextureEntry
{
    size_t index;
    imglib::SimpleImage image;

    size_t tileCount;
};

bool TextureAtlas::build(const std::vector<std::string> &blocks, const TexturePack &pack)
{
    const std::vector<TextureInfo> &textureInfo=pack.textureInfo();
    std::vector<size_t> textureMap(textureInfo.size(), 0);
    std::vector<size_t> blockIds(blocks.size());
    std::vector<size_t> textureIds;

    m_textureResolution=pack.getResolution();

    for(size_t i=0; i<blocks.size(); ++i)
        blockIds[i]=pack.getBlockInfoId(blocks[i]);

    //find what textures are actually used for the requested blocks
    for(size_t i=0; i<blockIds.size(); ++i)
    {
        if(blockIds[i]==0)
            continue;

        const BlockInfo &blockInfo=pack.getBlockInfo(blockIds[i]);

        for(size_t face=0; face<6; ++face)
            textureMap[blockInfo.faces[face]]=1;
    }

    //get total textures we are going to deal with
    for(size_t i=0; i<textureMap.size(); ++i)
    {
        if(textureMap[i])
        {
            textureMap[i]=textureIds.size();
            textureIds.push_back(i);
        }
    }
    
    std::string packPath=pack.getPath();
    size_t textureCount=textureIds.size();

    std::vector<ImageEntry> imageEntries(textureCount);
    m_textureEntries.resize(textureCount);
//    std::vector<imglib::SimpleImage> images(textureCount);

//    size_t maxTilesX=0;
//    size_t maxTilesY=0;
    //
    size_t textureTileCount=0;
    for(size_t i=0; i<textureCount; ++i)
    {
        const TextureInfo &info=textureInfo[textureIds[i]];
        const LayerInfo &baseLayer=*(info.base);
        std::string imagePath=packPath+"/"+baseLayer.path;
        
        ImageEntry &entry=imageEntries[i];
        imglib::SimpleImage &image=entry.image;

        if(!imglib::load(image, imagePath))
            continue;

        entry.index=i;
        entry.method=baseLayer.method;
        entry.tileX=image.width/m_textureResolution;
        entry.tileY=image.height/m_textureResolution;

//        maxTilesX=std::max(entry.tileX, maxTilesX);
//        maxTilesY=std::max(entry.tileY, maxTilesY);

        entry.tileCount=entry.tileX*entry.tileY;
        textureTileCount+=entry.tileCount;
    }

    //figure out minimum texture atlas size
//    size_t x=(int)sqrt((float)textureTileCount);
//    size_t y=x;
//
//    if(x<maxTilesX)
//        x=maxTilesX;
//    if(y<maxTilesY)
//        x=maxTilesY;
//    
//    while(x*y<textureTileCount)
//    {
//        x++;
//        if(x*y>textureTileCount)
//            break;
//        y++;
//    }
//
//    std::vector<bool> textureTiles(x*y, false);
    
//    size_t textureWidth=x*m_textureResolution;
//    size_t textureHeight=y*m_textureResolution;
    size_t maxTextureTilesX=m_maxTextureWidth/m_textureResolution;
    size_t maxTextureTilesY=m_maxTextureHeight/m_textureResolution;
    size_t tilesX=0;
    size_t tilesY=0;
    size_t tilePosX=0;
    size_t tilePosY=0;
    
    Regular2DGrid<bool> tiles(maxTextureTilesX, maxTextureTilesY, false);
//    std::vector<bool> tiles(maxTilesX*maxTilesY, false);
    imglib::SimpleImage textureImage;
    
    textureImage.allocData(imglib::Format::RGBA, imglib::Depth::Bit8, m_maxTextureWidth, m_maxTextureHeight);

    //todo:need to take care of multiple resolution textures
    //copy textures over to atlas texture and update entries with position data
//    imglib::ReuseImage image;

    std::vector<size_t> sortedIndex(m_textureEntries.size());
    std::sort(imageEntries.begin(), imageEntries.end(), [](const ImageEntry &entry1, const ImageEntry &entry2){return entry1.tileCount>entry2.tileCount; });

    int maxTilesX=0;
    int maxTilesY=0;

    for(size_t i=0; i<m_textureEntries.size(); ++i)
    {
        ImageEntry &entry=imageEntries[i];
        const TextureInfo &info=textureInfo[textureIds[i]];
        imglib::SimpleImage &image=entry.image;
//        std::string imagePath=packPath+"/"+info.base->path;
//
//        if(!imglib::load(image, imagePath))
//        {
//            entry.x=0;
//            entry.y=0;
//            continue;
//        }
        glm::ivec2 tileSize(entry.tileX, entry.tileY);
        glm::ivec2 tilePos=tiles.findEmpty(tileSize);

        if(tilePos.x<0)
            continue;

        sortedIndex[entry.index]=i;

        maxTilesX=std::max(maxTilesX, (tilePos.x+tileSize.x));
        maxTilesY=std::max(maxTilesY, (tilePos.y+tileSize.y));

        tiles.fill(tilePos, tileSize, true);
        entry.x=tilePos.x*m_textureResolution;
        entry.y=tilePos.y*m_textureResolution;

        imglib::copyTo(textureImage, entry.x, entry.y, image);

//        posX++;
//        if(posX>=x)
//        {
//            posX=0;
//            posY++;
//        }
    }
    
    m_width=maxTilesX*m_textureResolution;
    m_height=maxTilesY*m_textureResolution;

    m_image.allocData(imglib::Format::RGBA, imglib::Depth::Bit8, m_width, m_height);
    imglib::copyTo(m_image, 0, 0, textureImage);

    //build up block information base on the atlas texture
    m_blockEntries.resize(blockIds.size());

    for(size_t i=0; i<blockIds.size(); ++i)
    {
        if(blockIds[i]==0)
            continue;

        const BlockInfo &blockInfo=pack.getBlockInfo(blockIds[i]);
        BlockEntry &blockEntry=m_blockEntries[i];

        m_blockMap.emplace(blocks[i], i);
        for(size_t face=0; face<6; ++face)
        {
            size_t id=blockInfo.faces[face];
            size_t textureEntryId=textureMap[id];
            textureEntryId=sortedIndex[textureEntryId];

            if(textureEntryId==0)
                continue;

//            TextureEntry &entry=m_textureEntries[textureEntryId];
            ImageEntry &entry=imageEntries[textureEntryId];

            blockEntry.faces[face]=entry;
        }
    }
    
    return false;
}

const TextureAtlas::BlockEntry &TextureAtlas::getBlockEntry(size_t index)
{
    assert(index<m_blockEntries.size());

    return m_blockEntries[index];
}


}//namespace voxigen
