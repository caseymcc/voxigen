#ifndef _voxigen_textureAtlas_h_
#define _voxigen_textureAtlas_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/texturePack.h"

#include <imglib/simpleImage.h>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace voxigen
{

//need empty function for regular2DGrid
inline bool empty(bool value) { return !value; }

class VOXIGEN_EXPORT TextureAtlas
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

    TextureAtlas(size_t maxTextureWidth=4096, size_t maxTextureHeight=4096);
    ~TextureAtlas();

    void load(const std::string &path, const std::string &name);
    void save(const std::string &path, const std::string &name);

    bool build(const std::vector<std::string> &blocks , const TexturePack &pack);

    const BlockEntry &getBlockEntry(size_t index) const;

    const imglib::SimpleImage &getImage() const { return m_image; }

    size_t resolution() const { return m_textureResolution; }

private:
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
};

typedef std::shared_ptr<TextureAtlas> SharedTextureAtlas;

}//namespace voxigen

#endif //_voxigen_textureAtlas_h_