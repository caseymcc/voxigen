#include <glbinding/gl/gl.h>

namespace voxigen
{

using namespace gl;

void openglDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void *userParam);

}//namespace voxigen