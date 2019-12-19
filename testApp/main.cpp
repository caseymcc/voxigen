#include "voxigen/defines.h"
#include "voxigen/volume/cell.h"
#include "voxigen/volume/regularGrid.h"
#include "voxigen/rendering/simpleRenderer.h"
#include "voxigen/generators/equiRectWorldGenerator.h"

#include "voxigen/texturing/texturePack.h"
#include "voxigen/texturing/textureAtlas.h"
#include "voxigen/rendering/simpleShapes.h"
#include "voxigen/fileio/filesystem.h"
#include "voxigen/fileio/log.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <glbinding/gl/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "debugScreen.h"
#include "marker.h"
#include "renderingOptions.h"
#include "initGlew.h"
#include "world.h"

//using namespace gl;
//
//#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM "<glbinding/gl/gl.h>"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
voxigen::SharedTextureAtlas buildTextureAtlas();

bool key_w=false;
bool key_a=false;
bool key_d=false;
bool key_s=false;
bool key_space=false;
bool key_left_shift=false;
bool key_caps_on=false;
//bool resetCamera=false;
//bool showPlayer=false;
//bool move_player=true;
//bool show_chunks=true;
//bool show_regions=true;
//bool cursor_capture=false;
RenderingOptions renderingOptions;

float lastFrame;
float currentFrame;
float deltaTime;

DebugScreen *debugScreen=nullptr;

//voxigen::SimpleFpsCamera camera;
//voxigen::Position player;
//typedef voxigen::RegularGrid<voxigen::Cell, 64, 64, 16> World;
//namespace voxigen
//{
////force generator instantiation
//template class GeneratorTemplate<EquiRectWorldGenerator<World>>;
//}
//voxigen::SimpleRenderer<World> *g_renderer;
WorldRenderer *g_renderer;

//Player info
Marker marker;
//unsigned int playerRegion;
//glm::ivec3 playerRegionIndex;
//glm::ivec3 playerChunkIndex;
//unsigned int playerChunk;
bool lastUpdateAdded;

void updatePosition(World &world);
void updateChunkInfo();

void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
{
//    if(severity != DEBUG_SEVERITY_NOTIFICATION)
    if(type == GL_DEBUG_TYPE_ERROR)
        LOG(INFO)<<"Opengl - error: "<<message;
//    else
//        LOG(INFO)<<"Opengl : "<<message;
}

class LogCallback:public generic::ILogCallback
{
public:
    LogCallback() {};
    virtual ~LogCallback() {};

    virtual bool write(generic::LogLevel level, const std::string &entry)
    {
        switch(level)
        {
        case generic::LogLevel::Message:
            LOG(INFO)<<entry;
            break;
        case generic::LogLevel::Warning:
            LOG(WARNING)<<entry;
            break;
        case generic::LogLevel::Error:
            LOG(ERROR)<<entry;
            break;
        case generic::LogLevel::Debug:
            LOG(INFO)<<entry;
            break;
        }
        return true;
    }
};

