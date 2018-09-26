////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Image Copies
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void copyImageToLocal_ui(read_only image2d_t input, int width, int height, int border, int localX, int localY, __local uint *imageCache, int *imageCacheX, int *imageCacheY)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;

    const int groupX=get_group_id(0);
    const int groupY=get_group_id(1);
    const int localSizeX=get_local_size(0);
    const int localSizeY=get_local_size(1);

    const int imageX=(groupX*localSizeX)-border;
    const int imageY=(groupY*localSizeY)-border;

    int imgCacheX=localSizeX+(2*border);
    int imgCacheY=localSizeY+(2*border);

    if(imageX+imgCacheX>=width)
        imgCacheX=width-imageX+1;
    if(imageY+imgCacheY>=height)
        imgCacheY=height-imageY+1;

    const int imageCacheSize=imgCacheX*imgCacheY;
    const int perItemCache=ceil((float)imageCacheSize/(localSizeX*localSizeY));
    const int cacheIndex=((localY*localSizeX)+localX)*perItemCache;

    if(cacheIndex < imageCacheSize)
    {
        const int cacheY=cacheIndex/imgCacheX;
        const int indexY=imageY+cacheY;
        int indexX=imageX+cacheIndex-(cacheY*imgCacheX);

        for(int i=0; i<perItemCache; ++i)
        {
            imageCache[cacheIndex+i]=read_imageui(input, nearestClampSampler, (int2)(indexX, indexY)).x;
            indexX++;
        }
    }

    (*imageCacheX)=imgCacheX;
    (*imageCacheY)=imgCacheY;

    barrier(CLK_LOCAL_MEM_FENCE);
}

void copyImageToLocal_uint4(read_only image2d_t input, int width, int height, int border, int localX, int localY, __local uint *imageCache, int *imageCacheX, int *imageCacheY)
{

}

void copyImageToLocal_float(read_only image2d_t input, int width, int height, int border, int localX, int localY, __local float *imageCache, int *imageCacheX, int *imageCacheY)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;

    const int groupX=get_group_id(0);
    const int groupY=get_group_id(1);
    const int localSizeX=get_local_size(0);
    const int localSizeY=get_local_size(1);

    const int imageX=(groupX*localSizeX)-border;
    const int imageY=(groupY*localSizeY)-border;

    int imgCacheX=localSizeX+(2*border);
    int imgCacheY=localSizeY+(2*border);

    if(imageX+imgCacheX>=width)
        imgCacheX=width-imageX+1;
    if(imageY+imgCacheY>=height)
        imgCacheY=height-imageY+1;

    const int imageCacheSize=imgCacheX*imgCacheY;
    const int perItemCache=ceil((float)imageCacheSize/(localSizeX*localSizeY));
    const int cacheIndex=((localY*localSizeX)+localX)*perItemCache;

    if(cacheIndex < imageCacheSize)
    {
        const int cacheY=cacheIndex/imgCacheX;
        const int indexY=imageY+cacheY;
        int indexX=imageX+cacheIndex-(cacheY*imgCacheX);

        for(int i=0; i<perItemCache; ++i)
        {
            imageCache[cacheIndex+i]=read_imagef(input, nearestClampSampler, (int2)(indexX, indexY)).x;
            indexX++;
        }
    }

    (*imageCacheX)=imgCacheX;
    (*imageCacheY)=imgCacheY;

    barrier(CLK_LOCAL_MEM_FENCE);
}

