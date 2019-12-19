#include "voxigen/rendering/openglDebug.h"
#include "voxigen/fileio/log.h"

namespace voxigen
{

using namespace gl;

void openglDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam)
{
#ifndef NDEBUG
#ifdef VOXIGEN_USE_LOGGING
    if(type==GL_DEBUG_TYPE_ERROR)
        LOG(INFO)<<"Opengl - error: "<<message;
    //    else
    //        LOG(INFO)<<"Opengl : "<<message;
#endif//VOXIGEN_USE_LOGGING
#endif
}

}//namespace voxigen