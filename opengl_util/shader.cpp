#include "opengl_util/shader.h"
#include "opengl_util/program.h"
#include "opengl_util/initGlew.h"

#include <filesystem>
#include <regex>

namespace opengl_util
{

Shader::Shader(GLenum type):
m_type(type)
{
    initGlew();
    m_id=glCreateShader(type);
}

Shader::~Shader()
{
}

void Shader::attach(Program *program)
{
    m_program=program;
}

void Shader::detach(Program *program)
{
    if(m_program == program)
        m_program=NULL;
    else
         assert(false);
}

bool Shader::rebuildSource(std::string &errorString)
{
    m_shaderSourceModified=m_shaderSource;
    return true;

}

bool Shader::load(const std::string &buffer, std::string &errorString)
{
    return load(buffer, "", errorString);
}

bool Shader::load(const std::string &buffer, std::string insert, std::string &errorString)
{
    if(buffer.empty())
    {
        errorString+="Failed to load shader";
        return false;
    }

    setSource(buffer);

    if(!insert.empty())
    {
        std::regex expression("^\\s*#version\\s*\\d+\\s*\\w*\\s*\\n?");
        std::match_results<std::string::const_iterator> what;

        //must insert any code after the #version identifier in the shader code
        if(std::regex_search(m_shaderSource, what, expression, std::regex_constants::match_default))
        {
            size_t pos=what.position()+what.length();
            m_shaderSource.insert(pos, insert);
        }
        else
        {
            m_shaderSource.insert(m_shaderSource.begin(), insert.begin(), insert.end());
        }
    }
    return true;
}

void Shader::setSource(const std::string &source)
{
    m_shaderSource=source;
}

bool Shader::compile(std::string &error)
{
    GLchar *sources[1];
    GLint lengths[1];

    if(!rebuildSource(error) )

        return false;

    sources[0]=(GLchar *)m_shaderSourceModified.data();
    lengths[0]=static_cast<GLint>(m_shaderSourceModified.size());

    glShaderSource(m_id, 1, sources, lengths);
    glCompileShader(m_id);

    GLint compiled=0;

    glGetShaderiv(m_id, GL_COMPILE_STATUS, &compiled);

    if(compiled == GL_FALSE)
    {
        GLint length;

        glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &length);

        std::vector<GLchar> infoLog(length);

        glGetShaderInfoLog(m_id, length, &length, infoLog.data());

        error="Failed to compile shader: ";
        error.append(infoLog.begin(), infoLog.end());
        return false;
    }
    return true;

}

} //namespace opengl_util

