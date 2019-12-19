#include <gflags/gflags.h>
#include <glog/logging.h>

#include <glbinding/gl/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "./testApp/initGlew.h"
#include "mapgen.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
{
//    if(severity != DEBUG_SEVERITY_NOTIFICATION)
    if(type == GL_DEBUG_TYPE_ERROR)
        LOG(INFO)<<"Opengl - error: "<<message;
//    else
//        LOG(INFO)<<"Opengl : "<<message;
}


MapGen mapgen;

int main(int argc, char ** argv)
{
//    FLAGS_log_dir="E:/projects/lumberyard_git/dev/Code/SDKs/voxigen/log";
//    FLAGS_alsologtostderr=true;
//
//    google::InitGoogleLogging(argv[0]);

    GLFWwindow *window;

    /* Initialize the library */
    if(!glfwInit())
        return -1;

    size_t width=1600;
    size_t height=1200;

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, (int)GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window=glfwCreateWindow(width, height, "MapGenApp", NULL, NULL);

    if(!window)
    {
        glfwTerminate();
        return -1;

    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    glbinding::initialize(glfwGetProcAddress);
    initGlew();//hack for imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io=ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    mapgen.initialize();

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//    glfwSetCursorPosCallback(window, mouse_callback);
//    glfwSetMouseButtonCallback(window,
//        [](GLFWwindow *, int button, int action, int modifiers) 
//        {
//        });
//    glfwSetCharCallback(window,
//        [](GLFWwindow *, unsigned int codepoint) 
//        {
//        });

#ifndef NDEBUG
    glDebugMessageCallback(debugMessage, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
#endif

    framebuffer_size_callback(window, width, height);

    while(!glfwWindowShouldClose(window))
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        mapgen.draw();
    
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //get opengl started
        glFlush();

        // Poll for and process events
        glfwPollEvents();

        // Swap front and back buffers when opengl ready
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    mapgen.setSize(width, height);
}

//void mouse_callback(GLFWwindow* window, double xpos, double ypos)
//{
//
//}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
