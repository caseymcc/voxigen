#ifndef _voxigen_gltext_
#define _voxigen_gltext_

#include <glbinding/gl/gl.h>
using namespace gl;

//put gltext in our namespace
namespace voxigen
{

#ifndef __gl_h_
#define __gl_h_
#define voxigen_undef_gl_h_
#endif

#ifndef __glcorearb_h_
#define __glcorearb_h_
#define voxigen_undef_glcorearb_h_
#endif

#include "voxigen/gltext.h"

#ifdef voxigen_undef_gl_h_
#undef __gl_h_
#endif

#ifdef voxigen_undef_glcorearb_h_
#undef __glcorearb_h_
#endif
}
#endif//_voxigen_gltext_