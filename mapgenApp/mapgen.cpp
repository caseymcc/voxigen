#include "mapgen.h"

#include "randomcolor.h"
#include "colorMap.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <tuple>

std::vector<char> packVectorString(const std::vector<std::string> &values)
{
    std::vector<char> packed;

    size_t size=0;
    for(const std::string &value:values)
        size+=value.size()+1;//null terminator

    packed.resize(size+1);

    size=0;
    for(const std::string &value:values)
    {
        memcpy(&packed[size], value.c_str(), value.size());
        size+=value.size();
        packed[size]=0;
        size++;
    }
    packed[size]=0;

    return packed;
}

MapGen::MapGen():
    m_textureValid(false),
    m_show(true)
{
    m_worldWidth=4194304;
    m_worldHeight=2097152;
    m_worldDepth=2560;

    m_colorMap=voxigen::generateColorMap(64, 64);
}

void MapGen::initialize()
{
//    fs::path worldsDirectory("worlds");

    m_world.create("", "TestApWorld", glm::ivec3(m_worldWidth, m_worldHeight, m_worldDepth), "EquiRectWorldGenerator");

    //pull out generator so we can work on it directly
    WorldGeneratorTemplate *genTemplate=(WorldGeneratorTemplate *)&m_world.getGenerator();
    m_worldGenerator=(WorldGenerator *)genTemplate->get();

    updateTexture();

    m_layerIndex=0;
    m_layerNames=packVectorString({"Tectonic Plates", "Plates Distance", "Continents"});
//    if(!fs::exists(worldsDirectory))
//        fs::create_directory(worldsDirectory);
//
//    std::vector<fs::directory_entry> worldDirectories;
//
//    for(auto &entry:fs::directory_iterator(worldsDirectory))
//        worldDirectories.push_back(entry);
//
//    if(worldDirectories.empty())
//    {
//        std::string worldDirectory=worldsDirectory.string()+"/TestApWorld";
//        fs::path worldPath(worldDirectory);
//
//        fs::create_directory(worldPath);
//
//        //creating earth sizes
//        world.create(worldDirectory, "TestApWorld", glm::ivec3(20971520, 10485760, 2560), "EquiRectWorldGenerator");
//    }
//    else
//        world.load(worldDirectories[0].path().string());

    m_colorMap=voxigen::generateColorMap(64, 64);
}

void MapGen::setSize(int width, int height)
{
    m_width=width;
    m_height=height;
}

void MapGen::draw()
{
    voxigen::EquiRectDescriptors &descriptors=m_worldGenerator->getDecriptors();

    int controlsWidth=350;

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)controlsWidth, (float)m_height});

    ImGui::Begin("Controls", &m_show, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
    
    if(ImGui::Button("Generate"))
        generate();

    ImGui::Separator();

    ImGui::InputInt("Seed", &m_noiseSeed);
    ImGui::SliderFloat("Plate Frequency", &descriptors.m_plateFrequency, 0.0001f, 1.0f, "%.4f", 3.0f);
    ImGui::SliderFloat("Continent Frequency", &descriptors.m_continentFrequency, 0.001f, 1.0f, "%.3f", 3.0f);

    ImGui::Separator();

    if(ImGui::Combo("Layer", &m_layerIndex, &m_layerNames[0]))
    {
        updateTexture();
    }

    ImGui::End();

    int imageWidth=std::max(m_width-controlsWidth, 0);

    ImGui::SetNextWindowPos({(float)controlsWidth, 0});
    ImGui::SetNextWindowSize({(float)imageWidth, (float)m_height});

    ImGui::Begin("Image", &m_show, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
    
    if(m_textureValid)
    {
        int imageHeight=m_height-20;//margin
        
        imageWidth-=20;//margin
        ImGui::Image((ImTextureID)m_textureId, {(float)imageWidth, (float)imageHeight}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.5f});

    }
    
    ImGui::End();
}

void MapGen::updateTexture()
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();

    std::vector<GLubyte> textureBuffer;

    textureBuffer.resize(influenceMap.size()*4);

    //    int min=std::numeric_limits<int>::max();
    //    int max=0;
    //
    //    for(size_t i=0; i<influenceMap.size(); ++i)
    //    {
    //        int value=influenceMap[i].tectonicPlate;
    //
    //        min=std::min(min, value);
    //        max=std::max(max, value);
    //    }

    if(m_layerIndex==0)
        updatePlateTexture(textureBuffer);
    else if(m_layerIndex==1)
        updatePlateDistanceTexture(textureBuffer);
    else
        updateContinentTexture(textureBuffer);
}

void MapGen::updatePlateTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();

    int plateCount=m_worldGenerator->getPlateCount();

    RandomColor::RandomColorGenerator colorGenerator;

    if(m_plateColors.size() <plateCount)
        m_plateColors=colorGenerator.randomColors(plateCount);

    size_t index=0;
    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        auto &color=m_plateColors[influenceMap[i].tectonicPlate];

        textureBuffer[index++]=(GLubyte)std::get<0>(color);
        textureBuffer[index++]=(GLubyte)std::get<1>(color);
        textureBuffer[index++]=(GLubyte)std::get<2>(color);
        textureBuffer[index++]=255;
    }

    m_textureWidth=influenceMapSize.x;
    m_textureHeight=influenceMapSize.y;

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureBuffer[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    m_textureValid=true;
}

void MapGen::updatePlateDistanceTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
    
    float min=std::numeric_limits<float>::max();
    float max=0.0f;

    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        float value=influenceMap[i].plateDistanceValue;

        min=std::min(min, value);
        max=std::max(max, value);
    }

    float delta=255/(max-min);
    size_t index=0;

    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        GLubyte value=std::max(0, std::min(255, (int)((influenceMap[i].plateDistanceValue-min)*delta)));

        textureBuffer[index++]=value;
        textureBuffer[index++]=value;
        textureBuffer[index++]=value;
        textureBuffer[index++]=255;
    }

    m_textureWidth=influenceMapSize.x;
    m_textureHeight=influenceMapSize.y;

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureBuffer[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    m_textureValid=true;
}

void MapGen::updateContinentTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();

    int plateCount=m_worldGenerator->getPlateCount();

    RandomColor::RandomColorGenerator colorGenerator;

    if(m_plateColors.size()<plateCount)
        m_plateColors=colorGenerator.randomColors(plateCount);

    size_t index=0;
    
    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        int elevation=32+32*influenceMap[i].continentValue;
        glm::ivec4 color=m_colorMap.color(elevation, 32);
        
        textureBuffer[index++]=(GLubyte)color.r;
        textureBuffer[index++]=(GLubyte)color.g;
        textureBuffer[index++]=(GLubyte)color.b;
        textureBuffer[index++]=255;
    }

    m_textureWidth=influenceMapSize.x;
    m_textureHeight=influenceMapSize.y;

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureBuffer[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    m_textureValid=true;
}

void MapGen::generate()
{
    m_worldGenerator->generateWorldOverview();

    updateTexture();
}