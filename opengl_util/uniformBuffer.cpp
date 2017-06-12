#include "opengl_util/uniformBuffer.h"
#include "opengl_util/program.h"

#include <assert.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace opengl_util
{

UniformBuffer::UniformBuffer(Program *program, GLuint blockIndex, GLuint blockBinding):
m_uboInit(false),
m_ubo(0),
m_program(program),
m_blockIndex(blockIndex),
m_blockBinding(blockBinding),
m_blockSize(0),
m_blockOffsetAlignment(0),
m_blocksToAlloc(1),
m_uboDirty(true)
{
}

UniformBuffer::~UniformBuffer()
{
    for(Uniform *uniform:m_uniforms)
    {
        delete uniform;
    }
    m_uniforms.clear();
    m_uniformMap.clear();

    if( m_ubo )
    {
        glDeleteBuffers( 1, &m_ubo );
        m_ubo = 0;
    }
}

void UniformBuffer::addUniform(std::string key, Uniform::Type type)
{
    BufferedUniform *uniform=nullptr;
    
    switch(type)
    {
    case Uniform::Type::Bool:
        uniform=new BufferedUniformTemplate<bool>(this, key);
        break;
    case Uniform::Type::Int:
        uniform=new BufferedUniformTemplate<int>(this, key);
        break;
    case Uniform::Type::UInt:
        uniform=new BufferedUniformTemplate<unsigned int>(this, key);
        break;
    case Uniform::Type::Float:
        uniform=new BufferedUniformTemplate<float>(this, key);
        break;
    case Uniform::Type::Double:
        uniform=new BufferedUniformTemplate<double>(this, key);
        break;
    case Uniform::Type::ivec2:
        uniform=new BufferedUniformTemplate<glm::ivec2>(this, key);
        break;
    case Uniform::Type::ivec3:
        uniform=new BufferedUniformTemplate<glm::ivec3>(this, key);
        break;
    case Uniform::Type::ivec4:
        uniform=new BufferedUniformTemplate<glm::ivec4>(this, key);
        break;
    case Uniform::Type::vec2:
        uniform=new BufferedUniformTemplate<glm::vec2>(this, key);
        break;
    case Uniform::Type::vec3:
        uniform=new BufferedUniformTemplate<glm::vec3>(this, key);
        break;
    case Uniform::Type::vec4:
        uniform=new BufferedUniformTemplate<glm::vec4>(this, key);
        break;
    case Uniform::Type::mat4:
        uniform=new BufferedUniformTemplate<glm::mat4>(this, key);
        break;
    }

    if(uniform != nullptr)
    {
        m_uniforms.push_back(uniform);
        m_uniformMap.insert(BufferedUniformMap::value_type(key, uniform));
    }
    else
        assert(false);
}


BufferedUniform &UniformBuffer::uniform(std::string key)
{
    assert(m_uboInit); //buildUbo must be called before accessing the uniforms
    assert(m_uniformMap.find(key) != m_uniformMap.end()); //uniform does not exists

    return *(m_uniformMap[key]);
}

void UniformBuffer::buildUbo()
{
    glGetIntegerv( GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &m_blockOffsetAlignment );
    glGetActiveUniformBlockiv(m_program->id(), m_blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &m_blockSize);
    
    if( m_blockSize <= m_blockOffsetAlignment )
    {
        m_alignedBlockSize = m_blockOffsetAlignment;
    }
    else
    {
        m_alignedBlockSize = m_blockOffsetAlignment;
        while( m_alignedBlockSize < m_blockSize )
        {
            m_alignedBlockSize += m_blockOffsetAlignment;
        }
    }
    
    if(m_block.size() != m_alignedBlockSize*m_blocksToAlloc)
    {
        m_block.resize(m_alignedBlockSize*m_blocksToAlloc);
    }
    
    size_t uniformSize=m_uniforms.size();
    std::vector<std::vector<GLchar>> namesHolder(uniformSize);
    std::vector<GLchar*> names(uniformSize);
    std::vector<GLuint> indices(uniformSize);
    std::vector<GLint> offset(uniformSize);
    std::vector<GLint> sizes(uniformSize);
    std::vector<GLint> arrayStride(uniformSize);
    std::vector<GLint> matrixStride(uniformSize);

    size_t nameIndex=0;
    for(Uniform *uniform:m_uniforms)
    {
        const std::string &key=uniform->key();
        std::vector<GLchar> &name=namesHolder[nameIndex];

        name.resize(key.size()+1);
        strcpy_s(name.data(), key.length()+1, key.data());

        names[nameIndex]=name.data();
        ++nameIndex;
    }

    glGetUniformIndices(m_program->id(), (GLsizei)uniformSize, names.data(), indices.data());
    
    for(size_t i=0; i<indices.size(); ++i)
    {
        if( indices[i] == 0xffffffff )
            assert(indices[i] != 0xffffffff);
    }

    glGetActiveUniformsiv(m_program->id(), (GLsizei)uniformSize, indices.data(), GL_UNIFORM_OFFSET, offset.data());
    glGetActiveUniformsiv(m_program->id(), (GLsizei)uniformSize, indices.data(), GL_UNIFORM_SIZE, sizes.data());
    glGetActiveUniformsiv(m_program->id(), (GLsizei)uniformSize, indices.data(), GL_UNIFORM_ARRAY_STRIDE, arrayStride.data());
    glGetActiveUniformsiv(m_program->id(), (GLsizei)uniformSize, indices.data(), GL_UNIFORM_MATRIX_STRIDE, matrixStride.data());

    for(size_t i=0; i<uniformSize; ++i)
    {
        BufferedUniform *uniform=m_uniforms[i];

        uniform->setOffset(offset[i]);
    }

    glGenBuffers(1, &m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);

    glBufferData(GL_UNIFORM_BUFFER, m_block.size(), m_block.data(), GL_STATIC_DRAW);
    m_uboInit=true;
}

void UniformBuffer::update()
{
    m_uboDirty=false;
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, m_block.size(), m_block.data());
}

void UniformBuffer::setUniform(size_t offset, const void *data, size_t size)
{
    assert(m_uboInit);
    assert(offset+size <= m_block.size());

    memcpy(m_block.data()+offset, data, size);
    m_uboDirty=true;
}

void UniformBuffer::bind()
{
    if(m_uboDirty)
        update();

    glBindBufferBase(GL_UNIFORM_BUFFER, m_blockBinding, m_ubo);
 }

} //namespace opengl_util
