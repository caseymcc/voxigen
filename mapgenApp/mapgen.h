#ifndef _voxigen_mapgen_h_
#define _voxigen_mapgen_h_

#include "./testApp/world.h"
#include "colorMap.h"

#include <glbinding/gl/gl.h>
using namespace gl;

class MapGen
{
public:
    MapGen();
    
    void initialize();
    void setSize(int width, int height);

    void draw();

private:
    void updateTexture();
    void updatePlateTexture(std::vector<GLubyte> &textureBuffer);
    void updatePlateDistanceTexture(std::vector<GLubyte> &textureBuffer);
    void updateContinentTexture(std::vector<GLubyte> &textureBuffer);
    void updateGeometryTexture(std::vector<GLubyte> &textureBuffer);

    void generate();

    bool m_show;

    int m_width;
    int m_height;

    World m_world;
    int m_worldWidth;
    int m_worldHeight;
    int m_worldDepth;

    WorldGenerator *m_worldGenerator;
    int m_noiseSeed;
    float m_plateFrequency;
    float m_continentFrequency;

    bool m_textureValid;
    GLuint m_textureId;
    int m_textureWidth;
    int m_textureHeight;

    int m_layerIndex;
    std::vector<char> m_layerNames;

    std::vector<std::tuple<int, int, int>> m_plateColors;
    voxigen::ColorMap m_colorMap;
};

#endif//_voxigen_mapgen_h_