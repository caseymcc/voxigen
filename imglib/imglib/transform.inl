#ifdef IMGLIB_OPENCL
#include "imglib/context_cl.h"
#include "imglib/transform_cl.h"
#endif //IMGLIB_OPENCL

namespace imglib
{

template<typename _Image>
void reduceSimple(_Image &srcImage, _Image &dstImage)
{
#ifdef IMGLIB_OPENCL
    if((location(srcImage)==Location::OpenCL)||(location(dstImage)==Location::OpenCL))
    {
        if(location(srcImage)==Location::System)
            assert(false); //copy to GPU
        else if(location(dstImage)==Location::System)
            assert(false); //copy from GPU
        else
        {
            ::cl::Event event;

            cl::reduceSimple(cl::getCurrentContext(), cl::getCurrentQueue(), openCLImage(srcImage), openCLImage(dstImage), nullptr, &event);
            event.wait();
        }

        return;
    }
#endif //IMGLIB_OPENCL

    assert(location(srcImage)==Location::System);
    assert(location(dstImage)==Location::System);

    reduceSimple(data(srcImage), size(srcImage), format(srcImage), width(srcImage), height(srcImage), data(dstImage), size(dstImage), format(dstImage), width(dstImage), height(dstImage));
}

template<typename _Image>
void expandSimple(_Image &srcImage, _Image &dstImage)
{
#ifdef IMGLIB_OPENCL
    if((location(srcImage)==Location::OpenCL)||(location(dstImage)==Location::OpenCL))
    {
        if(location(srcImage)==Location::System)
            assert(false); //copy to GPU
        else if(location(dstImage)==Location::System)
            assert(false); //copy from GPU
        else
        {
            ::cl::Event event;

            cl::expandSimple(, , openCLImage(srcImage), openCLImage(dstImage), nullptr, &event);
            event.wait();
        }

        return;
    }
#endif //IMGLIB_OPENCL

    assert(location(srcImage)==Location::System);
    assert(location(dstImage)==Location::System);

    expandSimple(data(srcImage), size(srcImage), format(srcImage), width(srcImage), height(srcImage), data(dstImage), size(dstImage), format(dstImage), width(dstImage), height(dstImage));
}

template<typename _Image>
void reduce(_Image &srcImage, _Image &dstImage)
{
    assert(location(srcImage)==Location::System);
    assert(location(dstImage)==Location::System);

    reduce(data(srcImage), size(srcImage), format(srcImage), width(srcImage), height(srcImage), data(dstImage), size(dstImage), format(dstImage), width(dstImage), height(dstImage));
}

template<typename _Image>
void expand(_Image &srcImage, _Image &dstImage)
{
    assert(location(srcImage)==Location::System);
    assert(location(dstImage)==Location::System);

    expand(data(srcImage), size(srcImage), format(srcImage), width(srcImage), height(srcImage), data(dstImage), size(dstImage), format(dstImage), width(dstImage), height(dstImage));
}

}//namespace imglib
