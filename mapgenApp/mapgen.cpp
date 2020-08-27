#include "mapgen.h"

#include "randomcolor.h"
#include "colorMap.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <tuple>

#include <imglib/draw.h>

LoadProgress g_loadProgress;

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

    voxigen::ColorScale tempColorScale;

    tempColorScale.addColorPosition(glm::ivec4(255, 255, 255, 255), 0.0f);
    tempColorScale.addColorPosition(glm::ivec4(255, 0, 255, 255), 0.2f);
    tempColorScale.addColorPosition(glm::ivec4(0, 0, 255, 255), 0.4f);
    tempColorScale.addColorPosition(glm::ivec4(0, 255, 0, 255), 0.6f);
    tempColorScale.addColorPosition(glm::ivec4(255, 0, 0, 255), 0.8f);
    tempColorScale.addColorPosition(glm::ivec4(0, 0, 0, 255), 1.0f);

    m_tempColorMap=voxigen::generateColorMap(tempColorScale, 64);

    voxigen::ColorScale moistureColorScale;

    moistureColorScale.addColorPosition(glm::ivec4(255, 0, 0, 255), 0.0f);
    moistureColorScale.addColorPosition(glm::ivec4(0, 255, 0, 255), 0.5f);
    moistureColorScale.addColorPosition(glm::ivec4(0, 0, 255, 255), 1.0f);

    m_moistureColorMap=voxigen::generateColorMap(moistureColorScale, 64);

    m_biomeColorMap=voxigen::generateHieghtMoistureColorMap(64, 64);

    m_plateCount=16;
}

void MapGen::initialize()
{
//    fs::path worldsDirectory("worlds");
    //pull out generator so we can work on it directly
    m_world.create("", "TestApWorld", glm::ivec3(m_worldWidth, m_worldHeight, m_worldDepth), "EquiRectWorldGenerator", g_loadProgress);

    WorldGeneratorTemplate *genTemplate=(WorldGeneratorTemplate *)&m_world.getGenerator();
    m_worldGenerator=(WorldGenerator *)genTemplate->get();

//    m_worldGenerator->m_plateCount=m_plateCount;

    m_layerIndex=0;
    m_info=0;
    m_overlay=0;
    m_overlayVector=0;
    m_layerNames=packVectorString({"Terrain", "Plate Info", "Tectonic Plates", "Plates Distance", "Terrain Scale"});

	updateTexture();
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

//    m_biomeColorMap=voxigen::generateHieghtMoistureColorMap(64, 64);
}

void MapGen::setSize(int width, int height)
{
    m_width=width;
    m_height=height;
}

