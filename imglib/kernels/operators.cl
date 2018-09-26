//_INLINE_ void atomicAddFloat_Local(volatile __local float *addr, float val)
//{
//    union
//    {
//        unsigned int u32;
//        float        f32;
//    } next, expected, current;
//    current.f32=*addr;
//    do
//    {
//        expected.f32=current.f32;
//        next.f32=expected.f32+val;
//        current.u32=atomic_cmpxchg((volatile __local unsigned int *)addr,
//            expected.u32, next.u32);
//    } while(current.u32!=expected.u32);
//}
//
//_INLINE_ void atomicAddFloat_Global(volatile __global float *addr, float val)
//{
//    union
//    {
//        unsigned int u32;
//        float        f32;
//    } next, expected, current;
//    current.f32=*addr;
//    do
//    {
//        expected.f32=current.f32;
//        next.f32=expected.f32+val;
//        current.u32=atomic_cmpxchg((volatile __global unsigned int *)addr,
//            expected.u32, next.u32);
//    } while(current.u32!=expected.u32);
//}
#include "atomic.cl"

__kernel void sum(read_only image2d_t input, int width, int height, __constant float *sumBuffer)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int y=get_global_id(0);
    const int localId=get_local_id(0);
    __local float localSum[64];

    if(y==0)
        (*sumBuffer)=0;

    if(y<height)
    {
        float sum=0;
        int2 pos=(int2)(0, y);

        for(int x=0; x<width, x++)
        {
            pos.x=x;
            sum+=read_imagef(input, nearestClampSampler, pos).x;
        }

        localSum[localId]=sum;
    }

    //sumBuffer(global) set by y==0, localSum(local) set by everything else
    barrier(CLK_LOCAL_MEM_FENCE|CLK_GLOBAL_MEM_FENCE);

    if(localId!=0)
        return;

    const int groupId=get_group_id(0);
    int count=64;
    const int index=groupId*count;

    if(index+64>height)
        count=height-index;

    sum=localSum[0];
    for(int i=1; i<64, i++)
        sum+=localSum[i];

    atomicAddFloat_Global(sumBuffer, sum);
}
