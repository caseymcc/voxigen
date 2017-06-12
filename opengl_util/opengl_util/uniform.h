#ifndef _opengl_util_uniform_h_
#define _opengl_util_uniform_h_

#include "opengl_util_export.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <assert.h>

#include "glm/glm.hpp"

#include <GL/glew.h>

namespace opengl_util
{

class OPENGL_UTIL_EXPORT Uniform
{
public:
    enum class Type
    {
        Unknown,
        Bool,
        Int,
        UInt,
        Float,
        Double,
        ivec2,
        ivec3,
        ivec4,
        vec2,
        vec3,
        vec4,
        mat4,
		sampler1D,
		sampler1DShadow,
		sampler2D,
		sampler2DShadow,
		sampler3D,
		samplerCube,
		usampler1D,
		usampler2D,
		usampler3D,
		usamplerCube
    };

    Uniform(const std::string &key, Type type, GLuint location):
        m_key(key), m_type(type), m_location(location){}

    /////////////////////////////////////////////////////////////////////////////////////////
    //If you hit any of these functions you have likely setup the uniform with one type and then
    //tried to set its value with another type. Check the Uniform::m_type and verify that it
    //is the same type you are trying to set it with.
    virtual Uniform &operator=(const bool &value){assert(false); return *this;}
    virtual Uniform &operator=(const int &value){assert(false); return *this;}
    virtual Uniform &operator=(const unsigned int &value){assert(false); return *this;}
    virtual Uniform &operator=(const float &value){assert(false); return *this;}
    virtual Uniform &operator=(const double &value){assert(false); return *this;}
    virtual Uniform &operator=(const glm::ivec2 &matrix){assert(false); return *this;}
    virtual Uniform &operator=(const glm::ivec3 &matrix){assert(false); return *this;}
    virtual Uniform &operator=(const glm::ivec4 &matrix){assert(false); return *this;}
    virtual Uniform &operator=(const glm::vec2 &matrix){assert(false); return *this;}
    virtual Uniform &operator=(const glm::vec3 &matrix){assert(false); return *this;}
    virtual Uniform &operator=(const glm::vec4 &matrix){assert(false); return *this;}
    virtual Uniform &operator=(const glm::mat4 &matrix){assert(false); return *this;}
    /////////////////////////////////////////////////////////////////////////////////////////

    const std::string &key(){return m_key;}
    virtual size_t length(){assert(false); return 0;}
    Type type(){ return m_type;}

    template<typename _Type> void setValue(const _Type &value){assert(false);}

protected:
    std::string m_key;
    Type m_type;
    GLuint m_location;
};

template<> void Uniform::setValue<bool>(const bool &value);
template<> void Uniform::setValue<int>(const int &value);
template<> void Uniform::setValue<unsigned int>(const unsigned int &value);
template<> void Uniform::setValue<float>(const float &value);
template<> void Uniform::setValue<double>(const double &value);
template<> void Uniform::setValue<glm::ivec2>(const glm::ivec2 &value);
template<> void Uniform::setValue<glm::ivec3>(const glm::ivec3 &value);
template<> void Uniform::setValue<glm::ivec4>(const glm::ivec4 &value);
template<> void Uniform::setValue<glm::vec2>(const glm::vec2 &value);
template<> void Uniform::setValue<glm::vec3>(const glm::vec3 &value);
template<> void Uniform::setValue<glm::vec4>(const glm::vec4 &value);
template<> void Uniform::setValue<glm::mat4>(const glm::mat4 &value);

typedef std::unordered_map<std::string, Uniform *> UniformMap;
typedef std::vector<Uniform *> Uniforms;

template<typename _ValueType>
struct UniformTypeInfo
{
	static Uniform::Type const typeId=Uniform::Type::Unknown;
    static Uniform::Type id(){return typeId;}
    static const char *name(){return "Unknown";}
    static size_t length(){return sizeof(_ValueType);}
};

template<>
struct UniformTypeInfo<bool>
{
    typedef bool Type;
    static Uniform::Type const typeId=Uniform::Type::Bool;
    static const char *name(){return "bool";}
};

template<>
struct UniformTypeInfo<int>
{
    typedef int Type;
    static Uniform::Type const typeId=Uniform::Type::Int;
    static const char *name(){return "int";}
};

template<>
struct UniformTypeInfo<unsigned int>
{
    typedef unsigned int Type;
    static Uniform::Type const typeId=Uniform::Type::UInt;
    static const char *name(){return "unsigned int";}
};

template<>
struct UniformTypeInfo<float>
{
    typedef float Type;
    static Uniform::Type const typeId=Uniform::Type::Float;
    static const char *name(){return "float";}
};

template<>
struct UniformTypeInfo<double>
{
	typedef double Type;
	static Uniform::Type const typeId=Uniform::Type::Double;
	static const char *name(){return "double";}
};

template<>
struct UniformTypeInfo<glm::ivec2>
{
    typedef glm::ivec2 Type;
    static Uniform::Type const typeId=Uniform::Type::ivec2;
    static const char *name() { return "ivec2"; }
};

template<>
struct UniformTypeInfo<glm::ivec3>
{
    typedef glm::ivec3 Type;
    static Uniform::Type const typeId=Uniform::Type::ivec3;
    static const char *name() { return "ivec3"; }
};

template<>
struct UniformTypeInfo<glm::ivec4>
{
    typedef glm::ivec4 Type;
    static Uniform::Type const typeId=Uniform::Type::ivec4;
    static const char *name() { return "ivec4"; }
};

template<>
struct UniformTypeInfo<glm::vec2>
{
    typedef glm::vec2 Type;
    static Uniform::Type const typeId=Uniform::Type::vec2;
    static const char *name() { return "vec2"; }
};
template<>
struct UniformTypeInfo<glm::vec3>
{
    typedef glm::vec3 Type;
    static Uniform::Type const typeId=Uniform::Type::vec3;
    static const char *name() { return "vec3"; }
};
template<>
struct UniformTypeInfo<glm::vec4>
{
    typedef glm::vec4 Type;
    static Uniform::Type const typeId=Uniform::Type::vec4;
    static const char *name() { return "vec2"; }
};
template<>
struct UniformTypeInfo<glm::mat4>
{
    typedef glm::mat4 Type;
    static Uniform::Type const typeId=Uniform::Type::mat4;
    static const char *name(){return "mat4";}
};

std::string uniformTypeName(Uniform::Type type);
Uniform::Type uniformType(GLenum glType);

template<typename _ValueType, typename _SetterType=_ValueType>
class UniformTemplate;

template<typename _ValueType, typename _SetterType>
class UniformTemplate:public Uniform
{
public:
    UniformTemplate(const std::string &key, GLuint location):Uniform(key, UniformTypeInfo<_ValueType>::typeId, location){}

    virtual Uniform &operator=(const _SetterType &value)
    {
        assert( sizeof(_ValueType) == sizeof(_SetterType) );
        setValue(value);
        return *this;
    }
    virtual size_t length(){return sizeof(_ValueType);}
};

} //namespace _opengl_util_uniform_h_



#endif //opengl_util