void MapGen::draw()
{
    bool forceUpdate=false;

    voxigen::EquiRectDescriptors &descriptors=m_worldGenerator->getDecriptors();

    int controlsWidth=350;

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)controlsWidth, (float)m_height});

    ImGui::Begin("Controls", &m_show, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
    
    if(ImGui::Button("Generate"))
    {
        generate();
        forceUpdate=true;
    }

    ImGui::Separator();

    ImGui::InputInt("Seed", &m_noiseSeed);
    if(ImGui::SliderInt("Plate Count", &m_plateCount, 1000, 10000))
    {
        generate();
        forceUpdate=true;
    }
    ImGui::SliderFloat("Plate Frequency", &descriptors.m_plateFrequency, 0.0001f, 1.0f, "%.4f", 3.0f);
    ImGui::SliderFloat("Continent Frequency", &descriptors.m_continentFrequency, 0.001f, 1.0f, "%.3f", 3.0f);

    ImGui::Separator();

    if(ImGui::Combo("Layer", &m_layerIndex, &m_layerNames[0]))
        forceUpdate=true;

    int info=m_info;
    ImGui::Text("Terrain");
    ImGui::RadioButton("Map", &info, 0);
    ImGui::RadioButton("Height", &info, 1);
    ImGui::RadioButton("Value", &info, 2);
    ImGui::RadioButton("Plates", &info, 3);
    ImGui::RadioButton("No Map", &info, 4);
    if(info!=m_info)
    {
        m_info=info;
        forceUpdate=true;
    }

    int overlay=m_overlay;
    ImGui::Text("Overlay");
    ImGui::RadioButton("None", &overlay, 0);
    ImGui::RadioButton("Collision", &overlay, 1);
    ImGui::RadioButton("Collision-Distance", &overlay, 2);
    ImGui::RadioButton("Scale", &overlay, 3);
    ImGui::RadioButton("Temp", &overlay, 4);
    ImGui::RadioButton("Moisture", &overlay, 5);
    ImGui::RadioButton("Moisture GreyScale", &overlay, 6);
    ImGui::RadioButton("Weather Cells", &overlay, 7);
    ImGui::RadioButton("Weather Bands", &overlay, 8);

    if(overlay!=m_overlay)
    {
        m_overlay=overlay;
        forceUpdate=true;
    }

    int overlayVector=m_overlayVector;
    ImGui::Text("Vector");
    ImGui::RadioButton("No Vector", &overlayVector, 0);
    ImGui::RadioButton("Direction", &overlayVector, 1);
    ImGui::RadioButton("Air", &overlayVector, 2);

    if(overlayVector!=m_overlayVector)
    {
        m_overlayVector=overlayVector;
        forceUpdate=true;
    }

//    if(ImGui::Checkbox("overlay", &m_overlay))
//        forceUpdate=true;

    ImVec2 mousePosition=ImGui::GetMousePos();

    mousePosition.x=mousePosition.x-controlsWidth-lastDrawPos.x-1.0f;//-1.of border
    mousePosition.y=mousePosition.y-lastDrawPos.y-1.0f;//-1.of border

    if(mousePosition.x<0.0f)
        mousePosition.x=0.0f;
    if(mousePosition.y<0.0f)
        mousePosition.y=0.0f;

    if(mousePosition.x>lastDrawSize.x-1)
        mousePosition.x=lastDrawSize.x-1;
    if(mousePosition.y>lastDrawSize.y-1)
        mousePosition.y=lastDrawSize.y-1;

    int texturePosX=mousePosition.x/lastDrawSize.x*m_textureWidth;
    int texturePosY=mousePosition.y/lastDrawSize.y*m_textureHeight;

    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
    size_t index=texturePosY*influenceMapSize.x+texturePosX;
    float value;
    float overlayValue;

    if((index<0)||(index>=influenceMapSize.x*influenceMapSize.y))
        index=0;

    if((m_info==0) || (m_info==1))
        value=influenceMap[index].heightBase;
    else if(m_info==2)
        value=influenceMap[index].plateHeight;
    else if(m_info==3)
        value=(float)influenceMap[index].tectonicPlate;
    else
        value=0.0f;

    ImGui::Text("Position: %d, %d : %f", texturePosX, texturePosY, value);

    if(m_overlay > 0)
    {
        overlayValue=0.0f;

        if(m_overlay==1)
            overlayValue=influenceMap[index].collision;
        else if(m_overlay==2)
            overlayValue=influenceMap[index].collision*influenceMap[index].plateDistanceValue;
        else if(m_overlay==3)
            overlayValue=influenceMap[index].terrainScale;
        else if(m_overlay==4)
            overlayValue=(influenceMap[index].temperature+90.0f)/160.0f;
        else if((m_overlay==5) || (m_overlay==6))
            overlayValue=std::min(influenceMap[index].moisture, 1.0f);
        else if(m_overlay==7)
            overlayValue=(float)influenceMap[index].weatherCell;
        else if(m_overlay==8)
            overlayValue=(float)influenceMap[index].weatherBand;
        ImGui::Text("Overlay: %f", overlayValue);
    }

    ImGui::End();

    if(forceUpdate)
        updateTexture();

    int imageWindowWidth=std::max(m_width-controlsWidth, 0);

    ImGui::SetNextWindowPos({(float)controlsWidth, 0});
    ImGui::SetNextWindowSize({(float)imageWindowWidth, (float)m_height});

    ImGui::Begin("Image", &m_show, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);

    lastDrawPos=ImGui::GetCursorPos();
    int imageWidth=imageWindowWidth-(2*lastDrawPos.y)-2.0f;//margin, -2.0f border
    int imageHeight=m_height-(2*lastDrawPos.y)-2.0f;//margin, -2.0f border

    float ratioX=(float)imageWidth/m_textureWidth;
    float ratioY=(float)imageHeight/m_textureHeight;

    ImVec2 textureStart=lastDrawPos;
    ImVec2 textureSize(imageWidth, imageHeight);

    if(ratioX<ratioY)
    {
        textureSize.y=ratioX*m_textureHeight;
        textureStart.y=lastDrawPos.y+(imageHeight-textureSize.y)/2.0f;
    }
    else
    {
        textureSize.x=ratioY*m_textureWidth;
        textureStart.x=lastDrawPos.x+(imageWidth-textureSize.x)/2.0f;
    }
    lastDrawPos=textureStart;
    ImGui::SetCursorPos(textureStart);

    lastDrawSize.x=textureSize.x;
    lastDrawSize.y=textureSize.y;

    if(m_textureValid)
    {
        ImGui::Image((ImTextureID)m_textureId, {(float)textureSize.x, (float)textureSize.y}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.5f});
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
    m_layerNames=packVectorString({"Terrain", "Plate Info", "Tectonic Plates", "Plates Distance"});

    if(m_layerIndex==0)
        updatePlateInfoTexture(textureBuffer);
    if(m_layerIndex==1)
        updatePlateInfoTexture(textureBuffer);
    else if(m_layerIndex==2)
        updatePlateTexture(textureBuffer);
    else if(m_layerIndex==3)
        updatePlateDistanceTexture(textureBuffer);
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

void MapGen::updatePlateInfoTexture(std::vector<GLubyte> &textureBuffer)
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
        glm::ivec4 color;
        
        if(m_info==0)
        {
            color=m_biomeColorMap.color((size_t)64*influenceMap[i].heightBase, (size_t)64*influenceMap[i].moisture);

            float value=(influenceMap[i].temperature+90.0f)/160.0f;

            if((value<0.4f) && (influenceMap[i].heightBase>0.5f)) //polar caps
            {
                if(value<0.2f)
                {
                    color.r=255;
                    color.g=255;
                    color.b=255;
                }
                else
                {
                    value=5.0f*(0.4f-value);
                    color.r=(255*value)+(color.r*(1.0f-value));
                    color.g=(255*value)+(color.g*(1.0f-value));
                    color.b=(255*value)+(color.b*(1.0f-value));
                }
            }
        }
        else if(m_info==1)
        {
            unsigned char value=255*influenceMap[i].heightBase;

            color.r=value;
            color.g=value;
            color.b=value;
            color.a=255;
        }
        else if(m_info==2)
            color=m_biomeColorMap.color((size_t)64*influenceMap[i].plateHeight, 32);
        else if(m_info==3)
        {
            auto &plateColor=m_plateColors[influenceMap[i].tectonicPlate];

            color.r=std::get<0>(plateColor);
            color.g=std::get<1>(plateColor);
            color.b=std::get<2>(plateColor);
            color.a=255;
        }
        else
        {
            color.r=0;
            color.g=0;
            color.b=0;
            color.a=255;
        }

        if(m_overlay==1)
        {
            if(influenceMap[i].collision<0.0f)
                color.r=color.r+(255.0f*-influenceMap[i].collision);
            else
                color.g=color.g+(255.0f*influenceMap[i].collision);
        }
        else if(m_overlay==2)
        {
            if(influenceMap[i].collision<0.0f)
                color.r=color.r+(255.0f*-influenceMap[i].collision*influenceMap[i].plateDistanceValue);
            else
                color.g=color.g+(255.0f*influenceMap[i].collision*influenceMap[i].plateDistanceValue);
        }
        else if(m_overlay==3)
        {
            if(influenceMap[i].terrainScale<0.0f)
                color.r=color.r+(255.0f*-influenceMap[i].terrainScale);
            else
                color.g=color.g+(255.0f*influenceMap[i].terrainScale);
        }
        else if(m_overlay==4)
        {
            //color scale intended for -50 to 60, temp runs -90 to 60
            float value=(influenceMap[i].temperature+50.0f)/110.0f;

            value=std::max(value, 0.0f);
            value=std::min(value, 1.0f);

            color=color+(m_tempColorMap.color((size_t)64*value));
        }
        else if(m_overlay==5)
        {
            float moisture=std::min(influenceMap[i].moisture, 1.0f);
            color=color+(m_moistureColorMap.color((size_t)63*moisture));
        }
        else if(m_overlay==6)
        {
            unsigned char value=255*influenceMap[i].moisture;

            if(influenceMap[i].heightBase > 0.5f)
                color=color+glm::ivec4(value, value, value, 255);
        }
        else if(m_overlay==7)
        {
            auto &plateColor=m_plateColors[influenceMap[i].weatherCell];

            color.r=std::get<0>(plateColor);
            color.g=std::get<1>(plateColor);
            color.b=std::get<2>(plateColor);
            color.a=255;
        }
        else if(m_overlay==8)
        {
            auto &plateColor=m_plateColors[influenceMap[i].weatherBand];

            color.r=std::get<0>(plateColor);
            color.g=std::get<1>(plateColor);
            color.b=std::get<2>(plateColor);
            color.a=255;
        }

        
        color.r=std::min(color.r, 255);
        color.g=std::min(color.g, 255);
        color.b=std::min(color.b, 255);

        textureBuffer[index++]=(GLubyte)color.r;
        textureBuffer[index++]=(GLubyte)color.g;
        textureBuffer[index++]=(GLubyte)color.b;
        textureBuffer[index++]=255;
    }

    if(m_overlayVector>0)
    {
        imglib::SimpleImage textureImage(imglib::Format::RGBA, imglib::Depth::Bit8, influenceMapSize.x, influenceMapSize.y, &textureBuffer[0], influenceMap.size());
        glm::tvec4<unsigned char> colorRed(255, 0, 0, 255);
        glm::tvec4<unsigned char> colorWhite(255, 255, 255, 255);

//        colorRed.r=255;
//        colorRed.g=0;
//        colorRed.b=0;
//        colorRed.a=255;

        glm::vec2 point;
        index=0;

        size_t skip=10;
        float fskip=(float)skip;

        point.y=0.0f;
        for(size_t y=0; y<influenceMapSize.y; y+=skip)
        {
            index=y*influenceMapSize.x;
            point.x=0.0f;
            for(size_t x=0; x<influenceMapSize.x; x+=skip)
            {
                glm::vec2 endPoint;
                glm::vec2 direction;

                if(m_overlayVector==1)
                    direction=influenceMap[index].direction;
                else
                    direction=influenceMap[index].airDirection;
                
                direction.y=-direction.y;//for images y origin is top left
                endPoint=(direction*8.0f)+point;

                endPoint.x=std::max(endPoint.x, 0.0f);
                endPoint.x=std::min(endPoint.x, (float)influenceMapSize.x);
                endPoint.y=std::max(endPoint.y, 0.0f);
                endPoint.y=std::min(endPoint.y, (float)influenceMapSize.y);

                drawLine(point, endPoint, colorRed, textureImage);

//                auto &plateColor=m_plateColors[influenceMap[index].tectonicPlate];
//
//                colorWhite.r=std::get<0>(plateColor);
//                colorWhite.g=std::get<1>(plateColor);
//                colorWhite.b=std::get<2>(plateColor);
//                colorWhite.a=255;

                drawPoint(point, colorWhite, textureImage);
                index+=skip;
                point.x+=fskip;
            }
            point.y+=fskip;
        }
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

//void MapGen::updateContinentTexture(std::vector<GLubyte> &textureBuffer)
//{
//    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
//    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
//
//    int plateCount=m_worldGenerator->getPlateCount();
//
//    RandomColor::RandomColorGenerator colorGenerator;
//
//    if(m_plateColors.size()<plateCount)
//        m_plateColors=colorGenerator.randomColors(plateCount);
//
//    size_t index=0;
//
//    for(size_t i=0; i<influenceMap.size(); ++i)
//    {
//        int elevation=32+32*influenceMap[i].continentValue;
//        glm::ivec4 color=m_biomeColorMap.color(elevation, 32);
//
//        textureBuffer[index++]=(GLubyte)color.r;
//        textureBuffer[index++]=(GLubyte)color.g;
//        textureBuffer[index++]=(GLubyte)color.b;
//        textureBuffer[index++]=255;
//    }
//
//    m_textureWidth=influenceMapSize.x;
//    m_textureHeight=influenceMapSize.y;
//
//    glBindTexture(GL_TEXTURE_2D, m_textureId);
//
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureBuffer[0]);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//
//    m_textureValid=true;
//}

