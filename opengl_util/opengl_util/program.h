#ifndef _opengl_util_program_h_
#define _opengl_util_program_h_

#include "opengl_util_export.h"

#include "shader.h"
#include "uniform.h"
#include "uniformBuffer.h"

#include <generic/fileIO.h>
#include <generic/stdFileIO.h>

namespace opengl_util
{

typedef std::unordered_map<std::string, size_t> UniformIdMap;

struct OPENGL_UTIL_EXPORT AttributeDetail
{
    AttributeDetail(std::string key, Uniform::Type type, GLuint index):key(key), type(type), index(index){}

    std::string key;
    Uniform::Type type;
    GLuint index;
};
typedef std::unordered_map<std::string, AttributeDetail> AttributeDetailMap;


struct OPENGL_UTIL_EXPORT UniformDetail
{
    UniformDetail(std::string key, Uniform::Type type):key(key), type(type){}

    std::string key;
    Uniform::Type type;

};
typedef std::vector<UniformDetail> UniformDetails;

class OPENGL_UTIL_EXPORT UniformBufferDetail
{
public:
    UniformBufferDetail(GLuint index):blockBinding(index){};

    GLuint blockIndex;
    GLuint blockBinding;
    UniformDetails uniformDetails;
};
typedef std::shared_ptr<UniformBufferDetail> SharedUniformBufferDetail;
typedef std::unordered_map<std::string, SharedUniformBufferDetail> UniformBufferDetailMap;

class OPENGL_UTIL_EXPORT Program
{
public:
    Program();
    virtual ~Program();

    GLuint id(){ return m_id; }

    GLuint getAttributeLocation(std::string key);

    UniformBuffer *createUniformBuffer(std::string name);
    bool useUniformBuffer(UniformBuffer *buffer);

    size_t getUniformId(std::string key);

    Uniform &uniform(size_t id);
    Uniform &uniform(std::string key);

    template<typename _FilIO=generic::io::StdFileIO>
    std::string loadShader(const std::string &shaderFile, void *userData=nullptr);

    template<typename _FileIO=generic::io::StdFileIO>
    bool load(const std::string &vertShaderFile, std::string vertInsert, const std::string &fragShaderFile, std::string fragInsert, std::string &error, void *userData=nullptr);
    template<typename _FilIO=generic::io::StdFileIO>
    bool load(const std::string &vertShaderFile, const std::string &fragShaderFile, std::string &error, void *userData=nullptr);

    bool attachAndLoadShader(const std::string &shaderSource, GLenum shaderType, std::string &error);
    bool attachAndLoadShader(const std::string &shaderSource, GLenum shaderType, std::string insert, std::string &error);

    bool attachLoadAndCompileShaders(const std::string &vertSource, std::string vertInsert, const std::string &fragSource, std::string fragInsert, std::string &error);
    bool attachLoadAndCompileShaders(const std::string &vertSource, const std::string &fragSource, std::string &error);
    bool attachLoadAndCompileShaders(const std::string &vertSource, const std::string &geomSource, const std::string &fragSource, std::string &error);

    bool attachVertShader(Shader *shader, std::string &error);
    bool attachFragShader(Shader *shader, std::string &error);
    bool attachGeomShader(Shader *shader, std::string &error);

    bool attachShader(Shader *shader, std::string &error);

    bool compile(std::string &error);
    void use();

    std::string getAttributesDefinition();
    std::string getUniformBufferDefinition();

protected:
    void generateProgram();
    void addUniform(std::string key, Uniform::Type type, GLuint location);
    void clear();

    Shader *m_vertexShader;
    Shader *m_fragmentShader;
    Shader *m_geometryShader;

    GLuint m_id;
    GLuint m_bindPoint;
    GLuint m_lastBlockBinding;

    AttributeDetailMap m_attributeDetails;
    UniformBufferDetailMap m_uniformBufferDetails;

    UniformIdMap m_uniformIdMap;
    Uniforms m_uniforms;

#ifndef NDEBUG
    UniformDummy m_dummyUniform; //used to return on invalid uniform
#endif
};
typedef std::shared_ptr<Program> SharedProgram;

template<typename _FileIO>
std::string Program::loadShader(const std::string &shaderFile, void *userData)
{
    std::string shader;
    _FileIO::Type *file=generic::io::open<_FileIO>(shaderFile, "rb", userData);

    if(!file)
        return shader;

    size_t bufferSize=generic::io::size<_FileIO>(file);

    shader.resize(bufferSize);
    generic::io::read<_FileIO>(shader.data(), sizeof(std::string::value_type), bufferSize, file);

    generic::io::close<_FileIO>(file);

    return shader;
}

template<typename _FileIO>
bool Program::load(const std::string &vertShaderFile, std::string vertInsert, const std::string &fragShaderFile, std::string fragInsert, std::string &error, void *userData)
{
    std::string vertShader=loadShader<_FileIO>(vertShaderFile, userData);
    std::string fragShader=loadShader<_FileIO>(fragShaderFile, userData);

    return attachLoadAndCompileShaders(vertShader, vertInsert, fragShader, fragInsert, error);
}

template<typename _FileIO>
bool Program::load(const std::string &vertShaderFile, const std::string &fragShaderFile, std::string &error, void *userData)
{
    return load(vertShaderFile, "", fragShaderFile, "", error, userData);
}



} //namespace opengl_util



#endif //_opengl_util_program_h_

