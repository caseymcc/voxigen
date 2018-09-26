#include "memory.cl"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//reduce
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//reduceBufferSimple/reduceImageSimple and expandBufferSimple/expandImageSimple
//   |   |   |   
//---------------
//   | 1 | 2 |   
//-------O-------
//   | 3 | 4 |   
//---------------
//   |   |   |   
//grid of the 4 pixels used to calculate O

__kernel void reduceBufferSimple_uint(__global uint *input, int inputWidth, int inputHeight, __global uint *output, int outputWidth, int outputHeight)
{
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int srcX=2*x;
    const int srcY=2*y*inputWidth;

    uint value1=input[srcY+srcX];
    uint value2=input[srcY+srcX+1];
    uint value3=input[srcY+inputWidth+srcX];
    uint value4=input[srcY+inputWidth+srcX+1];

    uint value=(value1+value2+value3+value4)/4;
    output[(y*outputWidth)+x]=value;
}

__kernel void reduceBufferSimple_uint3(__global uint *input, int inputWidth, int inputHeight, __global uint *output, int outputWidth, int outputHeight)
{
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int stride=inputWidth*3;
    const int srcX=2*x*3;
    const int srcY=2*y*stride;

    uint3 value1=(uint3)(input[srcY+srcX], input[srcY+srcX+1], input[srcY+srcX+2]);
    uint3 value2=(uint3)(input[srcY+srcX+3], input[srcY+srcX+4], input[srcY+srcX+5]);
    uint3 value3=(uint3)(input[srcY+stride+srcX], input[srcY+stride+srcX+1], input[srcY+stride+srcX+2]);
    uint3 value4=(uint3)(input[srcY+stride+srcX+3], input[srcY+stride+srcX+4], input[srcY+stride+srcX+5]);

    uint3 value=(value1+value2+value3+value4)/4;
    
    output[(y*outputWidth)+x]=value.x;
    output[(y*outputWidth)+x+1]=value.y;
    output[(y*outputWidth)+x+2]=value.z;
}

__kernel void reduceImageSimple_uint(read_only image2d_t input, int width, int height, write_only image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int srcX=2*x;
    const int srcY=2*y;

    uint value1=read_imageui(input, nearestClampSampler, (int2)(srcX, srcY)).x;
    uint value2=read_imageui(input, nearestClampSampler, (int2)(srcX+1, srcY)).x;
    uint value3=read_imageui(input, nearestClampSampler, (int2)(srcX, srcY+1)).x;
    uint value4=read_imageui(input, nearestClampSampler, (int2)(srcX+1, srcY+1)).x;

    uint value=(value1+value2+value3+value4)/4;
    write_imageui(output, (int2)(x, y), value);
}

__kernel void reduceImageSimple_uint4(read_only image2d_t input, int width, int height, write_only image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int srcX=2*x;
    const int srcY=2*y;

    uint4 value1=read_imageui(input, nearestClampSampler, (int2)(srcX, srcY));
    uint4 value2=read_imageui(input, nearestClampSampler, (int2)(srcX+1, srcY));
    uint4 value3=read_imageui(input, nearestClampSampler, (int2)(srcX, srcY+1));
    uint4 value4=read_imageui(input, nearestClampSampler, (int2)(srcX+1, srcY+1));

    uint4 value=(value1+value2+value3+value4)/4;
    write_imageui(output, (int2)(x, y), value);
}

__kernel void reduceImageSimple_float(read_only image2d_t input, int width, int height, write_only image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int srcX=2*x;
    const int srcY=2*y;

    float value1=read_imagef(input, nearestClampSampler, (int2)(srcX, srcY)).x;
    float value2=read_imagef(input, nearestClampSampler, (int2)(srcX+1, srcY)).x;
    float value3=read_imagef(input, nearestClampSampler, (int2)(srcX, srcY+1)).x;
    float value4=read_imagef(input, nearestClampSampler, (int2)(srcX+1, srcY+1)).x;

    float value=(value1+value2+value3+value4)/4;
    write_imagef(output, (int2)(x, y), value);
}

