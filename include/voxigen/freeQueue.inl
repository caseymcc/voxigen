
namespace voxigen
{

template<typename _Type>
FreeQueue<_Type>::FreeQueue():m_maxSize(0), m_growSize(1)
{

}

template<typename _Type>
FreeQueue<_Type>::~FreeQueue()
{
    for(size_t i=0; i<m_allItems.size(); ++i)
    {
        delete m_allItems[i];
    }
}

template<typename _Type>
void FreeQueue<_Type>::setMaxSize(size_t value)
{
    m_maxSize=value;
}

template<typename _Type>
void FreeQueue<_Type>::setGrowSize(size_t value)
{
    m_growSize=value;
}

template<typename _Type>
typename FreeQueue<_Type>::Type *FreeQueue<_Type>::get()
{
    if(m_freeItems.empty())
    {
        if((m_maxSize == 0) || (m_allItems.size()<m_maxSize))
        {
            size_t growSize=std::min(m_growSize, m_maxSize-m_allItems.size());
            
            for(size_t i=0; i<growSize; ++i)
            {
                Type *item=new Type();

                m_allItems.push_back(item);
                m_freeItems.push_back(item);
            }
        }

        if(m_freeItems.empty())
            return nullptr;
    }

    Type *item=m_freeItems.back();

    m_freeItems.pop_back();
    return item;
}

template<typename _Type>
template<typename _Functor>
typename FreeQueue<_Type>::Type *FreeQueue<_Type>::get(const _Functor &functor)
{
    if(m_freeItems.empty())
    {
        if((m_maxSize==0)||(m_allItems.size()<m_maxSize))
        {
            for(size_t i=0; i<m_growSize; ++i)
            {
                Type *item=new Type();

                functor(item);
                m_allItems.push_back(item);
                m_freeItems.push_back(item);
            }
        }

        if(m_freeItems.empty())
            return nullptr;
    }

    Type *item=m_freeItems.back();

    m_freeItems.pop_back();
    return item;
}

template<typename _Type>
void FreeQueue<_Type>::release(Type *item)
{
    m_freeItems.push_back(item);
}

template<typename _Type>
template<typename _Functor>
void FreeQueue<_Type>::release(Type *item, _Functor &functor)
{
    functor(item);
    m_freeItems.push_back(item);
}

}//namespace voxigen