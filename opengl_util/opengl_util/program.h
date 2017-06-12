#ifndef _opengl_util_program_h_
#define _opengl_util_program_h_

#include "opengl_util_export.h"

#include "shader.h"
#include "uniform.h"
#include "uniformBuffer.h"

namespace opengl_util
{

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

    Uniform &uniform(std::string key);

    bool attachAndLoadShader(const std::string &shaderFile, GLenum shaderType, std::string &error);
    bool attachAndLoadShader(const std::string &shaderFile, GLenum shaderType, std::string insert, std::string &error);


    bool attachLoadAndCompileShaders(const std::string &vertFile, const std::string &fragFile, std::string &error);
    bool attachLoadAndCompileShaders(const std::string &vertFile, const std::string &geomFile, const std::string &fragFile, std::string &error);

    bool attachVertShader(Shader *pShader, std::string &error);
    bool attachFragShader(Shader *pShader, std::string &error);
    bool attachGeomShader(Shader *pShader, std::string &error);

    bool attachShader(Shader *pShader, std::string &error);

    bool compile(std::string &error);
    void use();

    std::string getAttributesDefinition();
    std::string getUniformBufferDefinition();

protected:
    void generateProgram();
    void addUniform(std::string key, Uniform::Type type, GLuint location);

    Shader *m_vertexShader;
    Shader *m_fragmentShader;
    Shader *m_geometryShader;

    GLuint m_id;
    GLuint m_bindPoint;
    GLuint m_lastBlockBinding;

    AttributeDetailMap m_attributeDetails;
    UniformBufferDetailMap m_uniformBufferDetails;

	UniformMap m_uniformMap;
    Uniforms m_uniforms;
};
typedef std::shared_ptr<Program> SharedProgram;

} //namespace opengl_util



#endif //_opengl_util_program_h_

