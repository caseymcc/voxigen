
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/filesystem.hpp>

#include "voxigen/defines.h"
#include "voxigen/cell.h"
#include "voxigen/regularGrid.h"
#include "voxigen/simpleRenderer.h"
#include "voxigen/equiRectWorldGenerator.h"

#include "voxigen/texturePack.h"
#include "voxigen/textureAtlas.h"
#include "voxigen/simpleShapes.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <filesystem>

namespace fs=boost::filesystem;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool key_w=false;
bool key_a=false;
bool key_d=false;
bool key_s=false;
bool key_space=false;
bool key_left_shift=false;
bool key_caps_on=false;
bool resetCamera=false;
bool showPlayer=false;

voxigen::SimpleFpsCamera camera;
voxigen::Position player;

unsigned int playerRegion;
glm::ivec3 playerRegionIndex;
glm::ivec3 playerChunkIndex;
unsigned int playerChunk;
bool lastUpdateAdded;

typedef voxigen::RegularGrid<voxigen::Cell, 64, 64, 16> World;
namespace voxigen
{
//force generator instantiation
template GeneratorTemplate<EquiRectWorldGenerator<World>>;
}
voxigen::SimpleRenderer<World> *g_renderer;

void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
{
//    if(severity != DEBUG_SEVERITY_NOTIFICATION)
    if(type == GL_DEBUG_TYPE_ERROR)
        LOG(INFO)<<"Opengl - error: "<<message;
//    else
//        LOG(INFO)<<"Opengl : "<<message;
}

std::string vertMarkerShader=
"#version 330 core\n"
"layout (location = 0) in vec3 inputVertex;\n"
"layout (location = 1) in vec3 inputNormal;\n"
"layout (location = 2) in vec2 inputTexCoord;\n"
"layout (location = 3) in vec4 color;\n"
"\n"
"out vec3 position;\n"
"out vec3 normal;\n"
"out vec4 vertexColor;\n"
"\n"
"uniform mat4 projectionView;\n"
"uniform vec3 regionOffset;\n"
"\n"
"void main()\n"
"{\n"
"   normal=inputNormal;\n"
"   vertexColor=color;\n"
"   vec3 position=regionOffset+inputVertex;\n"
"   gl_Position=projectionView*vec4(position, 1.0);\n"
"}\n"
"";

std::string fragMarkerShader=
"#version 330 core\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec4 vertexColor;\n"
"\n"
"out vec4 color;\n"
"\n"
"//uniform vec3 lightPos;\n"
"//uniform vec3 lightColor;\n"
"\n"
"void main()\n"
"{\n"
"   //vec3 lightDir=normalize(lightPos-position); \n"
"   //float diff=max(dot(normal, lightDir), 0.0); \n"
"   //vec3 diffuse=diff * lightColor; \n"
"   //color=vec4((ambient+diffuse)*vertexColor.xyz, 1.0f); \n"
"   color=vertexColor;\n"
"}\n"
"";

