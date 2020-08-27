#ifndef _voxigen_textureArray_h_
#define _voxigen_textureArray_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/texturing/texturePack.h"
#include "voxigen/volume/regular2DGrid.h"

#include <generic/stdFileIO.h>
#include <imglib/simpleImage.h>
#include <imglib/imageIo.h>
#include <imglib/copyTo.h>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace voxigen
{

//need empty function for regular2DGrid
inline bool empty(bool value) { return !value; }

class VOXIGEN_EXPORT TextureArray
{
public:
    struct TextureEntry
    {
        LayerMethod method;

        size_t x;
        size_t y;

        size_t tileX;
        size_t tileY;
    };

//    struct Face
//    {
//        size_t x;
//        size_t y;
//    };

    struct BlockEntry
    {
        std::array<TextureEntry, 6> faces;
    };

    typedef std::unordered_map<std::string, size_t> BlockMap;

    TextureArray(size_t maxTextureWidth=4096, size_t maxTextureHeight=4096);
    ~TextureArray();

    void load(const std::string &path, const std::string &name);
    void save(const std::string &path, const std::string &name);

    //bool build(const std::vector<std::string> &blocks , const TexturePack &pack);

    const BlockEntry &getBlockEntry(size_t index) const;

    const imglib::SimpleImage &getImage() const { return m_image; }

    size_t resolution() const { return m_textureResolution; }
    size_t size() const { return m_blockEntries.size(); }

    template<typename _FileIO>
    friend std::shared_ptr<TextureArray> generateTextureArray(const std::vector<std::string> &blocks, const TexturePack &pack, void *userData);

private:
    //these items are private and not used outside of the class api
#pragma warning(push)
#pragma warning(disable:4251)
    size_t m_version;
    std::vector<TextureEntry> m_textureEntries;
    BlockMap m_blockMap;
    std::vector<BlockEntry> m_blockEntries;
   
    size_t m_maxTextureWidth;
    size_t m_maxTextureHeight;

    size_t m_width;
    size_t m_height;

    size_t m_textureResolution;

    std::string m_imagePath;
    imglib::SimpleImage m_image;
#pragma warning(pop)
};

typedef std::shared_ptr<TextureArray> SharedTextureArray;

struct ImageEntry:public TextureArray::TextureEntry
{
    size_t index;
    imglib::SimpleImage image;

    size_t tileCount;
};

template<typename _FileIO=generic::io::StdFileIO>
SharedTextureArray generateTextureArray(const std::vector<std::string> &blocks, const TexturePack &pack, void *userData=nullptr)
{
    SharedTextureArray textureArray(new TextureArray());

    const std::vector<TextureInfo> &textureInfo=pack.textureInfo();
    std::vector<size_t> textureMap(textureInfo.size(), 0);
    std::vector<size_t> blockIds(blocks.size());
    std::vector<size_t> textureIds;

    textureArray->m_textureResolution=pack.getResolution();

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
    textureArray->m_textureEntries.resize(textureCount);

    size_t maxWidth=0;
    size_t maxHeight=0;

    size_t textureTileCount=0;
    for(size_t i=0; i<textureCount; ++i)
    {
        const TextureInfo &info=textureInfo[textureIds[i]];
        const LayerInfo &baseLayer=*(info.base);
        std::string imagePath=packPath+"/"+baseLayer.path;

        ImageEntry &entry=imageEntries[i];
        imglib::SimpleImage &image=entry.image;

        if(!imglib::load<_FileIO>(image, imagePath, userData))
            continue;

        entry.index=i;
        entry.method=baseLayer.method;
        entry.tileX=image.width/textureArray->m_textureResolution;
        entry.tileY=image.height/textureArray->m_textureResolution;

        if(image.width>maxWidth)
            maxWidth=image.width;
        if(image.height>maxHeight)
            maxHeight=image.height;

        entry.tileCount=entry.tileX*entry.tileY;
        textureTileCount+=entry.tileCount;
    }

    size_t maxTextureTilesX=textureArray->m_maxTextureWidth/textureArray->m_textureResolution;
    size_t maxTextureTilesY=textureArray->m_maxTextureHeight/textureArray->m_textureResolution;
    size_t tilesX=0;
    size_t tilesY=0;
    size_t tilePosX=0;
    size_t tilePosY=0;

    Regular2DGrid<bool> tiles(maxTextureTilesX, maxTextureTilesY, false);
    //    std::vector<bool> tiles(maxTilesX*maxTilesY, false);
    imglib::SimpleImage textureImage;

    textureImage.allocData(imglib::Format::RGBA, imglib::Depth::Bit8, textureArray->m_maxTextureWidth, textureArray->m_maxTextureHeight);

    //todo:need to take care of multiple resolution textures
    //copy textures over to atlas texture and update entries with position data
//    imglib::ReuseImage image;

    std::vector<size_t> sortedIndex(textureArray->m_textureEntries.size());
    std::sort(imageEntries.begin(), imageEntries.end(), [](const ImageEntry &entry1, const ImageEntry &entry2){return entry1.tileCount>entry2.tileCount; });

    int maxTilesX=0;
    int maxTilesY=0;

    for(size_t i=0; i<textureArray->m_textureEntries.size(); ++i)
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
        entry.x=tilePos.x*textureArray->m_textureResolution;
        entry.y=tilePos.y*textureArray->m_textureResolution;

        imglib::copyTo(textureImage, entry.x, entry.y, image);

        //        posX++;
        //        if(posX>=x)
        //        {
        //            posX=0;
        //            posY++;
        //        }
    }

    textureArray->m_width=maxTilesX*textureArray->m_textureResolution;
    textureArray->m_height=maxTilesY*textureArray->m_textureResolution;

    textureArray->m_image.allocData(imglib::Format::RGBA, imglib::Depth::Bit8, textureArray->m_width, textureArray->m_height);
    imglib::copyTo(textureArray->m_image, 0, 0, textureImage);

    //build up block information base on the atlas texture
    textureArray->m_blockEntries.resize(blockIds.size());

    for(size_t i=0; i<blockIds.size(); ++i)
    {
        if(blockIds[i]==0)
            continue;

        const BlockInfo &blockInfo=pack.getBlockInfo(blockIds[i]);
        TextureArray::BlockEntry &blockEntry=textureArray->m_blockEntries[i];

        textureArray->m_blockMap.emplace(blocks[i], i);
        for(size_t face=0; face<6; ++face)
        {
            size_t id=blockInfo.faces[face];
            size_t textureEntryId=textureMap[id];
            textureEntryId=sortedIndex[textureEntryId];

            if(textureEntryId==0)
                continue;

            //            TextureEntry &entry=textureArray->m_textureEntries[textureEntryId];
            ImageEntry &entry=imageEntries[textureEntryId];

            blockEntry.faces[face]=entry;
        }
    }

    return textureArray;
}
}//namespace voxigen

#endif //_voxigen_textureArray_h_