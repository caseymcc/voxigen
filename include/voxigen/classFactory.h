#ifndef _voxigen_classFactory_h_
#define _voxigen_classFactory_h_

#include <unordered_map>
#include <memory>

namespace voxigen
{

template <typename _Type> struct TypeName
{
    static std::string get()
    {
        const char *fullName=typeid(_Type).name();
        const char *name=strstr(fullName, "class");
        return (name)? name+6 : fullName;
    }
};

template<typename _Class, typename _BaseClass>
class RegisterClass:public _BaseClass
{
public:
    RegisterClass():_BaseClass(){ &s_typeName; }

    static _BaseClass *create() { return dynamic_cast<_BaseClass *>(new _Class()); }

private:
    static std::string s_typeName;
};


template<typename _BaseClass>
class ClassFactory
{
public:
    typedef _BaseClass *(*CreateFunction)();
    typedef std::unordered_map<std::string, CreateFunction> CreateFunctions;

    static ClassFactory<_BaseClass> &instance() { if(s_instance==nullptr) s_instance=new ClassFactory<_BaseClass>(); return *s_instance; }
    
    std::unique_ptr<_BaseClass> create(std::string typeName)
    {
        typename CreateFunctions::iterator iter=m_functionMap.find(typeName);

        std::unique_ptr<_BaseClass> classInstance;

        if(iter!=m_functionMap.end())
            classInstance.reset(iter->second());
        return classInstance;
    }

    std::vector<std::string> names()
    {
        std::vector<std::string> names;

        for(typename CreateFunctions::iterator iter=m_functionMap.begin(); iter!=m_functionMap.end(); ++iter)
            names.push_back(iter->first);
        return names;
    }

    std::string registerClass(std::string typeName, CreateFunction createFunction)
    {
        m_functionMap[typeName]=createFunction;
        return typeName;
    }

private:
    ClassFactory() {};

    static ClassFactory<_BaseClass> *s_instance;
    CreateFunctions m_functionMap;
};

template<typename _BaseClass>
ClassFactory<_BaseClass> *ClassFactory<_BaseClass>::s_instance=nullptr;

template<typename _Class, typename _BaseClass> std::string RegisterClass<_Class, _BaseClass>::s_typeName=\
ClassFactory<_BaseClass>::instance().registerClass(_Class::typeName(), &RegisterClass<_Class, _BaseClass>::create);

template<typename _BaseClass> std::unique_ptr<_BaseClass> createClass(std::string className)
{
    return ClassFactory<_BaseClass>::instance().create(className);
}

}//namespace voxigen

#endif //_voxigen_classFactory_h_