__kernel void reduceImageSimple_float4(read_only image2d_t input, int width, int height, write_only image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0);
    const int y=get_global_id(1);

    const int srcX=2*x;
    const int srcY=2*y;

    float4 value1=read_imagef(input, nearestClampSampler, (int2)(srcX, srcY));
    float4 value2=read_imagef(input, nearestClampSampler, (int2)(srcX+1, srcY));
    float4 value3=read_imagef(input, nearestClampSampler, (int2)(srcX, srcY+1));
    float4 value4=read_imagef(input, nearestClampSampler, (int2)(srcX+1, srcY+1));

    float4 value=(value1+value2+value3+value4)/4;
    write_imagef(output, (int2)(x, y), value);
}


//reduceBuffer_xxx/reduceImage_xxx  and expandBuffer_xxx/expandImage_xxx 
//This code uses a 16 pixel (4x4) bilinear interpolated reduce function, the reduce could have been a simple areverage of the 4 pixels (see reduceImageSimple)
//below the reduced pixel 6-O (6, 7, 10, 11), but this way it explicitly encodes data that is used in the expand function into the reduced
//pixels. For instance if you wanted to expand and recreate pixel 11, your only source of information is the reduced pixels 6-O, 
//8-O, 14-O and 16-O, if the reduce function only used the 4 neighbor pixels for 6-O (6, 7, 10, 11) then pixel 11's information is only 
//contained in 6-O. However using the 16 pixel reduce, information about 11 is in 6-O, 8-O, 14-O and 16-O which should better approximate 11
//during expand. Obviously the expand will not match the original image before the reduce but this a least uses the pixels that will be 
//used to calculate the original pixel from the reduced pixels.

//   |   |   |   |   |   |   |
//---O-------O-------O-------O---
//   | 1 | 2 | 3 | 4 |   |   |
//-------------------------------
//   | 5 | 6 | 7 | 8 |   |   |
//---O-------O-------O-------O---
//   | 9 | 10| 11| 12|   |   |
//-------------------------------
//   | 13| 14| 15| 16|   |   |
//---O-------O-------O-------O---
//   |   |   |   |   |   |   |
//grid of the 16 pixels used to calculate 6-O

uint reduceLocal_uint(__local uint *input, int x, int y, int stride)
{
    const int y_1=y-stride;
    const int y1=y+stride;
    const int y2=y1+stride;

    float value00=(1.0f/64.0f)*input[x-1+y_1];
    float value01=(3.0f/64.0f)*input[x-1+y];
    float value02=(3.0f/64.0f)*input[x-1+y1];
    float value03=(1.0f/64.0f)*input[x-1+y2];
    float value10=(3.0f/64.0f)*input[x+y_1];
    float value11=(9.0f/64.0f)*input[x+y];
    float value12=(9.0f/64.0f)*input[x+y1];
    float value13=(3.0f/64.0f)*input[x+y2];
    float value20=(3.0f/64.0f)*input[x+1+y_1];
    float value21=(9.0f/64.0f)*input[x+1+y];
    float value22=(9.0f/64.0f)*input[x+1+y1];
    float value23=(3.0f/64.0f)*input[x+1+y2];
    float value30=(1.0f/64.0f)*input[x+2+y_1];
    float value31=(3.0f/64.0f)*input[x+2+y];
    float value32=(3.0f/64.0f)*input[x+2+y1];
    float value33=(1.0f/64.0f)*input[x+2+y2];

    uint value=(value00+value01+value02+value03+
        value10+value11+value12+value13+
        value20+value21+value22+value23+
        value30+value31+value32+value33);

    return value;
}

__kernel void reduceBuffer_uint(__global uint *input, int width, int height, __global uint *output, __local uint *localCache)
{
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);

    int cacheX;
    int cacheY;

    copyBufferToLocal_ui(input, width, height, 2, localX, localY, localCache, &cacheX, &cacheY);
    uint value=reduceLocal_uint(localCache, localX, localY, cacheX);
    output[y*width+x]=value;
}

__kernel void reduceImage_unit(read_only image2d_t input, int width, int height, write_only image2d_t output, __local uint *localCache)
{
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);

    int cacheX;
    int cacheY;

    copyImageToLocal_ui(input, width, height, 2, localX, localY, localCache, &cacheX, &cacheY);
    uint value=reduceLocal_uint(localCache, localX, localY, cacheX);

    write_imageui(output, (int2)(x, y), value);
}

