#ifndef _voxigen_generator_h_
#define _voxigen_generator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/classFactory.h"

namespace voxigen
{

class Generator
{
public:
    Generator() {}
    virtual ~Generator() {}

    virtual void initialize(GridDescriptors *descriptors)=0;
//    virtual void terminate()=0;

    virtual void generateChunk(unsigned int hash, void *buffer, size_t size)=0;
};
typedef std::shared_ptr<Generator> SharedGenerator;

template<typename _Generator>
class GeneratorTemplate:RegisterClass<GeneratorTemplate<_Generator>, Generator>
{
public:
    GeneratorTemplate():m_generator(new _Generator()) {}
    virtual ~GeneratorTemplate() {}

    virtual void initialize(GridDescriptors *descriptors) { m_generator->initialize(descriptors); }
//    virtual void terminate() { m_generator->terminate(); }

    virtual void generateChunk(unsigned int hash, void *buffer, size_t size) { m_generator->generateChunk(hash, buffer, size); };

private:
    std::unique_ptr<_Generator> m_generator;
};

}//namespace voxigen

#endif //_voxigen_generator_h_