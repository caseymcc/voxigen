#include "voxigen/block.h"
#include "voxigen/world.h"
#include "voxigen/simpleRenderer.h"

#include <GLFW/glfw3.h>
#include <GL/glew.h>

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

    glewInit();
    glViewport(0, 0, width, height);

    voxigen::World<voxigen::Block> world("TestWorld");

    world.load();
    
    glm::ivec3 worldMiddle=world.getDescriptors().size/2;

    voxigen::SimpleCamera player;

    player.position=glm::vec3(worldMiddle);
    player.forward=glm::vec3(1.0f, 0.0f, 0.0f);
    player.up=glm::vec3(0.0f, 0.0f, 1.0f);

    voxigen::SimpleRenderer<voxigen::Block> renderer(&world);

    renderer.build();
    
//    renderer.updateProjection(width, height);
//    renderer.setCamera(player);
//
//    renderer.updateChunks();
    while(!glfwWindowShouldClose(window))
    {
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