#ifndef _generic_objectHeap_h_
#define _generic_objectHeap_h_

#include <vector>

namespace generic
{

template<typename _Object>
class ObjectHeap
{
public:
    ObjectHeap(size_t maxSize=64);

    _Object *get();
    void release(_Object *object);

private:
    std::vector<_Object> m_objects;
    std::vector<_Object *> m_freeObjects;
};


template<typename _Object>
ObjectHeap<_Object>::ObjectHeap(size_t maxSize)
{
    m_objects.resize(maxSize);
    m_freeObjects.resize(maxSize);

    for(size_t i=0; i<m_objects.size(); ++i)
        m_freeObjects[i]=&(m_objects[i]);
}

template<typename _Object>
_Object *ObjectHeap<_Object>::get()
{
    if(m_freeObjects.empty())
        return nullptr;

    _Object *object=m_freeObjects.back();

    m_freeObjects.pop_back();
    return object;
}

template<typename _Object>
void ObjectHeap<_Object>::release(_Object *object)
{
    m_freeObjects.push_back(object);
}

}//namespace generic

#endif//_generic_objectHeap_h_