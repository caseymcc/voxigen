#ifndef _voxigen_loadingscreen_h_
#define _voxigen_loadingscreen_h_

#include "renderingOptions.h"
#include "world.h"

#include <glbinding/gl/gl.h>
using namespace gl;

class LoadingScreen
{
public:
    LoadingScreen(RenderingOptions *renderingOptions);

    void initialize();
    void terminate();

    void setSize(int width, int height);

    void start();
    void build();
    void draw();

    void update(std::string status, int progress);

private:
    RenderingOptions *m_renderingOptions;

    int m_width;
    int m_height;
};

#endif //_voxigen_loadingscreen_h_
