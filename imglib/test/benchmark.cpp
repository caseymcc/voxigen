#include "imglib/simpleImage.h"
#include "imglib/png.h"

#ifdef IMGLIB_OPENCL
#include "imglib/operators_cl.h"
#include "imglib/transform_cl.h"

#include "options.h"
#include "benchmarkOpenCL.h"
#endif //IMGLIB_OPENCL

#include "opencl_util/device.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#include <gflags/gflags.h>

DEFINE_bool(disable_cpu, false, "Disables test using cpu");
DEFINE_bool(devices, false, "List all available processing devices");
#ifdef IMGLIB_OPENCL
DEFINE_bool(opencl, true, "Benchmarks opencl based functions");
DEFINE_string(opencl_platform, "any", "OpenCL platform to use for benchmarking");
DEFINE_string(opencl_device, "any", "OpenCL device to use for benchmarking");
#endif //IMGLIB_OPENCL
#ifdef IMGLIB_CUDA
DEFINE_bool(cuda, false, "Benchmarks cuda based functions");
#endif //IMGLIB_CUDA
DEFINE_int32(interations, 100, "Number of runs to average results over");

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement=0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance=1;
}

void benchmarkCPU(imglib::SimpleImage &image, Options &options)
{

}


void benchmark(char *filename)
{
    Options options;
    imglib::SimpleImage image;

    if(!imglib::loadPng(image, filename))
    {
        std::cout<<"Failed to open png image: "<<filename<<std::endl;
        return;
    }

    options.interations=FLAGS_interations;

    if(!FLAGS_disable_cpu)
        benchmarkCPU(image, options);

#ifdef IMGLIB_OPENCL
    options.device=FLAGS_opencl_device;
    options.platform=FLAGS_opencl_platform;

    if(FLAGS_opencl)
        benchmarkOpenCL(image, options);
#endif//IMGLIB_OPENCL
}

void listDevices()
{
#ifdef IMGLIB_OPENCL
    std::vector<std::string> deviceNames=cl::util::getDevices();

    if(!deviceNames.empty())
    {
        std::cout<<"OpenCL devices:"<<std::endl;

        for(std::string &deviceName:deviceNames)
        {
            cl::util::DeviceInfo deviceInfo;

            cl::util::getDeviceInfo(deviceName, deviceInfo);
            std::cout<<"  Device: "<<deviceInfo.name()<<" ("<<deviceInfo.platform()<<", "<<deviceInfo.vendor()<<")"<<std::endl;
            std::cout<<"    Type: "<<((deviceInfo.type()==cl::util::Type::GPU)?"GPU":"CPU")<<std::endl;
            std::cout<<"    Version: "<<deviceInfo.version()<<std::endl;

            cl::util::vector<size_t> maxWorkItemSizes=deviceInfo.maxWorkItemSizes();
            

            cl::util::vector<size_t>::iterator iter=maxWorkItemSizes.begin();
//            std::vector<size_t> maxWorkItemSizes=deviceInfo.maxWorkItemSizes();

            std::cout<<"    Max Work Item Size: test iter";
            for(;iter!=maxWorkItemSizes.end(); iter++)
            {
                std::cout<<*iter<<", ";
            }
            std::cout<<std::endl;

            std::cout<<"    Max Work Item Size: ";
            for(size_t i=0; i<maxWorkItemSizes.size(); ++i)
                std::cout<<maxWorkItemSizes[i]<<", ";
            std::cout<<std::endl;
        }
        std::cout<<std::endl;
    }
#endif //IMGLIB_OPENCL
#ifdef IMGLIB_CUDA
    std::vector<libAKAZE::cuda::CudaDevice> cudaDevices=libAKAZE::cuda::getDevices();

    if(!cudaDevices.empty())
    {
        std::cout<<"CUDA devices:"<<std::endl;

        for(libAKAZE::cuda::CudaDevice &cudaDevice:cudaDevices)
        {
            std::cout<<"  Device: "<<cudaDevice.name<<std::endl;
        }
    }
#endif //IMGLIB_CUDA
}

int main(int argc, char **argv)
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if(FLAGS_devices)
        listDevices();
    else
    {
        for(int i=1; i<argc; ++i)
        {
            benchmark(argv[i]);
        }
    }

#if(defined(_WINDOWS))
    system("pause");
#endif
}
