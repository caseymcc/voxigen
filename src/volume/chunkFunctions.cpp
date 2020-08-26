#include "voxigen/volume/chunkFunctions.h"

namespace voxigen
{

void spiralIndex(glm::ivec3 &volumeSize, std::vector<size_t> &indexes)
{
    indexes.clear();

    glm::ivec3 center(volumeSize.x/2, volumeSize.y/2, volumeSize.z/2);
    int maxSteps=glm::compMax(center);

    size_t z=center.z;

    for(int i=0; i<maxSteps; ++i)
    {
        spiralIndexXYPlane(z, i, volumeSize, center, indexes);
        for(int j=1; j<=i; ++j)
        {
            spiralIndexXYPlane(z+j, i-j, volumeSize, center, indexes);
            spiralIndexXYPlane(z-j, i-j, volumeSize, center, indexes);
        }
    }

}

void spiralIndexXYPlane(int z, int pos, glm::ivec3 &volumeSize, glm::ivec3 &center, std::vector<size_t> &indexes)
{
    if(pos<0)
        return;

    int x;
    int y;
    int index;

    if(pos==0)
    {
        index=(volumeSize.y*volumeSize.x*z)+(volumeSize.x*center.y)+center.x;
        indexes.push_back(index);
    }
    else
    {
        x=center.x+pos;

        if((x>=0)&&(x<volumeSize.x))
        {
            for(y=center.y; y<=center.y+pos; ++y)
            {
                if((y<0)&&(y>=volumeSize.y))
                    continue;

                index=(volumeSize.y*volumeSize.x*z)+(volumeSize.x*y)+x;
                indexes.push_back(index);
            }
        }
        else
            y=center.y+pos;
        if((y>=0)&&(y<volumeSize.y))
        {
            for(x=center.x+pos; x>=center.x-pos; --x)
            {
                if((x<0)&&(x>=volumeSize.x))
                    continue;

                index=(volumeSize.y*volumeSize.x*z)+(volumeSize.x*y)+x;
                indexes.push_back(index);
            }
        }
        else
            x=center.x-pos;
        if((x>=0)&&(x<volumeSize.x))
        {
            for(y=center.y+pos; y>=center.y-pos; --y)
            {
                if((y<0)&&(y>=volumeSize.y))
                    continue;

                index=(volumeSize.y*volumeSize.x*z)+(volumeSize.x*y)+x;
                indexes.push_back(index);
            }
        }
        else
            y=center.y-pos;
        if((y>=0)&&(y<volumeSize.y))
        {
            for(x=center.x-pos; x<=center.x+pos; ++x)
            {
                if((x<0)&&(x>=volumeSize.x))
                    continue;

                index=(volumeSize.y*volumeSize.x*z)+(volumeSize.x*y)+x;
                indexes.push_back(index);
            }
        }
        else
            x=center.x+pos;
        if((x>=0)&&(x<volumeSize.x))
        {
            for(y=center.y-pos; y<center.y; ++y)
            {
                if((y<0)&&(y>=volumeSize.y))
                    continue;

                index=(volumeSize.y*volumeSize.x*z)+(volumeSize.x*y)+x;
                indexes.push_back(index);
            }
        }
    }
}

}//namespace voxigen
