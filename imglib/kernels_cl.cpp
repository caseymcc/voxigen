#include "imglib/kernels_cl.h"

#include <memory>
#include <unordered_map>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cassert>

namespace imglib{namespace cl
{

typedef std::unordered_map<std::string, KernelInfo> KernelMap;
typedef std::unordered_map<cl_context, KernelMap> ContextKernelMap;

typedef std::unordered_map<std::string, cl_program> ProgramMap;
typedef std::unordered_map<cl_context, ProgramMap> ContextProgramMap;

std::vector<cl_device_id> getDevices(cl_context context)
{
    std::vector<cl_device_id> devices;
//    cl_int error;
    size_t paramSize;

    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, nullptr, &paramSize);
    if(paramSize<=0)
    {
        assert(false);
        return devices;
    }

    devices.resize(paramSize/sizeof(cl_device_id));
    clGetContextInfo(context, CL_CONTEXT_DEVICES, paramSize, devices.data(), nullptr);

    return devices;
}

cl_program getProgram(cl_context context, const char *name, const char *source)
{
    static ContextProgramMap contextProgramMap;

    ProgramMap &programs=contextProgramMap[context];
    ProgramMap::iterator iter=programs.find(name);

    if(iter!=programs.end())
        return iter->second;

    cl_int error;

    const char *sourceList[1]={source};
    size_t lengths[1]={strlen(source)};

    cl_program program=clCreateProgramWithSource(context, (cl_uint)1, sourceList, lengths, &error);

    assert(error==CL_SUCCESS);
    if(error!=CL_SUCCESS)
        return 0;

    std::vector<cl_device_id> devices=getDevices(context);

    error=clBuildProgram(program, devices.size(), devices.data(), "", nullptr, nullptr);

    if(error!=CL_SUCCESS)
    {
        if((error==CL_BUILD_PROGRAM_FAILURE) ||(error==CL_INVALID_BINARY))
        {
            std::string errorStr;
            size_t paramSize;

            clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, nullptr, &paramSize);
            errorStr.resize(paramSize);
            clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, errorStr.size(), (void *)errorStr.data(), nullptr);

            std::cout<<std::endl<<" BUILD LOG: "<<name<<std::endl;
            std::cout<<" ************************************************"<<std::endl;
            std::cout<<errorStr;
            std::cout<<" ************************************************"<<std::endl;

            if(error==CL_INVALID_BINARY)
            {
//                std::vector<char *> programBinaries=program.getInfo<CL_PROGRAM_BINARIES>();
                std::string programBinaries;
                
                clGetProgramInfo(program, CL_PROGRAM_BINARIES, 0, nullptr, &paramSize);
                programBinaries.resize(paramSize);
                clGetProgramInfo(program, CL_PROGRAM_BINARIES, programBinaries.size(), (void *)programBinaries.data(), nullptr);

                std::cout<<std::endl<<" BINARY LOG: "<<name<<std::endl;
                std::cout<<" ************************************************"<<std::endl;
                for(size_t i=0; i<programBinaries.size(); ++i)
                    std::cout<<programBinaries[i];
                std::cout<<" ************************************************"<<std::endl;

            }

            assert(false);
            return false;
        }
        assert(false);
        return false;
    }

    programs.insert({name, program});
    return program;
}

cl_kernel getKernel(cl_context context, const char *kernelName, const char *programName, const char *source)
{
    KernelInfo kernelInfo=getKernelInfo(context, kernelName, programName, source);

    return kernelInfo.kernel;
}

KernelInfo getKernelInfo(cl_context context, const char *kernelName, const char *programName, const char *source)
{
    static ContextKernelMap contextKernelMap;
    
    KernelMap &kernels=contextKernelMap[context];
    KernelMap::iterator iter=kernels.find(kernelName);

    if(iter!=kernels.end())
        return iter->second;

    cl_program program=getProgram(context, programName, source);
    KernelInfo kernelInfo;
    std::vector<cl_device_id> devices=getDevices(context);

    cl_int error;
    
    kernelInfo.kernel=clCreateKernel(program, kernelName, &error);
    assert(error==CL_SUCCESS);

    clGetKernelWorkGroupInfo(kernelInfo.kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelInfo.workGroupSize, nullptr);
    clGetKernelWorkGroupInfo(kernelInfo.kernel, devices[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &kernelInfo.preferredWorkGroupMultiple, nullptr);
    clGetKernelWorkGroupInfo(kernelInfo.kernel, devices[0], CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &kernelInfo.localMemoryUsed, nullptr);

    //compying info so it is available when setting up kernel call
//    kernelInfo->deviceComputeUnits=openCLContext->deviceInfo.computeUnits;
//    kernelInfo->deviceLocalMemory=openCLContext->deviceInfo.localMemory;

    kernels.insert({kernelName, kernelInfo});

    return kernelInfo;
}

}}//namespace imglib::cl