uint4 reduceLocal_uint4_skipAlpha(__local uint *input, int x, int y, int stride)
{
    const int srcX=2*x*3;
    const int srcY=2*y*stride;

    const int srcY_1=srcY-stride;
    const int srcY1=srcY+stride;
    const int srcY2=srcY1+stride;

    float3 value00=(1.0f/64.0f)*(float3)(input[srcX-3+srcY_1], input[srcX-2+srcY_1], input[srcX-1+srcY_1]);
    float3 value01=(3.0f/64.0f)*(float3)(input[srcX-3+srcY], input[srcX-2+srcY], input[srcX-1+srcY]);
    float3 value02=(3.0f/64.0f)*(float3)(input[srcX-3+srcY1], input[srcX-2+srcY1], input[srcX-1+srcY1]);
    float3 value03=(1.0f/64.0f)*(float3)(input[srcX-3+srcY2], input[srcX-2+srcY2], input[srcX-1+srcY2]);
    float3 value10=(3.0f/64.0f)*(float3)(input[srcX+srcY_1], input[srcX+1+srcY_1], input[srcX+2+srcY_1]);
    float3 value11=(9.0f/64.0f)*(float3)(input[srcX+srcY], input[srcX+1+srcY], input[srcX+2+srcY]);
    float3 value12=(9.0f/64.0f)*(float3)(input[srcX+srcY1], input[srcX+1+srcY1], input[srcX+2+srcY1]);
    float3 value13=(3.0f/64.0f)*(float3)(input[srcX+srcY2], input[srcX+1+srcY2], input[srcX+2+srcY2]);
    float3 value20=(3.0f/64.0f)*(float3)(input[srcX+3+srcY_1], input[srcX+4+srcY_1], input[srcX+5+srcY_1]);
    float3 value21=(9.0f/64.0f)*(float3)(input[srcX+3+srcY], input[srcX+4+srcY], input[srcX+5+srcY]);
    float3 value22=(9.0f/64.0f)*(float3)(input[srcX+3+srcY1], input[srcX+4+srcY1], input[srcX+5+srcY1]);
    float3 value23=(3.0f/64.0f)*(float3)(input[srcX+3+srcY2], input[srcX+4+srcY2], input[srcX+5+srcY2]);
    float3 value30=(1.0f/64.0f)*(float3)(input[srcX+6+srcY_1], input[srcX+7+srcY_1], input[srcX+8+srcY_1]);
    float3 value31=(3.0f/64.0f)*(float3)(input[srcX+6+srcY], input[srcX+7+srcY], input[srcX+8+srcY]);
    float3 value32=(3.0f/64.0f)*(float3)(input[srcX+6+srcY1], input[srcX+7+srcY1], input[srcX+8+srcY1]);
    float3 value33=(1.0f/64.0f)*(float3)(input[srcX+6+srcY2], input[srcX+7+srcY2], input[srcX+8+srcY2]);

    uint4 value=(uint4)(convert_uint3(value00+value01+value02+value03+
        value10+value11+value12+value13+
        value20+value21+value22+value23+
        value30+value31+value32+value33), 255);

    return value;
}

//__kernel void reduceBuffer_uint4(__global uint *input, __global uint * output, __local uint *localCache)
//{
//    const int x=get_global_id(0)*4;
//    const int y=get_global_id(1);
//    const int localX=get_local_id(0);
//    const int localY=get_local_id(1);
//    int cacheX;
//    int cacheY;
//
//    copyBufferToLocal_uint4(input, width, height, 2, localX, localY, localCache, &cacheX, &cacheY);
//    uint4 value=reduceLocal_uint4(input, localX, localY, cacheX);
//
//    int stride=width*4;
//    output[x+y]=value.x;
//    output[x+y+1]=value.y;
//    output[x+y+2]=value.z;
//    output[x+y+3]=value.w;
//}

__kernel void reduceImage_uint4(read_only image2d_t input, int width, int height, write_only image2d_t output, __local uint *localCache)
{
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);
    int cacheX;
    int cacheY;

    copyImageToLocal_uint4(input, width, height, 2, localX, localY, localCache, &cacheX, &cacheY);
