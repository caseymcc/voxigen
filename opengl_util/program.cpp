#include "opengl_util/program.h"
#include "opengl_util/initGlew.h"

#include <filesystem>
#include <regex>

namespace opengl_util
{

Program::Program():
m_vertexShader(nullptr),
m_fragmentShader(nullptr),
m_geometryShader(nullptr),
m_id(0)
{
}



Program::~Program()
{
    if(m_vertexShader!= nullptr)
        delete m_vertexShader;
    if(m_fragmentShader!= nullptr)
        delete m_fragmentShader;
    if(m_geometryShader!= nullptr)
        delete m_geometryShader;
}

void Program::generateProgram()
{
    initGlew();
    if(m_id == 0)
        m_id=glCreateProgram();
}

GLuint Program::getAttributeLocation(std::string key)
{
    AttributeDetailMap::iterator iter=m_attributeDetails.find(key);

    if(iter==m_attributeDetails.end())
    {
        assert(false);
        return 0;
    }

    return iter->second.index;
}

void Program::addUniform(std::string key, Uniform::Type type, GLuint location)
{
    Uniform *uniform=nullptr;

    switch(type)
    {
    case Uniform::Type::Bool:
        uniform=new UniformTemplate<bool>(key, location);
        break;
    case Uniform::Type::Int:
        uniform=new UniformTemplate<int>(key, location);
        break;
    case Uniform::Type::UInt:
        uniform=new UniformTemplate<unsigned int>(key, location);
        break;
    case Uniform::Type::Float:
        uniform=new UniformTemplate<float>(key, location);
        break;
    case Uniform::Type::Double:
        uniform=new UniformTemplate<double>(key, location);
        break;
    case Uniform::Type::ivec2:
        uniform=new UniformTemplate<glm::ivec2>(key, location);
        break;
    case Uniform::Type::ivec3:
        uniform=new UniformTemplate<glm::ivec3>(key, location);
        break;
    case Uniform::Type::ivec4:
        uniform=new UniformTemplate<glm::ivec4>(key, location);
        break;
    case Uniform::Type::vec2:
        uniform=new UniformTemplate<glm::vec2>(key, location);
        break;
    case Uniform::Type::vec3:
        uniform=new UniformTemplate<glm::vec3>(key, location);
        break;
    case Uniform::Type::vec4:
        uniform=new UniformTemplate<glm::vec4>(key, location);
        break;
    case Uniform::Type::mat4:
        uniform=new UniformTemplate<glm::mat4>(key, location);
        break;
    case Uniform::Type::sampler1D:
        return;
        break;
    case Uniform::Type::sampler1DShadow:
        return;
        break;
    case Uniform::Type::sampler2D:
        return;
        break;
    case Uniform::Type::sampler2DShadow:
        return;
        break;
    case Uniform::Type::sampler3D:
        return;
        break;
    case Uniform::Type::samplerCube:
        return;
        break;
    case Uniform::Type::usampler1D:
        return;
        break;
    case Uniform::Type::usampler2D:
        return;
        break;
    case Uniform::Type::usampler3D:
        return;
        break;
    case Uniform::Type::usamplerCube:
        return;
        break;
    }

    if(uniform!=nullptr)
    {
        m_uniforms.push_back(uniform);
        m_uniformMap.insert(UniformMap::value_type(key, uniform));
    }
}

UniformBuffer *Program::createUniformBuffer(std::string name)
{
    //generate program if not already done
    generateProgram();

    UniformBufferDetailMap::iterator iter=m_uniformBufferDetails.find(name);

    if(iter == m_uniformBufferDetails.end())
        return nullptr;

    UniformBufferDetail *bufferDetail=iter->second.get();
    UniformBuffer *buffer=new UniformBuffer(this, bufferDetail->blockIndex, bufferDetail->blockBinding);

    for(UniformDetail &detail : bufferDetail->uniformDetails)
        buffer->addUniform(detail.key, detail.type);

    buffer->buildUbo();
    return buffer;
}

bool Program::useUniformBuffer(UniformBuffer *buffer)
{
    assert(buffer->getProgram() == this); //buffer was not created for this program

    buffer->bind();
    return false;

}

Uniform &Program::uniform(std::string key)
{
    assert(m_uniformMap.find(key) != m_uniformMap.end()); //uniform does not exist in program
    return *(m_uniformMap[key]);
}

bool Program::attachAndLoadShader(const std::string &shaderFile, GLenum shaderType, std::string &error)
{
    return attachAndLoadShader(shaderFile, shaderType, "", error);
}

bool Program::attachAndLoadShader(const std::string &shaderFile, GLenum shaderType, std::string insert, std::string &error)
{
    bool success=false;
    Shader *shader=new Shader(shaderType);

    switch(shaderType)
    {
        case(GL_VERTEX_SHADER):
        case(GL_FRAGMENT_SHADER):
        case(GL_GEOMETRY_SHADER):
            success=attachShader(shader, error);

            if(!success)
            {
                delete shader;
                shader= nullptr;
            }
            break;
        default:
            error += "Error: Unsupported shader type ";
            break;
    }

    if(success && shader->load(shaderFile, insert, error))
        return true;
    return false;
}



bool Program::attachLoadAndCompileShaders(const std::string &vertFile, const std::string &fragFile, std::string &error)
{
    if(attachAndLoadShader(vertFile, GL_VERTEX_SHADER, error))
    {
        if(attachAndLoadShader(fragFile, GL_FRAGMENT_SHADER, error))
        {
            if(compile(error))
                return true;
        }
    }
    return false;
}



bool Program::attachLoadAndCompileShaders(const std::string &vertFile, const std::string &geomFile, const std::string &fragFile, std::string &error)
{
    if(attachAndLoadShader(vertFile, GL_VERTEX_SHADER, error))
    {
        if(attachAndLoadShader(geomFile, GL_GEOMETRY_SHADER, error))
        {
            if(attachAndLoadShader(fragFile, GL_FRAGMENT_SHADER, error))
            {
                if(compile(error))
                    return true;
            }
        }
    }
    return false;

}



bool Program::attachVertShader(Shader *shader, std::string &error)
{
    if(shader && shader->type() == GL_VERTEX_SHADER)
    {
        if(attachShader(shader, error))
            return true;
    }
    else
    {
        error += "Error: The shader could not be attached.  Either the shader is nullptr, or "
            "it is not of type GL_VERTEX_SHADER";
    }
    return false;
}

bool Program::attachFragShader(Shader *shader, std::string &error)

{
    if(shader && shader->type() == GL_FRAGMENT_SHADER)
    {
        if(attachShader(shader, error))
            return true;
    }
    else
    {
        error += "Error: The shader could not be attached.  Either the shader is nullptr, or "
            "it is not of type GL_FRAGMENT_SHADER";
    }
    return false;
}



bool Program::attachGeomShader(Shader *pShader, std::string &error)
{
    if(pShader && pShader->type() == GL_GEOMETRY_SHADER)
    {
        if(attachShader(pShader, error))
            return true;
    }
    else
    {
        error += "Error: The shader could not be attached.  Either the shader is nullptr, or "
            "it is not of type GL_GEOMETRY_SHADER";
    }
    return false;
}



bool Program::attachShader(Shader *shader, std::string &error)
{
    if(!shader)
    {
        error += "Error: A nullptr shader cannot be attached";
        return false;
    }

    Shader *programShader=nullptr;
    GLenum shaderType = shader->type();

    switch(shaderType)
    {
        case(GL_VERTEX_SHADER):
            programShader=m_vertexShader;
            break;
        case(GL_FRAGMENT_SHADER):
            programShader=m_fragmentShader;
            break;
        case(GL_GEOMETRY_SHADER):
            programShader=m_geometryShader;
            break;
        default:
            error += "Error: Unsupported shader type";
            return false;
    }

    if(programShader==shader)
        return true;

    generateProgram();

    if(programShader)
    {
        glDetachShader(m_id, programShader->id());
        programShader->detach(this);
    }

    programShader=shader;
    programShader->attach(this);

    glAttachShader(m_id, programShader->id());

    switch(shaderType)
    {
        case(GL_VERTEX_SHADER):
            m_vertexShader=programShader;
            break;
        case(GL_FRAGMENT_SHADER):
            m_fragmentShader=programShader;
            break;
        case(GL_GEOMETRY_SHADER):
            m_geometryShader=programShader;
            break;
        default:
            break;
    }
    return true;
}



bool Program::compile(std::string &errorString)
{
    generateProgram();

    bool linkAttempt=false;
    bool error=false;

    for(AttributeDetailMap::value_type &value:m_attributeDetails)
    {
        AttributeDetail &attributeDetail=value.second;

        glBindAttribLocation(m_id, attributeDetail.index, attributeDetail.key.c_str());
    }

    if(m_vertexShader!= nullptr)
    {
        std::string shaderError;

        if(!m_vertexShader->compile(shaderError))
        {
            error=true;
            errorString+="Vert Shader error: ";
            errorString+=shaderError;
        }

        linkAttempt=true;
    }

    if(m_fragmentShader!= nullptr)
    {
        std::string shaderError;

        if(!m_fragmentShader->compile(shaderError))
        {
            error=true;
            errorString+="Frag Shader error: ";
            errorString+=shaderError;
        }
        linkAttempt=true;
    }

    if(m_geometryShader!= nullptr)
    {
        std::string shaderError;

        if(!m_geometryShader->compile(shaderError))
        {
            error=true;
            errorString+="Geom Shader error: ";
            errorString+=shaderError;
        }
        linkAttempt=true;
    }

    if(!linkAttempt)
    {
        errorString="No shader attached";

        return false;
    }

    if(error)
        return false;

    glLinkProgram(m_id);

    GLint linked=0;

    glGetProgramiv(m_id, GL_LINK_STATUS, (int *)&linked);

    if(linked == GL_FALSE)
    {
        GLint length=0;

        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);

        std::vector<GLchar> infoLog(length);

        glGetProgramInfoLog(m_id, length, &length, infoLog.data());

        errorString="Failed to link program: ";
        errorString.append(infoLog.begin(), infoLog.end());
        return false;
    }

