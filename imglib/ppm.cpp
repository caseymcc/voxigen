#include "imglib/ppm.h"

#include <cassert>
#include <vector>

namespace imglib
{

void writePpmGreyScale(ImageWrapper &image, FILE *file);
void writePpmGreyScale32f(ImageWrapper &image, FILE *file);
void writePpmRGB(ImageWrapper &image, FILE *file);
void writePpmRGBA(ImageWrapper &image, FILE *file);

bool readNextNumber(FILE *file, std::vector<char> &buffer, size_t &number)
{
    //skip whitespace
    size_t read=fread(buffer.data(), 1, 1, file);
    while((read!=0)&&((buffer[0]=='\n')||(buffer[0]=='\r')||(buffer[0]==' ')||(buffer[0]=='\t')))
    {
        read=fread(buffer.data(), 1, 1, file);
    }

    if(read==0)
        return false;

    //read number
    size_t index=0;

    while((read!=0) && (index<buffer.size()) && ((buffer[index]!='\n') && (buffer[index]!='\r') && (buffer[index]!=' ') && (buffer[index]!='\t')))
    {
        index++;
        read=fread(&buffer[index], 1, 1, file);
    }

    if((read==0)||(index==0))
        return false;

    buffer[index]='\0';
    number=atoi(buffer.data());
    return true;
}

bool loadPpm(ImageWrapper image, const char *filename)
{
    FILE *file=fopen(filename, "rb");

    if(file==NULL)
        return false;

    std::vector<char> buffer(10);

    size_t read=fread(buffer.data(), 1, 2, file);

    if(read==0)
        return false;

    if(strncmp(buffer.data(), "P6", 2)!=0)
        return false;

    size_t width;
    size_t height;
    size_t maxColor;

    if(!readNextNumber(file, buffer, width))
        return false;
    if(!readNextNumber(file, buffer, height))
        return false;
    if(!readNextNumber(file, buffer, maxColor))
        return false;

    if(maxColor!=255)
    {
        assert(false);//not implemented
        return false;
    }

    image.resize(Format::RGB, Depth::Bit8, width, height);

    if(image.dataSize()<width*height*3)
        return false;

    uint8_t *data=image.data();

    fread(image.data(), 1, width*height*3, file);
    
    return true;
}

bool savePpm(ImageWrapper image, const char *filename)
{
	FILE *file=fopen(filename, "wb");

	if(file==NULL)
		return false;

    bool success=false;
	fprintf(file, "P6\n%d %d\n255\n", image.width(), image.height());

	switch(image.format())
	{
	case Format::Binary:
	case Format::GreyScale:
        if(image.depth()==Depth::Bit8)
        {
            writePpmGreyScale(image, file);
            success=true;
        }
        else if(image.depth()==Depth::Float)
        {
            writePpmGreyScale32f(image, file);
            success=true;
        }
	case Format::RGB:
        if(image.depth()==Depth::Bit8)
        {
            writePpmRGB(image, file);
            success=true;
        }
		break;
	case Format::RGBA:
        if(image.depth()==Depth::Bit8)
        {
            writePpmRGBA(image, file);
            success=true;
        }
		break;
	}
	
	fclose(file);
	return success;
}

void writePpmGreyScale(ImageWrapper &image, FILE *file)
{
	uint8_t *data=image.data();

	for(size_t y=0; y<image.height(); ++y)
	{
		for(size_t x=0; x<image.width(); ++x)
		{
			fwrite(data, 1, 1, file);
			fwrite(data, 1, 1, file);
			fwrite(data, 1, 1, file);
			++data;
		}
	}
}

void writePpmGreyScale32f(ImageWrapper &image, FILE *file)
{
	float *data=(float *)image.data();

	float min=std::numeric_limits<float>::max();
	float max=std::numeric_limits<float>::min();

	for(size_t y=0; y<image.height(); ++y)
	{
		for(size_t x=0; x<image.width(); ++x)
		{
			float &value=*data;

			if(value>max)
				max=value;
			else if(value<min)
				min=value;
			++data;
		}
	}
	
	float range=max-min;
	float scale=255.0f/range;

	data=(float *)image.data();
	for(size_t y=0; y<image.height(); ++y)
	{
		for(size_t x=0; x<image.width(); ++x)
		{
			uint8_t value=((*data)+min)*scale;
			fwrite(&value, 1, 1, file);
			fwrite(&value, 1, 1, file);
			fwrite(&value, 1, 1, file);
			++data;
		}
	}
}

void writePpmRGB(ImageWrapper &image, FILE *file)
{
	uint8_t *data=image.data();

	for(size_t y=0; y<image.height(); ++y)
	{
		for(size_t x=0; x<image.width(); ++x)
		{
			fwrite(data, 1, 3, file);
			data+=3;
		}
	}
}

void writePpmRGBA(ImageWrapper &image, FILE *file)
{
	uint8_t *data=image.data();

	for(size_t y=0; y<image.height(); ++y)
	{
		for(size_t x=0; x<image.width(); ++x)
		{
			fwrite(data, 1, 3, file);
			data+=4;
		}
	}
}

}//namespace imglib