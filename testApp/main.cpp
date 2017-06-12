#include "voxigen/block.h"
#include "voxigen/world.h"
#include "voxigen/simpleRenderer.h"

#include <GLFW/glfw3.h>

int main(int argc, char ** argv)
{
    GLFWwindow* window;

    /* Initialize the library */
    if(!glfwInit())
        return -1;

    window=glfwCreateWindow(640, 480, "TestApp", NULL, NULL);
    
    if(!window)
    {
        glfwTerminate();
        return -1;

    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    voxigen::World<voxigen::Block> world("TestWorld");
    voxigen::SimpleRenderer<voxigen::Block> renderer(&world);

    world.load();


    while(!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        renderer.draw();
    }

    glfwTerminate();
    return 0;
}