void MapGen::updateHeightMapTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
    imglib::SimpleImage textureImage(imglib::Format::RGBA, imglib::Depth::Bit8, influenceMapSize.x, influenceMapSize.y, &textureBuffer[0], influenceMap.size());

    size_t index=0;

    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        const float &height=influenceMap[i].heightBase;
        
        glm::ivec4 color=m_biomeColorMap.color((size_t)64*height, 32);

        textureBuffer[index++]=(GLubyte)color.r;
        textureBuffer[index++]=(GLubyte)color.g;
        textureBuffer[index++]=(GLubyte)color.b;
//		textureBuffer[index++]=(GLubyte)255.0f*height;
//		textureBuffer[index++]=(GLubyte)255.0f*height;
//		textureBuffer[index++]=(GLubyte)255.0f*height;
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


//void MapGen::updateGeometryTexture(std::vector<GLubyte> &textureBuffer)
//{
//    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
//    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
//	imglib::SimpleImage textureImage(imglib::Format::RGBA, imglib::Depth::Bit8, influenceMapSize.x, influenceMapSize.y, &textureBuffer[0], influenceMap.size());
//
//    int plateCount=m_worldGenerator->getPlateCount();
//
//    RandomColor::RandomColorGenerator colorGenerator;
//
//    if(m_plateColors.size()<plateCount)
//        m_plateColors=colorGenerator.randomColors(plateCount);
//
////    size_t index=0;
////
////    for(size_t i=0; i<influenceMap.size(); ++i)
////    {
////        const bool &isPoint=influenceMap[i].point;
////
////        if(isPoint)
////        {
////            textureBuffer[index++]=(GLubyte)255;
////            textureBuffer[index++]=(GLubyte)255;
////            textureBuffer[index++]=(GLubyte)255;
////        }
////        else
////        {
////            textureBuffer[index++]=(GLubyte)0;
////            textureBuffer[index++]=(GLubyte)0;
////            textureBuffer[index++]=(GLubyte)0;
////        }
////        textureBuffer[index++]=255;
//
////    }
//    std::vector<glm::vec2> &points=m_worldGenerator->m_influencePoints;
//	std::vector<std::vector<glm::vec2>> &lines=m_worldGenerator->m_influenceLines;
//	
////	int32_t color=0xff0000ff;
//    glm::tvec4<unsigned char> color;
//
//    color.r=255;
//    color.g=0;
//    color.b=0;
//    color.a=255;
//
//	for(std::vector<glm::vec2> &line:lines)
//	{
//		for(size_t i=1; i<line.size(); ++i)
//		{
//			imglib::drawLine(line[i-1], line[i], color, textureImage);
//		}
//	}
//
//    color.r=255;
//    color.g=255;
//    color.b=255;
//    color.a=255;
//
//    for(auto &point:points)
//        imglib::drawPoint(point, color, textureImage);
//
//    m_textureWidth=influenceMapSize.x;
//    m_textureHeight=influenceMapSize.y;
//
//    glBindTexture(GL_TEXTURE_2D, m_textureId);
//
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureBuffer[0]);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//
//    m_textureValid=true;
//}


void MapGen::generate()
{
    m_worldGenerator->m_plateSeed=m_noiseSeed;
    m_worldGenerator->m_plateCount=m_plateCount;
    m_worldGenerator->generateWorldOverview(g_loadProgress);

//    updateTexture();
}