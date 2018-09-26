
__kernel void convolve(read_only image2d_t input, __constant float *kernelBuffer, const int kernelSize, write_only image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;
    const int xOutput=get_global_id(0);
    const int yOutput=get_global_id(1);
    float sum=0;

    int filterOffset=kernelSize/2;
    int xInput=xOutput-filterOffset;
    int yInput=yOutput-filterOffset;

    for(int row=0; row<kernelSize; row++)
    {
        const int indexFilterRow=row*kernelSize;
        const int yInputRow=yInput+row;

        for(int col=0; col<kernelSize; col++)
        {
            const int indexFilter=indexFilterRow+col;
            float value=read_imagef(input, nearestClampSampler, (int2)(xInput+col, yInputRow)).x;

            sum+=kernelBuffer[indexFilter]*value;
        }
    }
    write_imagef(output, (int2)(xOutput, yOutput), sum);
}

__kernel void separableConvolveImage2DXY(read_only image2d_t input, int width, int height, __constant float *kernelX, __constant float *kernelY, int kernelSize, float scale, write_only image2d_t output, __local float *imageCache)
{
    const int globalX=get_global_id(0);
    const int globalY=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);
    const int halfKernelSize=kernelSize/2;

    int imageCacheX;
    int imageCacheY;

    copyToLocal(input, width, height, halfKernelSize, localX, localY, imageCache, &imageCacheX, &imageCacheY);
    
    if((globalX>=width)||(globalY>=height))
        return;
    
//    {
//        int cachePos=((localY+halfKernelSize)*imageCacheX)+localX+halfKernelSize;
//
//        write_imagef(output, (int2)(globalX, globalY), imageCache[cachePos]);
//        return;
//    }

//perform convolve on X
    const int posY=localY*2;
    float2 sum2=0.0f;

    if(posY<imageCacheY)
    {
        int cachePos=(posY*imageCacheX)+localX;

        for(int i=0; i<kernelSize; i++)
        {
            float2 value=(float2)(imageCache[cachePos], imageCache[cachePos+imageCacheX]);

            sum2+=kernelX[i]*value;
            cachePos++;
        }
    }

    int cachePos=((posY)*imageCacheX)+localX+halfKernelSize;

    barrier(CLK_LOCAL_MEM_FENCE);

    if(posY<imageCacheY)
    {
        imageCache[cachePos]=sum2.x;
        imageCache[cachePos+imageCacheX]=sum2.y;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

//    cachePos=((localY+halfKernelSize)*imageCacheX)+localX+halfKernelSize;
//    write_imagef(output, (int2)(globalX, globalY), imageCache[cachePos]);

//perform convolve on Y
    cachePos=(localY*imageCacheX)+localX+halfKernelSize;
    float sum=0.0f;

    for(int i=0; i<kernelSize; i++)
    {
        float value=imageCache[cachePos];

        sum+=kernelY[i]*value;
        cachePos+=imageCacheX;
    }

    sum=sum*scale;
    write_imagef(output, (int2)(globalX, globalY), sum);
}

__kernel void separableConvolveXImage2D(read_only image2d_t input, __constant float *kernelX, int kernelSize, float scale, write_only image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;

    const int globalX=get_global_id(0);
    const int globalY=get_global_id(1);

    float sum=0;
    int filterOffset=kernelSize/2;
    int xInput=globalX-filterOffset;

    for(int x=0; x<kernelSize; x++)
    {
        float value=read_imagef(input, nearestClampSampler, (int2)(xInput+x, globalY)).x;

        sum+=kernelX[x]*value;
    }

    sum=sum*scale;
    write_imagef(output, (int2)(globalX, globalY), sum);
}

__kernel void separableConvolveXImage2D_local(read_only image2d_t input, int width, int height, __constant float *kernelBuffer, int kernelSize, float scale, write_only image2d_t output, __local float *imageCache)
{
    const int xOutput=get_global_id(0);
    const int yOutput=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);

    const int halfKernelSize=kernelSize/2;
    int imageCacheX;
    int imageCacheY;

    copyToLocal(input, width, height, halfKernelSize, localX, localY, imageCache, &imageCacheX, &imageCacheY);

    int cachePos=((localY+halfKernelSize)*imageCacheX)+localX;
    float sum=0.0f;

    for(int i=0; i<kernelSize; i++)
    {
        sum+=kernelBuffer[i]*imageCache[cachePos];
        cachePos++;
    }

    sum=sum*scale;
    write_imagef(output, (int2)(xOutput, yOutput), sum);
}