//    uint4 value=reduceLocal_uint4(input, localX, localY, cacheX);

//    write_imageui(output, (int2)(x, y), uint4(value, 255));

    uint4 value=reduceLocal_uint4_skipAlpha(localCache, localX, localY, cacheX);

    write_imageui(output, (int2)(x, y), value);
}

float reduceLocal_float(__local float *input, int x, int y, int stride)
{
    const int srcX=2*x;
    const int srcY=2*y*stride;

    const int srcY_1=srcY-stride;
    const int srcY1=srcY+stride;
    const int srcY2=srcY1+stride;

    float value00=(1.0f/64.0f)*input[srcX-1+srcY_1];
    float value01=(3.0f/64.0f)*input[srcX-1+srcY];
    float value02=(3.0f/64.0f)*input[srcX-1+srcY1];
    float value03=(1.0f/64.0f)*input[srcX-1+srcY2];
    float value10=(3.0f/64.0f)*input[srcX+srcY_1];
    float value11=(9.0f/64.0f)*input[srcX+srcY];
    float value12=(9.0f/64.0f)*input[srcX+srcY1];
    float value13=(3.0f/64.0f)*input[srcX+srcY2];
    float value20=(3.0f/64.0f)*input[srcX+1+srcY_1];
    float value21=(9.0f/64.0f)*input[srcX+1+srcY];
    float value22=(9.0f/64.0f)*input[srcX+1+srcY1];
    float value23=(3.0f/64.0f)*input[srcX+1+srcY2];
    float value30=(1.0f/64.0f)*input[srcX+2+srcY_1];
    float value31=(3.0f/64.0f)*input[srcX+2+srcY];
    float value32=(3.0f/64.0f)*input[srcX+2+srcY1];
    float value33=(1.0f/64.0f)*input[srcX+2+srcY2];

    float value=value00+value01+value02+value03+
        value10+value11+value12+value13+
        value20+value21+value22+value23+
        value30+value31+value32+value33;

    return value;
}

__kernel void reduceImage_float(read_only image2d_t input, int width, int height, write_only  image2d_t output, __local float *localCache)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);

    int cacheX;
    int cacheY;

    copyImageToLocal_float(input, width, height, 2, localX, localY, localCache, &cacheX, &cacheY);
    float value=reduceLocal_float(localCache, localX, localY, cacheX);

    write_imagef(output, (int2)(x, y), value);
}

float4 reduceLocal_float4_skipAlpha(__local float *input, int x, int y, int stride)
{
    const int srcX=2*x*4;
    const int srcY=2*y*stride;

    const int srcY_1=srcY-stride;
    const int srcY1=srcY+stride;
    const int srcY2=srcY1+stride;

    float3 value00=(1.0f/64.0f)*(float3)(input[srcX-3+srcY_1], input[srcX-2+srcY_1], input[srcX-1+srcY_1]);
    float3 value01=(3.0f/64.0f)*(float3)(input[srcX-3+srcY], input[srcX-2+srcY], input[srcX-1+srcY]);
    float3 value02=(3.0f/64.0f)*(float3)(input[srcX-3+srcY1], input[srcX-2+srcY1], input[srcX-1+srcY1]);
    float3 value03=(1.0f/64.0f)*(float3)(input[srcX-3+srcY2], input[srcX-2+srcY2], input[srcX-1+srcY2]);
    float3 value10=(3.0f/64.0f)*(float3)(input[srcX+srcY_1], input[srcX+1+srcY_1], input[srcX+2+srcY_1]);
    float3 value11=(9.0f/64.0f)*(float3)(input[srcX+srcY], input[srcX+1+srcY], input[srcX+2+srcY]);
    float3 value12=(9.0f/64.0f)*(float3)(input[srcX+srcY1], input[srcX+1+srcY1], input[srcX+2+srcY1]);
    float3 value13=(3.0f/64.0f)*(float3)(input[srcX+srcY2], input[srcX+1+srcY2], input[srcX+2+srcY2]);
    float3 value20=(3.0f/64.0f)*(float3)(input[srcX+3+srcY_1], input[srcX+4+srcY_1], input[srcX+5+srcY_1]);
    float3 value21=(9.0f/64.0f)*(float3)(input[srcX+3+srcY], input[srcX+4+srcY], input[srcX+5+srcY]);
    float3 value22=(9.0f/64.0f)*(float3)(input[srcX+3+srcY1], input[srcX+4+srcY1], input[srcX+5+srcY1]);
    float3 value23=(3.0f/64.0f)*(float3)(input[srcX+3+srcY2], input[srcX+4+srcY2], input[srcX+5+srcY2]);
    float3 value30=(1.0f/64.0f)*(float3)(input[srcX+6+srcY_1], input[srcX+7+srcY_1], input[srcX+8+srcY_1]);
    float3 value31=(3.0f/64.0f)*(float3)(input[srcX+6+srcY], input[srcX+7+srcY], input[srcX+8+srcY]);
    float3 value32=(3.0f/64.0f)*(float3)(input[srcX+6+srcY1], input[srcX+7+srcY1], input[srcX+8+srcY1]);
    float3 value33=(1.0f/64.0f)*(float3)(input[srcX+6+srcY2], input[srcX+7+srcY2], input[srcX+8+srcY2]);

    float4 value=(float4)(value00+value01+value02+value03+
        value10+value11+value12+value13+
        value20+value21+value22+value23+
        value30+value31+value32+value33, 1.0);

    return value;
}

