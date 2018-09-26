#include "convolve.h"

namespace imglib
{

template<typename _SourceImageType, typename _DestinationImageType>
void convolve(_SourceImageType &src, _DestinationImageType &dst, Kernel &kenel, float scale=1.0)
{
    Location srcLocation=location(src);
    Location dstLocation=location(dst);

    //can't handle src and dst in different locations
    assert(srcLocation == dstLocation);

    if(srcLocation == Location::System)
        convolve();
    else if(srcLocation == Location::System)
        cl::convolve(src, dst, )
    else
        assert(false) //needs implementation
}

///convolve image with SeperableKernel, border is replicated
template<typename _SourceImageType, typename _DestinationImageType>
void separableConvolve(_SourceImageType &src, _DestinationImageType &dst, SeparableKernel &kernel, float scale=1.0)
{
    assert(false) //needs implementation
}

}//namespace imglib
