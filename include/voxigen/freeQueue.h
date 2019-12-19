#ifndef _voxigen_freeQueue_h_
#define _voxigen_freeQueue_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/volume/regularGrid.h"
#include "voxigen/search.h"
#include "voxigen/texturing/textureAtlas.h"

#include <string>
#include <deque>

#ifdef _WINDOWS
#include "windows.h"
#endif

namespace voxigen
{

/////////////////////////////////////////////////////////////////////////////////////////
//FreeQueue
/////////////////////////////////////////////////////////////////////////////////////////
template<typename _Type>
class FreeQueue
{
public:
    typedef _Type Type;
    typedef std::vector<Type *> TypeVector;

    FreeQueue();
    ~FreeQueue();

    void setMaxSize(size_t value);
    void setGrowSize(size_t value);

    Type *get();
    template<typename _Functor>
    Type *get(const _Functor &functor);
    void release(_Type *item);
    template<typename _Functor>
    void release(_Type *item, _Functor &functor);

private:
    TypeVector m_allItems;
    TypeVector m_freeItems;

    size_t m_growSize;
    size_t m_maxSize;
};

}//namespace voxigen

#include "freeQueue.inl"

#endif //_voxigen_freeQueue_h_