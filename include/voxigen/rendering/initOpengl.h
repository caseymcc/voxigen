#ifndef _voxigen_initOpenGL_h_
#define _voxigen_initOpenGL_h_

#include <voxigen/voxigen_export.h>
#include <glbinding/glbinding.h>

namespace voxigen
{

VOXIGEN_EXPORT void initOpenGL(glbinding::GetProcAddress functionPointerResolver);

} //namespace voxigen

#endif //_voxigen_initOpenGL_h_

