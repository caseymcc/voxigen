#include "opengl_util/uniformBuffer.h"
#include "opengl_util/program.h"

#include <assert.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace opengl_util
{

template<> void Uniform::setValue<bool>(const bool &value)
{   glUniform1i(m_location, value);}
template<> void Uniform::setValue<int>(const int &value)
{   glUniform1i(m_location, value);}
template<> void Uniform::setValue<unsigned int>(const unsigned int &value)
{   glUniform1i(m_location, value);}
template<> void Uniform::setValue<float>(const float &value)
{   glUniform1f(m_location, value);}
template<> void Uniform::setValue<double>(const double &value)
{   glUniform1f(m_location, (float)value);}
template<> void Uniform::setValue<glm::ivec2>(const glm::ivec2 &value)
{   glUniform2iv(m_location, 1, glm::value_ptr(value));}
template<> void Uniform::setValue<glm::ivec3>(const glm::ivec3 &value)
{   glUniform3iv(m_location, 1, glm::value_ptr(value));}
template<> void Uniform::setValue<glm::ivec4>(const glm::ivec4 &value)
{   glUniform4iv(m_location, 1, glm::value_ptr(value));}
template<> void Uniform::setValue<glm::vec2>(const glm::vec2 &value)
{   glUniform2fv(m_location, 1, glm::value_ptr(value));}
template<> void Uniform::setValue<glm::vec3>(const glm::vec3 &value)
{   glUniform3fv(m_location, 1, glm::value_ptr(value));}
template<> void Uniform::setValue<glm::vec4>(const glm::vec4 &value)
{   glUniform4fv(m_location, 1, glm::value_ptr(value));}
template<> void Uniform::setValue<glm::mat4>(const glm::mat4 &value)
{   glUniformMatrix4fv(m_location, 1, GL_FALSE, glm::value_ptr(value));}

std::string uniformTypeName(Uniform::Type type)
{
    std::string typeName="int";

    switch(type)
    {
    case Uniform::Type::Bool:
        typeName=UniformTypeInfo<bool>::name();
        break;
    case Uniform::Type::Int:
        typeName=UniformTypeInfo<int>::name();
        break;
    case Uniform::Type::UInt:
        typeName=UniformTypeInfo<unsigned int>::name();
        break;
    case Uniform::Type::Float:
       typeName=UniformTypeInfo<float>::name();
        break;
    case Uniform::Type::Double:
        typeName=UniformTypeInfo<double>::name();
        break;
    case Uniform::Type::ivec2:
        typeName=UniformTypeInfo<glm::ivec2>::name();
        break;
    case Uniform::Type::ivec3:
        typeName=UniformTypeInfo<glm::ivec3>::name();
        break;
    case Uniform::Type::ivec4:
        typeName=UniformTypeInfo<glm::ivec4>::name();
        break;
    case Uniform::Type::vec2:
        typeName=UniformTypeInfo<glm::vec2>::name();
        break;
    case Uniform::Type::vec3:
        typeName=UniformTypeInfo<glm::vec3>::name();
        break;
    case Uniform::Type::vec4:
        typeName=UniformTypeInfo<glm::vec4>::name();
        break;
    case Uniform::Type::mat4:
        typeName=UniformTypeInfo<glm::mat4>::name();;
        break;
    default:
        assert(false);
        break;
    }
    return typeName;
}


Uniform::Type uniformType(GLenum glType)
{
    static std::unordered_map<GLenum, Uniform::Type> uniformGlMap;

    if(uniformGlMap.empty())
    {
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_BOOL, Uniform::Type::Bool));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_INT, Uniform::Type::Int));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_UNSIGNED_INT, Uniform::Type::UInt));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_FLOAT, Uniform::Type::Float));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_DOUBLE, Uniform::Type::Double));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_INT_VEC2, Uniform::Type::ivec2));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_INT_VEC3, Uniform::Type::ivec3));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_INT_VEC4, Uniform::Type::ivec4));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_FLOAT_VEC2, Uniform::Type::vec2));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_FLOAT_VEC3, Uniform::Type::vec3));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_FLOAT_VEC4, Uniform::Type::vec4));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_FLOAT_MAT4, Uniform::Type::mat4));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_SAMPLER_1D, Uniform::Type::sampler1D));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_SAMPLER_1D_SHADOW, Uniform::Type::sampler1DShadow));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_SAMPLER_2D, Uniform::Type::sampler2D));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_SAMPLER_2D_SHADOW, Uniform::Type::sampler2DShadow));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_SAMPLER_3D, Uniform::Type::sampler3D));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_SAMPLER_CUBE, Uniform::Type::samplerCube));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_UNSIGNED_INT_SAMPLER_1D, Uniform::Type::usampler1D));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_UNSIGNED_INT_SAMPLER_2D, Uniform::Type::usampler2D));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_UNSIGNED_INT_SAMPLER_3D, Uniform::Type::usampler3D));
        uniformGlMap.insert(std::unordered_map<GLenum, Uniform::Type>::value_type(GL_UNSIGNED_INT_SAMPLER_CUBE, Uniform::Type::usamplerCube));
    }

    std::unordered_map<GLenum, Uniform::Type>::iterator iter=uniformGlMap.find(glType);

    if(iter==uniformGlMap.end())
        return Uniform::Type::Unknown;

    return iter->second;
}


} //namespace opengl_util

