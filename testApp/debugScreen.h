#ifndef _voxigen_debugscreen_h_
#define _voxigen_debugscreen_h_

#include <glbinding/gl/gl.h>
using namespace gl;

#include "renderingOptions.h"
#include "world.h"

class DebugScreen
{
public:
    DebugScreen(RenderingOptions *renderingOptions);

    void initialize();
    void terminate();

    void setSize(int width, int height);

    void startBuild();

    void update(World *world);
    void updateControls();
    void updateChunkInfo(WorldRenderer *renderer);

    void build();
    void draw();
//    void setChunkInfo(std::string &info);

//    void togglePlayer_change(bool state);
private:
    RenderingOptions *m_renderingOptions;

    int m_width;
    int m_height;
};

#endif //_voxigen_debugscreen_h_
