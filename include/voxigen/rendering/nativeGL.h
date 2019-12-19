#ifndef _voxigen_nativeGL_h_
#define _voxigen_nativeGL_h_

#include <memory>
#include "voxigen/voxigen_export.h"

namespace voxigen
{

struct NativeStruct;

class VOXIGEN_EXPORT NativeGL
{
public:
    NativeGL();
    ~NativeGL();

    void createSharedContext();
    void makeCurrent();
    void releaseCurrent();

private:
#pragma warning(push)
#pragma warning(disable:4251)
    std::unique_ptr<NativeStruct> hidden;
#pragma warning(pop)
};

}//namespace voxigen

#endif //_voxigen_nativeGL_h_