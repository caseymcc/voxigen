__kernel void zeroMemory(__global uchar *buffer)
{
    const int globalX=get_global_id(0);

    buffer[globalX]=0;
}

__kernel void zeroFloatMemory(__global float *buffer)
{
    const int globalX=get_global_id(0);

    buffer[globalX]=0.0f;
}

__kernel void zeroIntMemory(__global int *buffer)
{
    const int globalX=get_global_id(0);

    buffer[globalX]=0.0f;
}
