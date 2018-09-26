
_INLINE_ void atomicAddFloat_Local(volatile __local float *addr, float val)
{
    union
    {
        unsigned int u32;
        float        f32;
    } next, expected, current;
    current.f32=*addr;
    do
    {
        expected.f32=current.f32;
        next.f32=expected.f32+val;
        current.u32=atomic_cmpxchg((volatile __local unsigned int *)addr,
            expected.u32, next.u32);
    } while(current.u32!=expected.u32);
}

_INLINE_ void atomicAddFloat_Global(volatile __global float *addr, float val)
{
    union
    {
        unsigned int u32;
        float        f32;
    } next, expected, current;
    current.f32=*addr;
    do
    {
        expected.f32=current.f32;
        next.f32=expected.f32+val;
        current.u32=atomic_cmpxchg((volatile __global unsigned int *)addr,
            expected.u32, next.u32);
    } while(current.u32!=expected.u32);
}