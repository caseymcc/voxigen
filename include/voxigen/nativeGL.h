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

private:
    std::unique_ptr<NativeStruct> hidden;
};

}//namespace voxigen

#endif //_voxigen_nativeGL_h_