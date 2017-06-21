
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "voxigen/block.h"
#include "voxigen/world.h"
#include "voxigen/simpleRenderer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool key_w=false;
bool key_a=false;
bool key_d=false;
bool key_s=false;
bool key_space=false;
bool key_left_shift=false;
voxigen::SimpleFpsCamera player;

int main(int argc, char ** argv)
{
    GLFWwindow* window;

    /* Initialize the library */
    if(!glfwInit())
        return -1;

    size_t width=640;
    size_t height=480;

    window=glfwCreateWindow(width, height, "TestApp", NULL, NULL);
    
    if(!window)
    {
        glfwTerminate();
        return -1;

    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    glewInit();
    glViewport(0, 0, width, height);

    voxigen::World<voxigen::Block> world("TestWorld");

    world.load();
    
    glm::ivec3 worldMiddle=(world.getDescriptors().size)/2;

    worldMiddle.z+=5.0f;
//    player.position=glm::vec3(worldMiddle);
////    player.position=glm::vec3(0.0f, -20.0f, 1.0f);
//    player.direction=glm::vec3(0.0f, 1.0f, 0.0f);
//    player.up=glm::vec3(0.0f, 0.0f, 1.0f);

//    player.position=glm::vec3(0.0f, 0.0f, 20.0f);
//    player.direction=glm::vec3(0.0f, 0.0f, -1.0f);
//    player.up=glm::vec3(0.0f, 1.0f, 0.0f);
    player.setPosition(worldMiddle);
    player.setYaw(0.0f);
    player.setPitch(0.0f);

    voxigen::SimpleRenderer<voxigen::Block> renderer(&world);

    renderer.setCamera(&player);
    renderer.build();
//    renderer.updateProjection(width, height);
    renderer.setViewRadius(64.0f);
    renderer.updateChunks();

    float turnSpeed=0.5;
    float movementSpeed=2;

    float lastFrame=glfwGetTime();
    float currentFrame;
    float deltaTime;

    while(!glfwWindowShouldClose(window))
    {
        currentFrame=glfwGetTime();
        deltaTime=currentFrame-lastFrame;
        lastFrame=currentFrame;

        glm::vec3 direction(0.0f, 0.0f, 0.0f);
        bool move=false;

        if(key_w)
        {
            direction+=glm::vec3(1.0f, 0.0f, 0.0f);
            move=true;
        }
        if(key_s)
        {
            direction+=glm::vec3(-1.0f, 0.0f, 0.0f);
            move=true;
        }
        if(key_a)
        {
            direction+=glm::vec3(0.0f, -1.0f, 0.0f);
            move=true;
        }
        if(key_d)
        {
            direction+=glm::vec3(0.0f, 1.0f, 0.0f);
            move=true;
        }
        if(key_space)
        {
            direction+=glm::vec3(0.0f, 0.0f, 1.0f);
            move=true;
        }
        if(key_left_shift)
        {
            direction+=glm::vec3(0.0f, 0.0f, -1.0f);
            move=true;
        }

        if(move)
            player.move(direction*movementSpeed*deltaTime);
        /* Render here */
//        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
//        glDepthFunc(GL_LESS);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//        renderer.setCamera(player);
        renderer.draw();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    player.setView(width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    else if(key==GLFW_KEY_W)
    {
        if(action==GLFW_PRESS)
            key_w=true;
        else if(action==GLFW_RELEASE)
            key_w=false;
    }
    else if(key==GLFW_KEY_S)
    {
        if(action==GLFW_PRESS)
            key_s=true;
        else if(action==GLFW_RELEASE)
            key_s=false;
    }
    else if(key==GLFW_KEY_A)
    {
        if(action==GLFW_PRESS)
            key_a=true;
        else if(action==GLFW_RELEASE)
            key_a=false;
    }
    else if(key==GLFW_KEY_D)
    {
        if(action==GLFW_PRESS)
            key_d=true;
        else if(action==GLFW_RELEASE)
            key_d=false;
    }
    else if(key==GLFW_KEY_SPACE)
    {
        if(action==GLFW_PRESS)
            key_space=true;
        else if(action==GLFW_RELEASE)
            key_space=false;
    }
    else if(key==GLFW_KEY_LEFT_SHIFT)
    {
        if(action==GLFW_PRESS)
            key_left_shift=true;
        else if(action==GLFW_RELEASE)
            key_left_shift=false;
    }
}