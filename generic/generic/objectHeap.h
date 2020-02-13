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

    void setMaxSize(size_t maxSize);
    size_t getMaxSize() { return m_objects.size(); }

    _Object *get();
    void release(_Object *object);

    bool exists(_Object *object);

private:
    std::vector<_Object> m_objects;
    std::vector<_Object *> m_freeObjects;
};


template<typename _Object>
ObjectHeap<_Object>::ObjectHeap(size_t maxSize)
{
    setMaxSize(maxSize);
}

template<typename _Object>
void ObjectHeap<_Object>::setMaxSize(size_t maxSize)
{
    if(m_objects.empty())
    {
        m_objects.resize(maxSize);
        m_freeObjects.resize(maxSize);

        for(size_t i=0; i<m_objects.size(); ++i)
            m_freeObjects[i]=&(m_objects[i]);
    }
    else
    {
        //cant change on the fly, would have to make sure all the requests are back
        assert(false);


        if(maxSize>m_objects.size())
        { 
            //enlarging so just add more
            size_t currentSize=m_objects.size();
            size_t freeIndex=m_freeObjects.size();

            m_objects.resize(maxSize);
            m_freeObjects.resize(maxSize);
            
            for(size_t i=currentSize; i<maxSize; ++i)
            {
                m_freeObjects[freeIndex]=&(m_objects[i]);
                freeIndex++;
            }
        }
        else
        {
            //need to implement this
            assert(false);
        }

    }
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

template<typename _Object>
bool ObjectHeap<_Object>::exists(_Object *object)
{
    return std::find(m_freeObjects.begin(), m_freeObjects.end(), object)!=m_freeObjects.end();
}

}//namespace generic

#endif//_generic_objectHeap_h_