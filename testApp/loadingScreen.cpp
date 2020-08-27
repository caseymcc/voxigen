#include "loadingScreen.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

LoadingScreen::LoadingScreen(RenderingOptions *renderingOptions):
    m_renderingOptions(renderingOptions)
{

}

void LoadingScreen::initialize()
{
    // Setup style
    
}

void LoadingScreen::terminate()
{

}

void LoadingScreen::setSize(int width, int height)
{
    m_width=width;
    m_height=height;
}

void LoadingScreen::start()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void LoadingScreen::update(std::string status, int progress)
{
    bool show=true;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(600, 450));
    ImGui::Begin("Info", &show, 
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);

    
    ImGui::Text("Status");
    ImGui::Text(status.c_str());
    ImGui::Text("Progress %d", progress);

    ImGui::End();
}

void LoadingScreen::build()
{
    ImGui::Render();
}

void LoadingScreen::draw()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}