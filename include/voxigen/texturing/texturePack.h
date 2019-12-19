#ifndef _voxigen_texturePack_h_
#define _voxigen_texturePack_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/texturing/blendMethod.h"
#include "voxigen/texturing/layerMethod.h"
#include "voxigen/meshes/faces.h"

#include <generic/serializer.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace voxigen
{

struct LayerInfo
{
    std::string name;
    std::string path;
    std::string normalPath;
    std::string dispPath;
    std::string specularPath;

    LayerMethod method;
    bool innerSeams;
    std::vector<std::string> coupling;

    glm::vec3 color;
    glm::vec3 altColor;

    std::vector<float> weights;
};

struct TextureInfo
{
    LayerInfo *base;
    LayerInfo *overlay;

    BlendMethod blendMethod;
};
struct TextureKey
{
    TextureKey(TextureInfo *info):textures(nullptr), id(0), textureInfo(info) {}
    TextureKey(std::vector<TextureInfo> *textures, size_t id):textures(textures), id(id), textureInfo(nullptr) {}

    bool operator==(const TextureKey &key) const
    {
        TextureInfo *info1;
        TextureInfo *info2;

        if(textureInfo)
            info1=textureInfo;
        else
            info1=&(*textures)[id];

        if(key.textureInfo)
            info2=key.textureInfo;
        else
            info2=&(*key.textures)[key.id];

        if(info1->base!=info2->base)
            return false;
        if(info1->overlay!=info2->overlay)
            return false;
        if(info1->blendMethod!=info2->blendMethod)
            return false;
        return true;
    }

    std::vector<TextureInfo> *textures;
    size_t id;
    TextureInfo *textureInfo;
};

}//namespace voxigen

//create hash for TextureKey
namespace std
{

template<>
struct hash<voxigen::TextureKey>
{
    std::size_t operator()(const voxigen::TextureKey &key) const
    {
        voxigen::TextureInfo *info;

        if(key.textureInfo)
            info=key.textureInfo;
        else
            info=&(*key.textures)[key.id];

        return ((std::hash<voxigen::LayerInfo *>()(info->base)
            ^(std::hash<voxigen::LayerInfo *>()(info->overlay)<<1))>>1)
            ^(std::hash<voxigen::BlendMethod>()(info->blendMethod)<<1);
    }
};

}//namespace std

namespace voxigen
{

struct BlockInfo
{
    std::string name;
    bool transparency;

    std::vector<TextureInfo> textures;
    std::array<size_t, 6> faces;
    size_t texturesUsed;
};

typedef std::unordered_map<std::string, size_t> LayerMap;
typedef std::unordered_map<std::string, size_t> BlockMap;

class VOXIGEN_EXPORT TexturePack
{
public:
    TexturePack();
    ~TexturePack();

//    template<typename _FileIO=generic::StdFileIO>
//    bool load(const std::string &path, void *userData=nullptr);

    size_t getLayerInfoId(std::string name);

    const std::vector<TextureInfo> &textureInfo() const;

    const TextureInfo &getTextureInfo(size_t id) const;
    const BlockInfo &getBlockInfo(size_t id) const;
    const BlockInfo &getBlockInfo(std::string name) const;

    size_t getBlockInfoId(std::string name) const;

    size_t getResolution() const;
    std::string getPath() const;

    template<typename _FileIO>
    friend std::shared_ptr<TexturePack> generateTexturePack(const std::string &path, void *userData);

private:
    bool loadConfig(generic::Deserializer &deserializer);
    bool loadLayers(generic::Deserializer &deserializer);
    bool loadBlocks(generic::Deserializer &deserializer);

    TextureInfo getTextureInfo(generic::Deserializer &deserializer);

//these items are private and not used outside of the class api
#pragma warning(push)
#pragma warning(disable:4251)
    std::string m_name;
    std::string m_description;
    std::string m_path;

    size_t m_version;
    size_t m_resolution;

    LayerMap m_layerMap;
    std::vector<LayerInfo> m_layerInfo;

    BlockMap m_blockMap;
    std::vector<BlockInfo> m_blockInfo;

    std::unordered_map<TextureKey, size_t> m_textureMap;
    std::vector<TextureInfo> m_textureInfo;
#pragma warning(pop)
};

typedef std::shared_ptr<TexturePack> SharedTexturePack;

template<typename _FileIO=generic::StdFileIO>
inline SharedTexturePack generateTexturePack(const std::string &path, void *userData=nullptr)
{
    std::shared_ptr<TexturePack> texturePack(new TexturePack());
    generic::JsonDeserializer deserializer;

    typename _FileIO::Type *pathFile=generic::open<_FileIO>(path+"/pack.json", "rb", userData);

    if(pathFile==nullptr)
        return false;

    typename _FileIO::Type *layerFile=generic::open<_FileIO>(path+"/layerProperties.json", "rb", userData);

    if(pathFile==nullptr)
    {
        generic::close<_FileIO>(pathFile);
        return false;
    }

    typename _FileIO::Type *textureFile=generic::open<_FileIO>(path+"/blockTextureMapping.json", "rb", userData);

    if(pathFile==nullptr)
    {
        generic::close<_FileIO>(pathFile);
        generic::close<_FileIO>(layerFile);
        return false;
    }

    size_t packFileSize=generic::size<_FileIO>(pathFile);
    size_t layerFileSize=generic::size<_FileIO>(layerFile);
    size_t textureFileSize=generic::size<_FileIO>(textureFile);

    size_t bufferSize=std::max(packFileSize+1, std::max(layerFileSize+1, textureFileSize+1));
    std::string buffer(bufferSize, 0);
    size_t readSize;

//load pack info
    readSize=generic::read<_FileIO>((void *)buffer.data(), sizeof(uint8_t), packFileSize, pathFile);

    if(readSize==0)
    {
        generic::close<_FileIO>(pathFile);
        generic::close<_FileIO>(layerFile);
        generic::close<_FileIO>(textureFile);
        return false;
    }

    buffer[readSize]=0;
    deserializer.parse(buffer);
    texturePack->loadConfig(deserializer);

//load layer information
    readSize=generic::read<_FileIO>((void *)buffer.data(), sizeof(uint8_t), layerFileSize, layerFile);

    if(readSize==0)
    {
        generic::close<_FileIO>(pathFile);
        generic::close<_FileIO>(layerFile);
        generic::close<_FileIO>(textureFile);
        return false;
    }

    buffer[readSize]=0;
    deserializer.parse(buffer);
    texturePack->loadLayers(deserializer);

//load block textures
    readSize=generic::read<_FileIO>((void *)buffer.data(), sizeof(uint8_t), textureFileSize, textureFile);

    if(readSize==0)
    {
        generic::close<_FileIO>(pathFile);
        generic::close<_FileIO>(layerFile);
        generic::close<_FileIO>(textureFile);
        return false;
    }

    buffer[readSize]=0;
    deserializer.parse(buffer);
    texturePack->loadBlocks(deserializer);
    
    generic::close<_FileIO>(pathFile);
    generic::close<_FileIO>(layerFile);
    generic::close<_FileIO>(textureFile);

    texturePack->m_path=path;
    return texturePack;
}

}//namespace voxigen



#endif //_voxigen_texturePack_h_