int main(int argc, char ** argv)
{
    FLAGS_log_dir="E:/projects/lumberyard_git/dev/Code/SDKs/voxigen/log";
//    FLAGS_logtostderr=true;
    FLAGS_alsologtostderr=true;

    //setup logging
    google::InitGoogleLogging(argv[0]);
    std::shared_ptr<LogCallback> logCallback=std::make_shared<LogCallback>();
    voxigen::Log::attachCallback(logCallback);

    GLFWwindow* window;

    /* Initialize the library */
    if(!glfwInit())
        return -1;

    size_t width=1600;
    size_t height=1080;

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, (int)GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window=glfwCreateWindow(width, height, "TestApp", NULL, NULL);

    if(!window)
    {
        glfwTerminate();
        return -1;

    }
    voxigen::SharedTextureAtlas textureAtlas=buildTextureAtlas();
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

//    voxigen::initOpenGL(glfwGetProcAddress);
    glbinding::initialize(glfwGetProcAddress);
    initGlew();//hack for imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io=ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __APPLE__    
    ImGui_ImplOpenGL3_Init("#version 410");
#else
    ImGui_ImplOpenGL3_Init();
#endif

    ImGui::StyleColorsDark();

    debugScreen=new DebugScreen(&renderingOptions);
    
//    debugScreen->initialize(window, true);
//    debugScreen->performLayout();
    debugScreen->initialize();
    debugScreen->setSize(width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window,
        [](GLFWwindow *, int button, int action, int modifiers) 
        {
//            debugScreen->mouseButtonCallbackEvent(button, action, modifiers);
        });
    glfwSetCharCallback(window,
        [](GLFWwindow *, unsigned int codepoint) 
        {
            if(!renderingOptions.cursor_capture){}
//                debugScreen->charCallbackEvent(codepoint);
        });

//    debugScreen->setVisible(true);

#ifndef NDEBUG
    GLint majorVersion, minorVersion;

    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

    if((majorVersion >= 4) && (minorVersion >= 3))
    {
        glDebugMessageCallback(debugMessage, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
    }
#endif


//create player marker
    marker.init();

    framebuffer_size_callback(window, width, height);

    World world;

    fs::path worldsDirectory("worlds");

    if(!fs::exists(worldsDirectory))
        fs::create_directory(worldsDirectory);

    std::vector<fs::directory_entry> worldDirectories;

    for(auto &entry:fs::directory_iterator(worldsDirectory))
        worldDirectories.push_back(entry);

    if(worldDirectories.empty())
    {
        std::string worldName="TestAppWorld";
        std::string worldDirectory=worldsDirectory.string()+"/"+worldName;
        fs::path worldPath(worldDirectory);
        
        fs::create_directory(worldPath);

        //creating earth sizes
//        world.create(worldDirectory, "TestApWorld", glm::ivec3(20971520, 10485760, 2560), "EquiRectWorldGenerator");
        world.create(worldDirectory, worldName, glm::ivec3(204800, 102400, 10240), "EquiRectWorldGenerator");
    }
    else
        world.load(worldDirectories[0].path().string());
    
    glm::ivec3 regionCellSize=world.regionCellSize();
    glm::ivec3 worldMiddle=(world.getDescriptors().m_size)/2;

    worldMiddle.z+=worldMiddle.z/2;
//    worldMiddle=glm::ivec3(1536, 1536, 384);

    renderingOptions.camera.setYaw(0.0f);
    renderingOptions.camera.setPitch(0.0f);
    
    voxigen::Key hashes=world.getHashes(glm::vec3(worldMiddle));

    renderingOptions.playerRegion=hashes.regionHash;
    renderingOptions.playerChunk=hashes.chunkHash;
    renderingOptions.playerRegionIndex=world.getRegionIndex(renderingOptions.playerRegion);
    renderingOptions.playerChunkIndex=world.getChunkIndex(renderingOptions.playerChunk);
    
    //set player position to local region
    glm::vec3 regionPos=world.gridPosToRegionPos(renderingOptions.playerRegion, glm::vec3(worldMiddle))+glm::vec3(32.0f, 32.0f, 0.0f);
    
    renderingOptions.camera.setPosition(renderingOptions.playerRegion, regionPos);
    renderingOptions.player.setPosition(renderingOptions.playerRegionIndex, regionPos);

    world.updatePosition(renderingOptions.playerRegionIndex, renderingOptions.playerChunkIndex);

    voxigen::SimpleRenderer<World> renderer(&world);

    g_renderer=&renderer;
    renderer.setTextureAtlas(textureAtlas);

    renderer.setCamera(&renderingOptions.camera);
    renderer.build();
    renderer.setViewRadius(glm::ivec3(128, 128, 128));
//    renderer.setViewRadius(glm::ivec3(1024, 1024, 1024));

    renderer.setCameraChunk(renderingOptions.playerRegionIndex, renderingOptions.playerChunkIndex);
    renderer.setPlayerChunk(renderingOptions.playerRegionIndex, renderingOptions.playerChunkIndex);
//    lastUpdateAdded=renderer.updateChunks();
//    renderer.updateChunks();

    lastFrame=glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    debugScreen->startBuild();
    debugScreen->update(&world);
    debugScreen->build();
    while(!glfwWindowShouldClose(window))
    {
        if(renderingOptions.resetCamera)
        {
            glm::ivec3 regionIndex=renderingOptions.player.getRegionIndex();
            glm::vec3 position=renderingOptions.player.getPosition();
            glm::ivec3 chunkIndex=world.getChunkIndex(position);
            voxigen::RegionHash regionHash=world.getRegionHash(regionIndex);

            renderingOptions.camera.setPosition(regionHash, position);
            renderer.setCameraChunk(regionIndex, chunkIndex);

            renderingOptions.resetCamera=false;
            renderingOptions.showPlayer=false;
        }

        updatePosition(world);

        //Rendering started
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        //render anything that needs it
        renderer.draw();

        if(renderingOptions.showPlayer)
        {
            glm::ivec3 cameraRegionIndex=world.getRegionIndex(renderingOptions.camera.getRegionHash());
            glm::vec3 cameraPosition=renderingOptions.camera.getPosition();
            glm::ivec3 regionIndex=renderingOptions.player.getRegionIndex();
            glm::vec3 position=renderingOptions.player.getPosition();

            glm::vec3 regionOffset=glm::vec3(regionIndex-cameraRegionIndex);
            glm::vec3 offset=regionOffset*glm::vec3(world.getDescriptors().m_regionCellSize)+position;

            marker.draw(renderingOptions.camera.getProjectionViewMat(), offset);
        }


        //render ui
//        debugScreen->drawContents();
//        debugScreen->drawWidgets();
//        glDepthFunc(GL_ALWAYS);
//        glDisable(GL_CULL_FACE);
        debugScreen->draw();
//        glEnable(GL_CULL_FACE);

        //get opengl started, we can handle some updating before calling the swap
        glFlush();

        bool regionsUpdated=false, chunksUpdated=false;
        //take time to make any scene updates
        renderer.update(regionsUpdated, chunksUpdated);
        
        debugScreen->startBuild();
        debugScreen->update(&world);
        debugScreen->updateControls();
        debugScreen->updateChunkInfo(&world, &renderer);
        debugScreen->build();

        if(chunksUpdated)
            updateChunkInfo();

        // Swap front and back buffers when opengl ready
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    //bring renderers down before world is terminated;
    renderer.destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

voxigen::SharedTextureAtlas buildTextureAtlas()
{
    fs::path texturePackPath("resources/TexturePacks/SoA_Default");

    voxigen::SharedTexturePack texturePack=voxigen::generateTexturePack(texturePackPath.string());

//    texturePack.load(texturePackPath.string());

    std::vector<std::string> blockNames={"dirt", "grass", "mud", "mud_dry", "sand", "clay", "cobblestone", "stone", "granite", "slate", "snow"};

//    voxigen::SharedTextureAtlas textureAtlas(new voxigen::TextureAtlas());
//    textureAtlas->build(blockNames, texturePack);
    voxigen::SharedTextureAtlas textureAtlas=generateTextureAtlas(blockNames, *texturePack);

    textureAtlas->save(texturePackPath.string(), "terrain");

    return textureAtlas;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
//    debugScreen->resizeCallbackEvent(width, height);

    glViewport(0, 0, width, height);
    if(debugScreen)
        debugScreen->setSize(width, height);
    renderingOptions.camera.setView(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(!renderingOptions.cursor_capture)
    {
//        debugScreen->cursorPosCallbackEvent(xpos, ypos);
        return;
    }

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

    renderingOptions.camera.setYaw(glm::radians(yaw));
    renderingOptions.camera.setPitch(glm::radians(pitch));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    else if((key==GLFW_KEY_GRAVE_ACCENT) &&(action==GLFW_RELEASE))
    {
        if(!renderingOptions.cursor_capture)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            renderingOptions.cursor_capture=true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            renderingOptions.cursor_capture=false;
        }
    }

    if(!renderingOptions.cursor_capture)
    {
//        debugScreen->keyCallbackEvent(key, scancode, action, mods);
        return;
    }

    if(key==GLFW_KEY_W)
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
    else if(key==GLFW_KEY_I)
    {
        if(action==GLFW_RELEASE)
            g_renderer->displayInfo(!g_renderer->isDisplayInfo());
    }
    else if(key==GLFW_KEY_Q)
    {
        if(action==GLFW_RELEASE)
        {
            renderingOptions.move_player=!renderingOptions.move_player;
//            debugScreen->update();
        }
    }
    else if(key==GLFW_KEY_E)
    {
        if(action==GLFW_RELEASE)
        {
            renderingOptions.show_chunks=!renderingOptions.show_chunks;
            g_renderer->showChunks(renderingOptions.show_chunks);
        }
    }
    else if(key==GLFW_KEY_C)
    {
        if(action==GLFW_RELEASE)
        {
            renderingOptions.show_regions=!renderingOptions.show_regions;
            g_renderer->showRegions(renderingOptions.show_regions);
        }
    }
    else if(key==GLFW_KEY_R)
    {
        if(action==GLFW_RELEASE)
            renderingOptions.resetCamera=true;
    }
}

void updatePosition(World &world)
{
    float movementSpeed=100;

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

        renderingOptions.camera.moveDirection(direction*movementSpeed*deltaTime, deltaPosition);
        glm::ivec3 cameraRegionIndex=world.getRegionIndex(renderingOptions.camera.getRegionHash());
        glm::vec3 cameraPosition=renderingOptions.camera.getPosition();

        if(world.alignPosition(cameraRegionIndex, cameraPosition))
        {
            voxigen::RegionHash cameraRegionHash=world.getRegionHash(cameraRegionIndex);

            renderingOptions.camera.setPosition(cameraRegionHash, cameraPosition);
            g_renderer->setCameraChunk(cameraRegionIndex, world.getChunkIndex(cameraPosition));
        }

        if(renderingOptions.move_player)
        {
            renderingOptions.player.move(deltaPosition);

            glm::ivec3 regionIndex=renderingOptions.player.getRegionIndex();
            glm::vec3 position=renderingOptions.player.getPosition();

            if(world.alignPosition(regionIndex, position))
                renderingOptions.player.setPosition(regionIndex, position);

            glm::ivec3 chunkIndex=world.getChunkIndex(position);

            if((renderingOptions.playerChunkIndex!=chunkIndex)||(renderingOptions.playerRegionIndex!=regionIndex))
            {
                renderingOptions.playerRegionIndex=regionIndex;
                renderingOptions.playerChunkIndex=chunkIndex;

                g_renderer->setPlayerChunk(renderingOptions.playerRegionIndex, renderingOptions.playerChunkIndex);
                world.updatePosition(renderingOptions.playerRegionIndex, renderingOptions.playerChunkIndex);
            }

            if((cameraRegionIndex!=regionIndex)||(cameraPosition!=position))
                renderingOptions.showPlayer=true;
            else
                renderingOptions.showPlayer=false;
        }
        else
        {
            glm::ivec3 regionIndex=renderingOptions.player.getRegionIndex();
            glm::vec3 position=renderingOptions.player.getPosition();

            if((cameraRegionIndex!=regionIndex)||(cameraPosition!=position))
                renderingOptions.showPlayer=true;
            else
                renderingOptions.showPlayer=false;
        }
    }
}

void updateChunkInfo()
{
    std::ostringstream chunkInfoStream;
    auto chunkRenderers=g_renderer->getChunkRenderers();

    for(auto chunkRenderer:chunkRenderers)
    {
        if(!chunkRenderer)
            continue;

        std::string chunkInfo;

        chunkRenderer->updateInfo(chunkInfo);
        chunkInfoStream<<chunkInfo;
    }

//    debugScreen->setChunkInfo(chunkInfoStream.str());
}