void copyImageToLocal_float4(read_only image2d_t input, int width, int height, int border, int localX, int localY, __local float *imageCache, int *imageCacheX, int *imageCacheY)
{
    const sampler_t nearestClampSampler=CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST|CLK_ADDRESS_CLAMP_TO_EDGE;

    const int groupX=get_group_id(0);
    const int groupY=get_group_id(1);
    const int localSizeX=get_local_size(0);
    const int localSizeY=get_local_size(1);

    const int imageX=(groupX*localSizeX)-border;
    const int imageY=(groupY*localSizeY)-border;

    int imgCacheX=localSizeX+(2*border);
    int imgCacheY=localSizeY+(2*border);

    if(imageX+imgCacheX>=width)
        imgCacheX=width-imageX+1;
    if(imageY+imgCacheY>=height)
        imgCacheY=height-imageY+1;

    const int imageCacheSize=imgCacheX*imgCacheY;
    const int perItemCache=ceil((float)imageCacheSize/(localSizeX*localSizeY));
    const int cacheIndex=((localY*localSizeX)+localX)*perItemCache;

    if(cacheIndex < imageCacheSize)
    {
        const int cacheY=cacheIndex/imgCacheX;
        const int indexY=imageY+cacheY;
        int indexX=imageX+cacheIndex-(cacheY*imgCacheX);

        for(int i=0; i<perItemCache; ++i)
        {
            float4 value=read_imagef(input, nearestClampSampler, (int2)(indexX, indexY));
            
            imageCache[(cacheIndex*4)+(i*4)]=value.x;
            imageCache[(cacheIndex*4)+(i*4)+1]=value.y;
            imageCache[(cacheIndex*4)+(i*4)+2]=value.z;
            imageCache[(cacheIndex*4)+(i*4)+3]=value.w;
            indexX++;
        }
    }

    (*imageCacheX)=imgCacheX;
    (*imageCacheY)=imgCacheY;

    barrier(CLK_LOCAL_MEM_FENCE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Buffer Copies
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void copyBufferToLocal_ui(__global uint *input, int width, int height, int border, int localX, int localY, __local uint *imageCache, int *imageCacheX, int *imageCacheY)
{
    const int groupX=get_group_id(0);
    const int groupY=get_group_id(1);
    const int localSizeX=get_local_size(0);
    const int localSizeY=get_local_size(1);

    const int imageX=(groupX*localSizeX)-border;
    const int imageY=(groupY*localSizeY)-border;

    int imgCacheX=localSizeX+(2*border);
    int imgCacheY=localSizeY+(2*border);

    if(imageX+imgCacheX>=width)
        imgCacheX=width-imageX+1;
    if(imageY+imgCacheY>=height)
        imgCacheY=height-imageY+1;

    const int imageCacheSize=imgCacheX*imgCacheY;
    const int perItemCache=ceil((float)imageCacheSize/(localSizeX*localSizeY));
    const int cacheIndex=((localY*localSizeX)+localX)*perItemCache;

    if(cacheIndex < imageCacheSize)
    {
        const int cacheY=cacheIndex/imgCacheX;
        const int indexY=imageY+cacheY;
        int indexX=imageX+cacheIndex-(cacheY*imgCacheX);

        for(int i=0; i<perItemCache; ++i)
        {
            imageCache[cacheIndex+i]=input[indexY*width+indexX];
            indexX++;
        }
    }

    (*imageCacheX)=imgCacheX;
    (*imageCacheY)=imgCacheY;

    barrier(CLK_LOCAL_MEM_FENCE);
}

void copyBufferToLocal_uint4(__global uint *input, int width, int height, int overlap, int localX, int localY, __local uint *imageCache, int *imageCacheX, int *imageCacheY)
{
    const int groupX=get_group_id(0);
    const int groupY=get_group_id(1);
    const int localSizeX=get_local_size(0);
    const int localSizeY=get_local_size(1);

    const int imageX=(groupX*localSizeX)-overlap;
    const int imageY=(groupY*localSizeY)-overlap;

    int imgCacheX=localSizeX+(2*overlap);
    int imgCacheY=localSizeY+(2*overlap);

    if(imageX+imgCacheX>=width)
        imgCacheX=width-imageX+1;
    if(imageY+imgCacheY>=height)
        imgCacheY=height-imageY+1;

    const int imageCacheSize=imgCacheX*imgCacheY;
    const int perItemCache=ceil((float)imageCacheSize/(localSizeX*localSizeY));
    const int cacheIndex=((localY*localSizeX)+localX)*perItemCache;
    const int stride=width*4;

    if(cacheIndex < imageCacheSize)
    {
        const int cacheY=cacheIndex/imgCacheX;
        const int indexY=imageY+cacheY;
        int indexX=imageX+cacheIndex-(cacheY*imgCacheX);

        for(int i=0; i<perItemCache; ++i)
        {
            imageCache[(cacheIndex*4)+(i*4)]=input[indexY*stride+(indexX*4)];
            imageCache[(cacheIndex*4)+(i*4)+1]=input[indexY*stride+(indexX*4)+1];
            imageCache[(cacheIndex*4)+(i*4)+2]=input[indexY*stride+(indexX*4)+2];
            imageCache[(cacheIndex*4)+(i*4)+3]=input[indexY*stride+(indexX*4)+3];
            indexX++;
        }
    }

    (*imageCacheX)=imgCacheX;
    (*imageCacheY)=imgCacheY;

    barrier(CLK_LOCAL_MEM_FENCE);
}

void copyBufferToLocal_float(__global float *input, int width, int height, int border, int localX, int localY, __local float *imageCache, int *imageCacheX, int *imageCacheY)
{
    const int groupX=get_group_id(0);
    const int groupY=get_group_id(1);
    const int localSizeX=get_local_size(0);
    const int localSizeY=get_local_size(1);

    const int imageX=(groupX*localSizeX)-border;
    const int imageY=(groupY*localSizeY)-border;

    int imgCacheX=localSizeX+(2*border);
    int imgCacheY=localSizeY+(2*border);

    if(imageX+imgCacheX>=width)
        imgCacheX=width-imageX+1;
    if(imageY+imgCacheY>=height)
        imgCacheY=height-imageY+1;

    const int imageCacheSize=imgCacheX*imgCacheY;
    const int perItemCache=ceil((float)imageCacheSize/(localSizeX*localSizeY));
    const int cacheIndex=((localY*localSizeX)+localX)*perItemCache;

    if(cacheIndex < imageCacheSize)
    {
        const int cacheY=cacheIndex/imgCacheX;
        const int indexY=imageY+cacheY;
        int indexX=imageX+cacheIndex-(cacheY*imgCacheX);

        for(int i=0; i<perItemCache; ++i)
        {
            imageCache[cacheIndex+i]=input[indexY*width+indexX];
            indexX++;
        }
    }

    (*imageCacheX)=imgCacheX;
    (*imageCacheY)=imgCacheY;

    barrier(CLK_LOCAL_MEM_FENCE);
}

void copyBufferToLocal_float4(__global float *input, int width, int height, int border, int localX, int localY, __local float *imageCache, int *imageCacheX, int *imageCacheY)
{
}