int main(int argc, char ** argv)
{
    FLAGS_log_dir="E:/projects/lumberyard_git/dev/Code/SDKs/voxigen/log";
//    FLAGS_logtostderr=true;
    FLAGS_alsologtostderr=true;

    google::InitGoogleLogging(argv[0]);

    GLFWwindow* window;

    /* Initialize the library */
    if(!glfwInit())
        return -1;

    size_t width=1920;
    size_t height=1080;

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    window=glfwCreateWindow(width, height, "TestApp", NULL, NULL);
    
    if(!window)
    {
        glfwTerminate();
        return -1;

    }

    voxigen::TexturePack texturePack;

    fs::path texturePackPath("resources/TexturePacks/SoA_Default");

    texturePack.load(texturePackPath.string());

    voxigen::SharedTextureAtlas textureAtlas(new voxigen::TextureAtlas());
    std::vector<std::string> blockNames={"dirt", "grass", "mud", "mud_dry", "sand", "clay", "cobblestone", "stone", "granite", "slate", "snow"};

    textureAtlas->build(blockNames, texturePack);
    textureAtlas->save(texturePackPath.string(), "terrain");

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    glewInit();

#ifndef NDEBUG
    glDebugMessageCallback(debugMessage, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
#endif
//create player marker
    unsigned int m_playerVertexArray;
    unsigned int playerVertexBufferID;
    unsigned int playerColor;
    const std::vector<float> &playerVertices=voxigen::SimpleCube<1, 1, 2>::vertCoords;

    glGenVertexArrays(1, &m_playerVertexArray);
    glGenBuffers(1, &playerVertexBufferID);
    glGenBuffers(1, &playerColor);

    glBindVertexArray(m_playerVertexArray);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, playerVertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, playerColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, playerVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*playerVertices.size(), playerVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, playerColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*1, glm::value_ptr(color), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    opengl_util::Program markerProgram;
    std::string error;
    if(!markerProgram.attachLoadAndCompileShaders(vertMarkerShader, fragMarkerShader, error))
    {
        assert(false);
        return 0;
    }

    size_t markerProjectionViewId=markerProgram.getUniformId("projectionView");
    size_t markerOffsetId=markerProgram.getUniformId("regionOffset");
//end player marker

    glViewport(0, 0, width, height);

    World world;

    fs::path worldsDirectory("worlds");

    if(!fs::exists(worldsDirectory))
        fs::create_directory(worldsDirectory);

    std::vector<fs::directory_entry> worldDirectories;

    for(auto &entry:fs::directory_iterator(worldsDirectory))
        worldDirectories.push_back(entry);

    if(worldDirectories.empty())
    {
        std::string worldDirectory=worldsDirectory.string()+"/TestApWorld";
        fs::path worldPath(worldDirectory);
        
        fs::create_directory(worldPath);
        world.create(worldDirectory, "TestApWorld", glm::ivec3(32768, 32768, 256), "EquiRectWorldGenerator");
    }
    else
        world.load(worldDirectories[0].path().string());
    
    glm::ivec3 regionCellSize=world.regionCellSize();
    glm::ivec3 worldMiddle=(world.getDescriptors().m_size)/2;

//    worldMiddle.z+=worldMiddle.z/2;
    
    camera.setYaw(0.0f);
    camera.setPitch(0.0f);
    
    voxigen::Key hashes=world.getHashes(worldMiddle);

    playerRegion=hashes.regionHash;
    playerChunk=hashes.chunkHash;
    playerRegionIndex=world.getRegionIndex(playerRegion);
    playerChunkIndex=world.getChunkIndex(playerChunk);
    
    //set player position to local region
    glm::vec3 regionPos=world.gridPosToRegionPos(playerRegion, worldMiddle)+glm::vec3(32.0f, 32.0f, 0.0f);
    
    camera.setPosition(playerRegion, regionPos);
    player.setPosition(playerRegionIndex, regionPos);

    world.updatePosition(playerRegionIndex, playerChunkIndex);

    voxigen::SimpleRenderer<World> renderer(&world);

    g_renderer=&renderer;
    renderer.setTextureAtlas(textureAtlas);

    renderer.setCamera(&camera);
    renderer.build();
    renderer.setViewRadius(glm::ivec3(1024, 1024, 256));

    renderer.setCameraChunk(playerRegionIndex, playerChunkIndex);
    renderer.setPlayerChunk(playerRegionIndex, playerChunkIndex);
//    lastUpdateAdded=renderer.updateChunks();
//    renderer.updateChunks();

    float movementSpeed=100;

    float lastFrame=glfwGetTime();
    float currentFrame;
    float deltaTime;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(!glfwWindowShouldClose(window))
    {
        if(resetCamera)
        {
            glm::ivec3 regionIndex=player.getRegionIndex();
            glm::vec3 position=player.getPosition();
            glm::ivec3 chunkIndex=world.getChunkIndex(position);
            voxigen::RegionHash regionHash=world.getRegionHash(regionIndex);

            camera.setPosition(regionHash, position);
            renderer.setCameraChunk(regionIndex, chunkIndex);

            resetCamera=false;
            showPlayer=false;
        }

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

//        bool updateChunks=false;

        if(move)
        {
            unsigned int chunkHash;
            bool updateCamera;
            glm::vec3 deltaPosition;

            camera.moveDirection(direction*movementSpeed*deltaTime, deltaPosition);
            glm::ivec3 cameraRegionIndex=world.getRegionIndex(camera.getRegionHash());
            glm::vec3 cameraPosition=camera.getPosition();

            if(world.alignPosition(cameraRegionIndex, cameraPosition))
            {
                voxigen::RegionHash cameraRegionHash=world.getRegionHash(cameraRegionIndex);

                camera.setPosition(cameraRegionHash, cameraPosition);
                g_renderer->setCameraChunk(cameraRegionIndex, world.getChunkIndex(cameraPosition));
            }

            if(!key_caps_on)
            {
                player.move(deltaPosition);

                glm::ivec3 regionIndex=player.getRegionIndex();
                glm::vec3 position=player.getPosition();

                if(world.alignPosition(regionIndex, position))
                    player.setPosition(regionIndex, position);

                glm::ivec3 chunkIndex=world.getChunkIndex(position);

                if((playerChunkIndex!=chunkIndex) || (playerRegionIndex!=regionIndex))
                {
                    playerRegionIndex=regionIndex;
                    playerChunkIndex=chunkIndex;

                    g_renderer->setPlayerChunk(playerRegionIndex, playerChunkIndex);
                    world.updatePosition(playerRegionIndex, playerChunkIndex);
                }

                if((cameraRegionIndex!=regionIndex)||(cameraPosition!=position))
                    showPlayer=true;
                else
                    showPlayer=false;
            }
            else
            {
                glm::ivec3 regionIndex=player.getRegionIndex();
                glm::vec3 position=player.getPosition();

                if((cameraRegionIndex!=regionIndex)||(cameraPosition!=position))
                    showPlayer=true;
                else
                    showPlayer=false;
            }
        }

        //Rendering started
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        //render anything that needs it
        renderer.draw();

        if(showPlayer)
        {
            markerProgram.use();
            
            markerProgram.uniform(markerProjectionViewId)=camera.getProjectionViewMat();
            
            glm::ivec3 cameraRegionIndex=world.getRegionIndex(camera.getRegionHash());
            glm::vec3 cameraPosition=camera.getPosition();
            glm::ivec3 regionIndex=player.getRegionIndex();
            glm::vec3 position=player.getPosition();

            glm::vec3 regionOffset=regionIndex-cameraRegionIndex;
            glm::vec3 offset=regionOffset*glm::vec3(world.getDescriptors().m_regionCellSize)+position;

            markerProgram.uniform(markerOffsetId)=offset;

            glBindVertexArray(m_playerVertexArray);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1);
        }

        //get opengl started, we can handle some updating before calling the swap
        glFlush();

        //take time to make any scene updates
        renderer.update();

        // Swap front and back buffers when opengl ready
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();

    }

    //bring renderers down before world is terminated;
    renderer.destroy();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    camera.setView(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static bool firstMouse=true;
    static float lastX;
    static float lastY;
    static float yaw=0.0f;
    static float pitch=0.0f;

    if(firstMouse)
    {
        lastX=xpos;
        lastY=ypos;
        firstMouse=false;
    }

    float xoffset=lastX-xpos;
    float yoffset=lastY-ypos;

    lastX=xpos;
    lastY=ypos;

    float sensitivity=0.1;

    xoffset*=sensitivity;
    yoffset*=sensitivity;

    yaw+=xoffset;
    pitch+=yoffset;

    if(yaw > 360.0f)
        yaw-=360.0f;
    if(yaw < 0)
        yaw+=360.0f;

    if(pitch > 89.0f)
        pitch=89.0f;
    if(pitch < -89.0f)
        pitch=-89.0f;

    camera.setYaw(glm::radians(yaw));
    camera.setPitch(glm::radians(pitch));
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
    else if(key==GLFW_KEY_O)
    {
        if(action==GLFW_RELEASE)
            g_renderer->displayOutline(!g_renderer->isDisplayOutline());
    }
    else if(key==GLFW_KEY_CAPS_LOCK)
    {
        if(action==GLFW_RELEASE)
            key_caps_on=!key_caps_on;
    }
    else if(key==GLFW_KEY_R)
    {
        if(action==GLFW_RELEASE)
            resetCamera=true;
    }
}