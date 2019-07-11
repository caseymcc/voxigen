#include "voxigen/nativeGL.h"

#include <cassert>
#if defined(_WIN32) || defined(_WIN64)
#include "windows.h"
#elif __linux__
#include <GL/glx.h>
#elif __APPLE__
#include <OpenGL/OpenGL.h>
#endif

namespace voxigen
{

struct NativeStruct
{   
#if defined(_WIN32) || defined(_WIN64)
    HDC m_dc;
    HGLRC m_glContext;
#elif __linux__
    Display *m_display;
    GLXDrawable m_drawable;
    GLXContext m_glContext;
#elif __APPLE__
    CGLPixelFormatObj m_pixelFormat;
    CGLContextObj m_glContext;
#endif
};

NativeGL::NativeGL()
{
    hidden=std::make_unique<NativeStruct>();
}

NativeGL::~NativeGL()
{}

void NativeGL::createSharedContext()
{
#if defined(_WIN32) || defined(_WIN64)
    hidden->m_dc=wglGetCurrentDC();
    HGLRC currentContext=wglGetCurrentContext();

    hidden->m_glContext=wglCreateContext(hidden->m_dc);

    BOOL success=wglShareLists(currentContext, hidden->m_glContext);

    assert(success);
#elif __linux__
    hidden->m_display=glXGetCurrentDisplay();
    hidden->m_drawable=glXGetCurrentDrawable();
    GLXContext currentContext=glXGetCurrentContext();

    int nelements;
    GLXFBConfig *fbConfig=glXChooseFBConfig(hidden->m_display, DefaultScreen(hidden->m_display), 0, &nelements);

    hidden->m_glContext=glXCreateNewContext(hidden->m_display, *fbConfig, GLX_RGBA_TYPE, currentContext, true);

    assert(hidden->m_glContext);
#elif __APPLE__
    CGLContextObj currentContext=CGLGetCurrentContext();
    hidden->m_pixelFormat=CGLGetPixelFormat(currentContext);

    CGLError error=CGLCreateContext(hidden->m_pixelFormat, currentContext, &hidden->m_glContext);

    assert(error == kCGLNoError);
#endif

}

void NativeGL::makeCurrent()
{
#if defined(_WIN32) || defined(_WIN64)
    wglMakeCurrent(hidden->m_dc, hidden->m_glContext);
#elif __linux__
    glXMakeCurrent(hidden->m_display, hidden->m_drawable, hidden->m_glContext);
#elif __APPLE__
    CGLSetCurrentContext(hidden->m_glContext);
#endif

}

}//namespace voxigen

