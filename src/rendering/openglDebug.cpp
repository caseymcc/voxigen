#include "voxigen/rendering/openglDebug.h"
#include "voxigen/fileio/log.h"

#include <unordered_map>

namespace voxigen
{

using namespace gl;

void openglDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
{
#ifndef NDEBUG
#ifdef VOXIGEN_USE_LOGGING
    if(type==GL_DEBUG_TYPE_ERROR)
        Log::error("Opengl - error: %s", message);
    //    else
    //        LOG(INFO)<<"Opengl : "<<message;
#endif//VOXIGEN_USE_LOGGING
#endif
}

std::unordered_map<gl::GLenum, std::string> glErrorStrings=
{
    {gl::GL_NO_ERROR, "GL_NO_ERROR"},
    {gl::GL_INVALID_ENUM, "GL_INVALID_ENUM"},
    {gl::GL_INVALID_VALUE, "GL_INVALID_VALUE"},
    {gl::GL_INVALID_OPERATION, "GL_INVALID_OPERATION"},
    {gl::GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION"},
    {gl::GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY"},
    {gl::GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW"},
    {gl::GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW"}
};

void checkGLError()
{
#ifndef NDEBUG
    //    assert(gl::glGetError()==gl::GL_NO_ERROR);
    gl::GLenum error=gl::glGetError();

    if(error!=gl::GL_NO_ERROR)
    {
        Log::error("Opengl Error: %d - %s\n", error, glErrorStrings[error].c_str());
    }
#endif
}

}//namespace voxigen