__kernel void separableConvolveXImage2DBuffer(read_only image2d_t input, __constant float *kernelX, const int kernelSize, float scale, __global float *output, int offset, int width, int height)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;

    const int xOutput=get_global_id(0);
    const int yOutput=get_global_id(1);

    float sum=0;
    int filterOffset=kernelSize/2;
    int xInput=xOutput-filterOffset;
    int outputPos=offset+(yOutput*width)+xOutput;

    for(int x=0; x<kernelSize; x++)
    {
        float value=read_imagef(input, nearestClampSampler, (int2)(xInput+x, yOutput)).x;

        sum+=kernelX[x]*value;
    }

    sum=sum*scale;
    output[outputPos]=sum;
}

__kernel void separableConvolveXBuffer(__global float *input, int inputOffset, int width, int height, __constant float *kernelBuffer, const int kernelSize, float scale, __global float *output, int outputOffset)
{
    const int xOutput=get_global_id(0);
    const int yOutput=get_global_id(1);

    float sum=0;
    int kSize=kernelSize;
    const int filterOffset=kernelSize/2;
    const int intputPos=inputOffset+(yOutput*width)+(xOutput-filterOffset);
    const int outputPos=outputOffset+(yOutput*width)+xOutput;

    int imageStart=xOutput-filterOffset;
    int imageEnd=xOutput+filterOffset;
    int kernelStart=0;

    if(imageStart < 0)
    {
        int shift=-imageStart;
        int pos=intputPos+shift;

        for(int i=0; i<shift; i++)
            sum+=kernelBuffer[i]*input[pos];

        kernelStart=shift;
    }

    if(imageEnd >= width)
    {
        int shift=imageEnd-width+1;
        int pos=intputPos+(kernelSize-shift-1);

        for(int i=kernelSize-shift; i<kernelSize; i++)
            sum+=kernelBuffer[i]*input[pos];

        kSize-=shift;
    }

    for(int i=kernelStart; i<kSize; i++)
        sum+=kernelBuffer[i]*input[intputPos+i];

    sum=sum*scale;
    output[outputPos]=sum;
}

__kernel void separableConvolveYImage2D(read_only image2d_t input, __constant float *kernelY, const int kernelSize, float scale, write_only image2d_t output)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;

    const int xOutput=get_global_id(0);
    const int yOutput=get_global_id(1);

    float sum=0;
    int kSize=kernelSize;
    int filterOffset=kernelSize/2;
    int yInput=yOutput-filterOffset;

    for(int y=0; y<kernelSize; y++)
    {
        float value=read_imagef(input, nearestClampSampler, (int2)(xOutput, yInput+y)).x;
        
        sum+=kernelY[y]*value;
    }

    sum=sum*scale;
    write_imagef(output, (int2)(xOutput, yOutput), sum);
}

__kernel void separableConvolveYImage2D_local(read_only image2d_t input, int width, int height, __constant float *kernelBuffer, int kernelSize, float scale, write_only image2d_t output, __local float *imageCache)
{
    const int xOutput=get_global_id(0);
    const int yOutput=get_global_id(1);
    const int localX=get_local_id(0);
    const int localY=get_local_id(1);

    const int halfKernelSize=kernelSize/2;
    int imageCacheX;
    int imageCacheY;

    copyToLocal(input, width, height, halfKernelSize, localX, localY, imageCache, &imageCacheX, &imageCacheY);

    int cachePos=(localY*imageCacheX)+localX+halfKernelSize;
    float sum=0.0f;

    for(int i=0; i<kernelSize; i++)
    {
        sum+=kernelBuffer[i]*imageCache[cachePos];
        cachePos+=imageCacheX;
    }

    sum=sum*scale;
    write_imagef(output, (int2)(xOutput, yOutput), sum);
}

__kernel void separableConvolveYBuffer(__global float *input, int inputOffset, int width, int height, __constant float *kernelBuffer, const int kernelSize, float scale, __global float *output, int outputOffset)
{
    const int xOutput=get_global_id(0);
    const int yOutput=get_global_id(1);

    float sum=0;
    const int filterOffset=kernelSize/2;
    const int intputPos=inputOffset+((yOutput-filterOffset)*width)+xOutput;
    const int outputPos=outputOffset+(yOutput*width)+xOutput;

    int imageStart=yOutput-filterOffset;
    int imageEnd=yOutput+filterOffset;
    int kernelStart=0;
    int kSize=kernelSize;

    if(imageStart<0)
    {
        int shift=-imageStart;
        int pos=intputPos+(shift*width);

        for(int i=0; i<shift; i++)
            sum+=kernelBuffer[i]*input[pos];

        kernelStart=shift;
    }

    if(imageEnd>=height)
    {
        int shift=imageEnd-height+1;
        int pos=intputPos+((kernelSize-shift-1)*width);

        for(int i=kernelSize-shift; i<kernelSize; i++)
            sum+=kernelBuffer[i]*input[pos];

        kSize-=shift;
    }

    for(int i=kernelStart; i<kSize; i++)
        sum+=kernelBuffer[i]*input[intputPos+(i*width)];

    sum=sum*scale;
    output[outputPos]=sum;
}

