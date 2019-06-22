#include "voxigen/texturePack.h"
#include <generic/jsonSerializer.h>

#include <sstream>
#include <set>
#include <cassert>

namespace voxigen
{

TexturePack::TexturePack()
{}

TexturePack::~TexturePack()
{}

bool TexturePack::load(const std::string &path)
{
    if(!loadConfig(path+"/pack.json"))
        return false;

    if(!loadLayers(path+"/layerProperties.json"))
        return false;

    if(!loadBlocks(path+"/blockTextureMapping.json"))
        return false;

    m_path=path;

    return true;
}

bool TexturePack::loadConfig(const std::string &path)
{
    generic::JsonDeserializer deserializer;

    if(!deserializer.open(path.c_str()))
        return false;

    deserializer.openObject();
    if(deserializer.key("version"))
        m_version=deserializer.getInt();

    if(deserializer.key("name"))
        m_name=deserializer.getString();
    else
        m_name="unknown";

    if(deserializer.key("description"))
        m_description=deserializer.getString();

    if(deserializer.key("resolution"))
        m_resolution=deserializer.getInt();
    else
        m_resolution=32;

    deserializer.closeObject();
}

bool TexturePack::loadLayers(const std::string &path)
{
    generic::JsonDeserializer deserializer;

    if(!deserializer.open(path.c_str()))
        return false;

    deserializer.openObject();
//    if(deserializer.key("version"))
//        m_version=deserializer.getInt();

    //create empty layer at 0 index to use as invalid
    {
        LayerInfo layer;

        layer.name="invalid";
        m_layerMap.emplace(layer.name, 0);
        m_layerInfo.push_back(layer);
    }

    if(deserializer.key("layers"))
    {
        if(deserializer.openArray())
        {
            size_t unknownIndex=0;

            do
            {
                if(!deserializer.openObject())
                    continue;

                LayerInfo layer;

                if(deserializer.key("name"))
                    layer.name=deserializer.getString();
                else
                {
                    layer.name=std::string("unknown")+std::to_string(unknownIndex);
                    unknownIndex++;
                }

                if(deserializer.key("path"))
                    layer.path=deserializer.getString();

                if(deserializer.key("normalPath"))
                    layer.normalPath=deserializer.getString();

                if(deserializer.key("dispPath"))
                    layer.dispPath=deserializer.getString();

                if(deserializer.key("specularPath"))
                    layer.specularPath=deserializer.getString();

                if(deserializer.key("method"))
                {
                    std::string methodName=deserializer.getString();

                    layer.method=getLayerMethod(methodName);
                }

                if(deserializer.key("color"))
                {
                    if(deserializer.openArray())
                    {
                        size_t index=0;

                        do
                        {
                            layer.color[index]=deserializer.getFloat();
                            index++;
                        } while(deserializer.advance() && (index<3));
                        deserializer.closeArray();
                    }
                }

                if(deserializer.key("altColor"))
                {
                    if(deserializer.openArray())
                    {
                        size_t index=0;

                        do
                        {
                            layer.altColor[index]=deserializer.getFloat();
                            index++;
                        } while(deserializer.advance()&&(index<3));
                        deserializer.closeArray();
                    }
                }

                if(deserializer.key("innerSeams"))
                    layer.innerSeams=deserializer.getBool();

                if(deserializer.key("coupling"))
                {
                    if(deserializer.openArray())
                    {
                        do
                        {
                            layer.coupling.push_back(deserializer.getString());
                        } while(deserializer.advance());

                        deserializer.closeArray();
                    }
                }

                if(deserializer.key("weights"))
                {
                    if(deserializer.openArray())
                    {
                        do
                        {
                            layer.weights.push_back(deserializer.getFloat());
                        } while(deserializer.advance());

                        deserializer.closeArray();
                    }
                }

                deserializer.closeObject();
                    
                m_layerMap.emplace(layer.name, m_layerInfo.size());
                m_layerInfo.push_back(layer);
            } while(deserializer.advance());
            deserializer.closeArray();
        }
    }
    return true;
}

TextureInfo TexturePack::getTextureInfo(generic::Deserializer &deserializer)
{
    TextureInfo texture;

    if(!deserializer.openObject())
        return texture;

    if(deserializer.key("base"))
    {
        std::string layerName=deserializer.getString();
        size_t layerId=getLayerInfoId(layerName);

        if(layerId)
            texture.base=&m_layerInfo[layerId];
        else
            texture.base=nullptr;
    }
    else
        texture.base=nullptr;

    if(deserializer.key("overlay"))
    {
        std::string layerName=deserializer.getString();
        size_t layerId=getLayerInfoId(layerName);

        if(layerId)
            texture.overlay=&m_layerInfo[layerId];
        else
            texture.overlay=nullptr;
    }
    else
        texture.overlay=nullptr;

    if(deserializer.key("blendMode"))
        texture.blendMethod=getBlendMethod(deserializer.getString());
    else
        texture.blendMethod=BlendMethod::alpha;

    deserializer.closeObject();

    return texture;
}

bool TexturePack::loadBlocks(const std::string &path)
{
    generic::JsonDeserializer deserializer;

    if(!deserializer.open(path.c_str()))
        return false;

    deserializer.openObject();

    size_t version;
    
    if(deserializer.key("version"))
        version=deserializer.getInt();

    if(version!=1)
        return false;

    //create empty block(and texture) at 0 index to use as invalid
    {
        BlockInfo block;

        block.name="invalid";

        m_blockMap.emplace(block.name, 0);
        m_blockInfo.push_back(block);

        TextureInfo texture;

        texture.base=nullptr;
        texture.overlay=nullptr;
        texture.blendMethod=BlendMethod::unknown;
        
        m_textureInfo.push_back(texture);
        m_textureMap.emplace(TextureKey(&m_textureInfo, 0), 0);
    }

    if(!deserializer.key("blocks"))
        return true;

    if(!deserializer.openArray())
        return true;

    size_t unknownIndex=0;

    do
    {
        if(!deserializer.openObject())
            continue;

        BlockInfo block;

        if(deserializer.key("name"))
            block.name=deserializer.getString();
        else
        {
            block.name=std::string("unknown")+std::to_string(unknownIndex);
            unknownIndex++;
        }

        if(deserializer.key("transparency"))
            block.transparency=deserializer.getBool();
        else
            block.transparency=false;

        std::vector<std::pair<std::string, std::vector<size_t>>> locations=
        {
            {"texture", {Face::left, Face::right, Face::front, Face::back, Face::bottom, Face::top}},
            {"textureOpX", {Face::left, Face::right}},
            {"textureOpY", {Face::front, Face::back}},
            {"textureOpZ", {Face::bottom, Face::top}},
            {"textureTop", {Face::top}},
            {"textureBottom", {Face::bottom}},
            {"textureFront", {Face::front}},
            {"textureBack", {Face::back}},
            {"textureLeft", {Face::left}},
            {"textureRight", {Face::right}}
        };

        std::set<size_t> textureSet;

        for(auto &location:locations)
        {
            if(deserializer.key(location.first))
            {
                TextureInfo texture=getTextureInfo(deserializer);
                size_t id;
                

                TextureKey key(&texture);
                auto textureIter=m_textureMap.find(key);

                if(textureIter != m_textureMap.end())
                    id=textureIter->second;
                else
                {
                    id=m_textureInfo.size();
                    
                    m_textureInfo.push_back(texture);
                    TextureKey finalKey(&m_textureInfo, id);
                    m_textureMap.emplace(finalKey, id);
                    
                }

                textureSet.insert(id);
                for(auto i:location.second)
                    block.faces[i]=id;

                block.texturesUsed=textureSet.size();
            }
        }

        deserializer.closeObject();

        m_blockMap.emplace(block.name, m_blockInfo.size());
        m_blockInfo.push_back(block);

    } while(deserializer.advance());

    deserializer.closeArray();

    return true;
}

size_t TexturePack::getLayerInfoId(std::string name)
{
    auto iter=m_layerMap.find(name);

    if(iter!=m_layerMap.end())
        return iter->second;

    return 0;
}

const std::vector<TextureInfo> &TexturePack::textureInfo() const
{
    return m_textureInfo;
}

const TextureInfo &TexturePack::getTextureInfo(size_t id) const
{
    assert(id<m_textureInfo.size());

    return m_textureInfo[id];
}

const BlockInfo &TexturePack::getBlockInfo(size_t id) const
{
    assert(id<m_blockInfo.size());

    return m_blockInfo[id];
}

const BlockInfo &TexturePack::getBlockInfo(std::string name) const
{
    size_t id=getBlockInfoId(name);

    assert(id<m_blockInfo.size());
    return m_blockInfo[id];
}

size_t TexturePack::getBlockInfoId(std::string name) const
{
    auto iter=m_blockMap.find(name);
    
    if(iter!=m_blockMap.end())
        return iter->second;

    return 0;
}

size_t TexturePack::getResolution() const
{
    return m_resolution;
}

std::string TexturePack::getPath() const
{
    return m_path;
}

}//namespace voxigen
