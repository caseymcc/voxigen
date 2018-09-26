
//converts from rgb to rgba 4 pixels at a time
__kernel void rgbToRGBA_uint(__global uint *srcBuffer, int width, int height, write_only image2d_t dstImage)
{
    const int x=get_global_id(0);
    const int dstX=x*4;
    const int y=get_global_id(1);
    const int pos=(y*(width/4*3))+x*3;

    uint buf1=srcBuffer[pos];
    uint buf2=srcBuffer[pos+1];
    uint buf3=srcBuffer[pos+2];

    write_imageui(dstImage, (int2)(dstX, y), (uint4)((buf1>>24)&0xff, (buf1>>16)&0xff, (buf1>>8)&0xff, 255));
    write_imageui(dstImage, (int2)(dstX+1, y), (uint4)((buf1)&0xff, (buf2>>24)&0xff, (buf2>>16)&0xff, 255));
    write_imageui(dstImage, (int2)(dstX+2, y), (uint4)((buf2>>8)&0xff, (buf2)&0xff, (buf3>>24)&0xff, 255));
    write_imageui(dstImage, (int2)(dstX+3, y), (uint4)((buf3>>16)&0xff, (buf3>>8)&0xff, (buf3)&0xff, 255));
}

__kernel void rgbToRGBA_float(__global float *srcBuffer, int width, int height, write_only image2d_t dstImage)
{
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int pos=(y*width*3)+x*3;

    float4 value=(float4)(srcBuffer[pos], srcBuffer[pos+1], srcBuffer[pos+2], 255);
    write_imagef(dstImage, (int2)(x, y), value);
}

