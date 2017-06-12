#ifndef _opengl_util_shader_h_
#define _opengl_util_shader_h_

#include "opengl_util_export.h"

#include <vector>
#include <unordered_map>

#include <GL/glew.h>

namespace opengl_util
{

class Program;

class OPENGL_UTIL_EXPORT Shader
{

public:
    Shader(GLenum type);
    virtual ~Shader();

    GLenum type() { return m_type; }
    GLuint id(){return m_id;}
    
    void attach(Program *program);
    void detach(Program *program);

    bool load(const std::string &buffer, std::string &errorString);
    bool load(const std::string &buffer, std::string insert, std::string &errorString);

    void setSource(const std::string &source);
    bool compile(std::string &error);

protected:
    bool rebuildSource(std::string &errorString);

    Program *m_program;
    GLenum m_type;
    GLuint m_id;
    std::string m_shaderSource;
    std::string m_shaderSourceModified;
};



} //namespace opengl_util



#endif //_opengl_util_shader_h_

