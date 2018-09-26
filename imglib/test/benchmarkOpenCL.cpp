#ifdef IMGLIB_OPENCL

#include "imglib/simpleImage.h"
#include "imglib/transform_cl.h"
#include "imglib/openCLImage.h"
#include "imglib/openCLImageBuffer.h"
#include "imglib/colorConversion_cl.h"

#include "benchmarkOpenCL.h"

#include "opencl_util/device.h"

#include <iostream>
#include <sstream>

void openClNotify(const char *errinfo, const void *private_info, ::size_t cb, void *user_data)
{
    std::cout<<"OpenCL info: "<<errinfo<<std::endl;
}

void benchmarkOpenCL(imglib::SimpleImage &image, Options &options)
{
    cl::util::DeviceInfo deviceInfo;
    ::cl::Context openClContext;

    openClContext=::cl::Context(cl::util::openDevice(options.device, deviceInfo, &openClNotify));
//    cl_context context=cl::util::openDevice(options.device, deviceInfo);
//    ::cl::Context openClContext(context);

    if(openClContext()==nullptr)
        return;

    std::cout<<"Device: "<<deviceInfo.name()<<" ("<<deviceInfo.platform()<<", "<<deviceInfo.vendor()<<")"<<std::endl;
    std::cout<<"  Type: "<<((deviceInfo.type()==cl::util::Type::GPU)?"GPU":"CPU")<<std::endl;
    std::cout<<"  Version: "<<deviceInfo.version()<<std::endl;

//    ::cl::CommandQueue commandQueue(openClContext);
    ::cl::CommandQueue commandQueue(openClContext);

    imglib::OpenCLImageBuffer rgbImage(openClContext, commandQueue, image, true);
    imglib::OpenCLImage srcImage(openClContext, commandQueue, image);
    imglib::OpenCLImage dstImage(openClContext, commandQueue, image);

    imglib::cl::convert(openClContext, commandQueue, rgbImage, srcImage);
    
    //test
    imglib::cl::reduce(openClContext, commandQueue, srcImage.clImage(), srcImage.clImage());

}
#endif //IMGLIB_OPENCL

