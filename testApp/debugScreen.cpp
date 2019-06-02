#include "debugScreen.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

DebugScreen::DebugScreen(RenderingOptions *renderingOptions):
    m_renderingOptions(renderingOptions)
{

}

void DebugScreen::initialize()
{
    // Setup style
    
}

void DebugScreen::terminate()
{

}

void DebugScreen::startBuild()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugScreen::setSize(int width, int height)
{
    m_width=width;
    m_height=height;
}

void DebugScreen::update(World *world)
{
    bool show=true;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(350, 450));
    ImGui::Begin("Info", &show, 
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);

    glm::ivec3 regionIndex=world->getRegionIndex(m_renderingOptions->camera.getRegionHash());
    const glm::vec3 &cameraPos=m_renderingOptions->camera.getPosition();
    glm::ivec3 chunkIndex=world->getChunkIndex(cameraPos);

    const glm::vec3 &playerPos=m_renderingOptions->player.getPosition();
    
    ImGui::Text("Camera Pos: Region: %d, %d, %d  Chunk: %d, %d, %d (%0.2f, %0.2f, %0.2f)", 
        regionIndex.x, regionIndex.y, regionIndex.z, chunkIndex.x, chunkIndex.y, chunkIndex.z, cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::Text("Player Pos: Region: %d, %d, %d  Chunk: %d, %d, %d (%0.2f, %0.2f, %0.2f)", 
        m_renderingOptions->playerRegionIndex.x, m_renderingOptions->playerRegionIndex.y, m_renderingOptions->playerRegionIndex.z, 
        m_renderingOptions->playerChunkIndex.x, m_renderingOptions->playerChunkIndex.y, m_renderingOptions->playerChunkIndex.z, 
        playerPos.x, playerPos.y, playerPos.z);
    ImGui::Text("Press \"`\" toggle mouse lock");
    ImGui::Text("Press \"q\" toggle camera/player movement");
    ImGui::Text("Press \"r\" reset camera to player");
    ImGui::Text("Press \"o\" toggle chunk overlay");
    ImGui::Text("Press \"i\" toggle chunk info");

    ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f/ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void DebugScreen::updateControls()
{
    bool show=true;

    ImGui::Begin("Controls", &show, ImVec2(150, 400), 0.5f);

    ImGui::Checkbox("Move player", &m_renderingOptions->move_player);
    if(ImGui::Button("Reset to player"))
        m_renderingOptions->resetCamera=true;
    
    ImGui::Checkbox("Regions", &m_renderingOptions->show_regions);
    ImGui::Checkbox("Chunks", &m_renderingOptions->show_chunks);

    ImGui::End();
}

void DebugScreen::updateChunkInfo(World *world, WorldRenderer *renderer)
{
    bool show=true;

    auto chunkRenderers=renderer->getChunkRenderers();
    std::string info;

    ImGui::SetNextWindowPos(ImVec2(m_width-350, 0));
    ImGui::SetNextWindowSize(ImVec2(350, 450));
    ImGui::Begin("Region/Chunk Info", &show, 
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
    
    ChunkRenderer *cameraChunk=nullptr;
    ChunkRenderer *playerChunk=nullptr;

    const glm::vec3 &cameraPos=m_renderingOptions->camera.getPosition();
    glm::ivec3 cameraChunkIndex=world->getChunkIndex(cameraPos);

    for(ChunkRenderer *chunkRenderer:chunkRenderers)
    {
        if(chunkRenderer->getChunkIndex() == cameraChunkIndex)
            cameraChunk=chunkRenderer;
        if(chunkRenderer->getChunkIndex() == m_renderingOptions->playerChunkIndex)
            playerChunk=chunkRenderer;
    }

    if(cameraChunk)
    {
        cameraChunk->updateInfo(info);
        ImGui::Text("Camera");
        ImGui::Text(info.c_str());
        ImGui::Text("");
    }
    else
    {
        ImGui::Text("Camera");
        ImGui::Text("Region Index:\nChunk Index:\nRenderer:\nChunk:");
        ImGui::Text("");
    }
    if(playerChunk)
    {
        playerChunk->updateInfo(info);
        ImGui::Text("Player");
        ImGui::Text(info.c_str());
    }
    else
    {
        ImGui::Text("Player");
        ImGui::Text("Region Index:\nChunk Index:\nRenderer:\nChunk:");
    }

    ImGui::End();
}

void DebugScreen::build()
{
    ImGui::Render();
}

void DebugScreen::draw()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}