    if(!m_uniformBufferDetails.empty())
        m_uniformBufferDetails.clear();
    m_lastBlockBinding=1;

//Get Attributes
    GLint activeAttributes;
    GLint attributeMaxNameLength;

    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeMaxNameLength);

    std::vector<GLchar> attributeName(attributeMaxNameLength);

    for(GLint i=0; i < activeAttributes; ++i)
    {
        GLsizei length;
        GLint size;
        GLenum type;

        glGetActiveAttrib(m_id, i, (GLsizei)attributeName.size(), &length, &size, &type, attributeName.data());

        GLint location=glGetAttribLocation(m_id, attributeName.data());
        Uniform::Type uniformType=opengl_util::uniformType(type);
        std::string key=attributeName.data();

        AttributeDetailMap::iterator iter=m_attributeDetails.find(key);

        if(iter == m_attributeDetails.end())
        {
            m_attributeDetails.insert(AttributeDetailMap::value_type(key, AttributeDetail(key, uniformType, location)));
        }
        else
        {
            AttributeDetail &detail=iter->second;

            detail.key=key;
            detail.type=uniformType;
            detail.index=location;
        }
    }



//Get uniform blocks
    GLint uniformBlocks;
    GLint uniformBlockMaxNameLength;
    GLint uniformMaxNameLength;

    glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_BLOCKS, &uniformBlocks);
    glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniformBlockMaxNameLength);
    glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxNameLength);

    std::vector<GLchar> blockName(uniformBlockMaxNameLength);
    std::vector<GLchar> uniformName(uniformMaxNameLength);

    for(GLint i=0; i<uniformBlocks; ++i)
    {
        GLsizei length;
        GLint blockBinding;
        GLint numBlockUniforms;
        GLuint blockIndex;

        glGetActiveUniformBlockName(m_id, i, uniformBlockMaxNameLength, &length, blockName.data());
        blockIndex=glGetUniformBlockIndex(m_id, blockName.data());
        glGetActiveUniformBlockiv(m_id, blockIndex, GL_UNIFORM_BLOCK_BINDING, &blockBinding);

        if(blockBinding == 0)
        {
            blockBinding=m_lastBlockBinding;
            ++m_lastBlockBinding;
        }

        UniformBufferDetail *uniformBufferDetail=new UniformBufferDetail(blockBinding);
        SharedUniformBufferDetail detail(uniformBufferDetail);

        uniformBufferDetail->blockIndex=blockIndex;
        m_uniformBufferDetails.insert(UniformBufferDetailMap::value_type(blockName.data(), detail));

        glGetActiveUniformBlockiv(m_id, uniformBufferDetail->blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numBlockUniforms);

        std::vector<GLint> indices(numBlockUniforms);
        std::vector<GLint> types(numBlockUniforms);

        glGetActiveUniformBlockiv(m_id, uniformBufferDetail->blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());

        std::vector<GLuint> uindices(indices.begin(), indices.end());

        glGetActiveUniformsiv(m_id, numBlockUniforms, uindices.data(), GL_UNIFORM_TYPE, types.data());

        for(GLint j=0; j<numBlockUniforms; ++j)
        {
            GLsizei uniformNameLength;

            glGetActiveUniformName(m_id, indices[j], uniformMaxNameLength, &uniformNameLength, uniformName.data());
            uniformBufferDetail->uniformDetails.push_back(UniformDetail(uniformName.data(), uniformType(types[j])));
        }
        glUniformBlockBinding(m_id, uniformBufferDetail->blockIndex, uniformBufferDetail->blockBinding);

    }

//get uniforms
    GLint uniforms;
    
    glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &uniforms);

    for(GLint i=0; i<uniforms; ++i)
    {
        GLsizei uniformNameLength;
        GLint size;
        GLenum type;
        GLuint location;

        glGetActiveUniform(m_id, i, uniformMaxNameLength, &uniformNameLength, &size, &type, uniformName.data());
        location=glGetUniformLocation(m_id, uniformName.data());
        
        std::vector<GLchar>::iterator iter=std::find(uniformName.begin(), uniformName.begin()+uniformNameLength, '.');
        
        if(iter == uniformName.begin()+uniformNameLength)
            addUniform(uniformName.data(), uniformType(type), location);
    }
    return true;
}



void Program::use()
{
    assert(m_id != 0);
    glUseProgram(m_id);
}



} //namespace opengl_util