__kernel void reduceImage_float4(read_only image2d_t input, int width, int height, write_only  image2d_t output, __local float *localCache)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0);
    const int y=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);

    int cacheX;
    int cacheY;

    copyImageToLocal_float4(input, width, height, 2, localX, localY, localCache, cacheX, cacheY);
    float4 value=reduceLocal_float4_skipAlpha(localCache, localX, localY, cacheX);

    write_imagef(output, (int2)(x, y), value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//expand
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__kernel void image_expand_ui(read_only image2d_t input, write_only  image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0)*2;
    const int y=get_global_id(1)*2;

    const int srcX=select(-1, (x-1)/4, x);
    const int srcY=select(-1, (y-1)/4, y);
    const float deltaX=select(1.0f, (x-1.0f)/4.0f, x);
    const float deltaY=select(1.0f, (y-1.0f)/4.0f, y);

    float3 value1=convert_float3(read_imageui(input, nearestClampSampler, (int2)(srcX, srcY)).xyz)*(1.0f-deltaX)*(1.0f-deltaY);
    float3 value2=convert_float3(read_imageui(input, nearestClampSampler, (int2)(srcX+1, srcY)).xyz)*deltaX*(1.0f-deltaY);
    float3 value3=convert_float3(read_imageui(input, nearestClampSampler, (int2)(srcX, srcY+1)).xyz)*(1.0f-deltaX)*deltaY;
    float3 value4=convert_float3(read_imageui(input, nearestClampSampler, (int2)(srcX+1, srcY+1)).xyz)*deltaX*deltaY;

    uint4 value=(uint4)(convert_uint3(value1+value2+value3+value4), 255);
    write_imageui(output, (int2)(x, y), value);
}

__kernel void image_expand_float(read_only image2d_t input, write_only  image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int x=get_global_id(0)*2;
    const int y=get_global_id(1)*2;

    const int srcX=select(-1, (x-1)/4, x);
    const int srcY=select(-1, (y-1)/4, y);
    const float deltaX=select(1.0f, (x-1.0f)/4.0f, x);
    const float deltaY=select(1.0f, (y-1.0f)/4.0f, y);

    float3 value1=read_imagef(input, nearestClampSampler, (int2)(srcX, srcY)).xyz*(1.0f-deltaX)*(1.0f-deltaY);
    float3 value2=read_imagef(input, nearestClampSampler, (int2)(srcX+1, srcY)).xyz*deltaX*(1.0f-deltaY);
    float3 value3=read_imagef(input, nearestClampSampler, (int2)(srcX, srcY+1)).xyz*(1.0f-deltaX)*deltaY;
    float3 value4=read_imagef(input, nearestClampSampler, (int2)(srcX+1, srcY+1)).xyz*deltaX*deltaY;

    float4 value=(float4)(value1+value2+value3+value4, 255);
    write_imagef(output, (int2)(x, y), value);
}