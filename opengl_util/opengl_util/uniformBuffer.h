#ifndef _opengl_util_uniformBuffer_h_
#define _opengl_util_uniformBuffer_h_

#include "opengl_util_export.h"
#include "opengl_util/uniform.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <assert.h>
#include <memory>

namespace opengl_util
{

class UniformBuffer;

class OPENGL_UTIL_EXPORT BufferedUniform:public Uniform
{
public:
    BufferedUniform(UniformBuffer *uniformBuffer, const std::string &key, Type type):
    Uniform(key, type, 0), m_uniformBuffer(uniformBuffer){}

    /////////////////////////////////////////////////////////////////////////////////////////
    //If you hit any of these functions you have likely setup the uniform with one type and then
    //tried to set its value with another type. Check the Uniform::m_type and verify that it
    //is the same type you are trying to set it with.
    virtual BufferedUniform &operator=(const bool &value){assert(false); return *this;}
    virtual BufferedUniform &operator=(const int &value){assert(false); return *this;}
    virtual BufferedUniform &operator=(const unsigned int &value){assert(false); return *this;}
    virtual BufferedUniform &operator=(const float &value){assert(false); return *this;}
    virtual BufferedUniform &operator=(const double &value){assert(false); return *this;}
    virtual BufferedUniform &operator=(const glm::ivec2 &matrix){assert(false); return *this;}
    virtual BufferedUniform &operator=(const glm::ivec3 &matrix){assert(false); return *this;}
    virtual BufferedUniform &operator=(const glm::ivec4 &matrix){assert(false); return *this;}
    virtual BufferedUniform &operator=(const glm::vec2 &matrix){assert(false); return *this;}
    virtual BufferedUniform &operator=(const glm::vec3 &matrix){assert(false); return *this;}
    virtual BufferedUniform &operator=(const glm::vec4 &matrix){assert(false); return *this;}
    virtual BufferedUniform &operator=(const glm::mat4 &matrix){assert(false); return *this;}
    /////////////////////////////////////////////////////////////////////////////////////////

    void setOffset(size_t offset)
    {
        m_offset=offset;
    }

    size_t getOffset()
    {
        return m_offset;
    }

protected:
    UniformBuffer *m_uniformBuffer;
    size_t m_offset;
};

typedef std::unordered_map<std::string, BufferedUniform *> BufferedUniformMap;
typedef std::vector<BufferedUniform *> BufferedUniforms;

class Program;

class OPENGL_UTIL_EXPORT UniformBuffer
{
public:
    UniformBuffer(Program *program, GLuint blockIndex, GLuint blockBinding);
    ~UniformBuffer();

    void addUniform(std::string key, Uniform::Type value);
    BufferedUniform &uniform(std::string key);

    void update();
    void setUniform(size_t offset, const void *data, size_t size);

    void bind();

    Program *getProgram(){return m_program;}

    void buildUbo();

protected:
    friend class Program;

    Program *m_program;
    GLuint m_blockIndex;
    GLuint m_blockBinding;
    GLint m_blockSize;
    GLint m_blockOffsetAlignment; ///< Not directly needed by UniformBuffer, but used for IndexedUniformBuffer.
    GLint m_alignedBlockSize; ///< Not directly needed by UniformBuffer, but used for IndexedUniformBuffer.
    GLint m_blocksToAlloc; ///< Not directly needed by UniformBuffer, but used for IndexedUniformBuffer.

    bool m_uboInit;
    GLuint m_ubo;
    bool m_uboDirty;

    BufferedUniformMap m_uniformMap;
    BufferedUniforms m_uniforms;

    std::vector<GLubyte> m_block;
};

template<typename _ValueType, typename _SetterType=_ValueType>
class BufferedUniformTemplate;

template<typename _ValueType, typename _SetterType>
class BufferedUniformTemplate:public BufferedUniform
{
public:
    BufferedUniformTemplate(UniformBuffer *uniformBuffer, const std::string &key):BufferedUniform(uniformBuffer, key, UniformTypeInfo<_ValueType>::typeId){}

    virtual BufferedUniform &operator=(const _SetterType &value)
    {
        assert( sizeof(_ValueType) == sizeof(_SetterType) );
        m_uniformBuffer->setUniform(m_offset, &value, sizeof(_ValueType)); 
        return *this;
    }

    virtual size_t length(){return sizeof(_ValueType);}
};

typedef std::shared_ptr<UniformBuffer> SharedUniformBuffer;

} //namespace opengl_util

#endif //_opengl_util_uniformBuffer_h_
