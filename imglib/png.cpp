#include "imglib/png.h"
#include <png.h>

#include <vector>
#include  <cassert>

namespace imglib
{

png_byte getPngColorType(Format format)
{
    png_byte color_type;

    switch(format)
    {
    case Format::GreyScale:
        color_type=PNG_COLOR_TYPE_GRAY;
        break;
    case Format::RA:
        color_type=PNG_COLOR_TYPE_GRAY_ALPHA;
        break;
    case Format::RGB:
        color_type=PNG_COLOR_TYPE_RGB;
        break;
    case Format::RGBA:
        color_type=PNG_COLOR_TYPE_RGB_ALPHA;
        break;
    }

    return color_type;
}

png_byte getPngBitDepth(Depth depth)
{
    png_byte bit_depth;

    switch(depth)
    {
    case Depth::Bit8:
        bit_depth=8;
        break;
    case Depth::Bit10:
        bit_depth=10;
        break;
    case Depth::Bit12:
        bit_depth=12;
        break;
    case Depth::Bit14:
        bit_depth=14;
        break;
    case Depth::Bit16:
        bit_depth=16;
        break;
    case Depth::Float:
        bit_depth=32;
        break;
    case Depth::Double:
        bit_depth=64;
        break;
    break;
    }

    return bit_depth;
}

std::vector<png_bytep> getRowPointers(ImageWrapper &image)
{
    std::vector<png_bytep> row_pointers(image.height());
    png_byte *imageData=(png_byte *)image.data();

    size_t pos=0;
    size_t stride=image.width()*sizeOfPixel(image.format(), image.depth());

    for(int y=0; y<image.height(); y++)
    {
        row_pointers[y]=&imageData[pos];
        pos+=stride;
    }

    return row_pointers;
}

bool loadPng(ImageWrapper image, const char *filename)
{
    FILE *file=fopen(filename, "rb");

    if(!file)
        return false;

    char header[8];

    fread(header, 1, 8, file);

    if(png_sig_cmp((png_const_bytep)header, 0, 8))
        return false;

    png_structp png_ptr;

    /* initialize stuff */
    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if(!png_ptr)
        return false;

    png_infop info_ptr;

    info_ptr=png_create_info_struct(png_ptr);
    if(!info_ptr)
        return false;

    if(setjmp(png_jmpbuf(png_ptr)))
        return false;

    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    int number_of_passes;

    png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    width=png_get_image_width(png_ptr, info_ptr);
    height=png_get_image_height(png_ptr, info_ptr);
    color_type=png_get_color_type(png_ptr, info_ptr);
    bit_depth=png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes=png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    if(setjmp(png_jmpbuf(png_ptr)))
        return false;

    Format format;
    Depth depth;
    int channels;

    if(color_type==PNG_COLOR_TYPE_GRAY)
    {
        channels=1;
        format=Format::GreyScale;
        depth=Depth::Bit8;
    }
    else if(color_type==PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        channels=2;
        format=Format::RA;
        depth=Depth::Bit8;
    }
    else if(color_type==PNG_COLOR_TYPE_RGB)
    {
        channels=3;
        format=Format::RGB;
        depth=Depth::Bit8;
    }
    else if(color_type==PNG_COLOR_TYPE_RGB_ALPHA)
    {
        channels=4;
        format=Format::RGBA;
        depth=Depth::Bit8;
    }

    image.resize(format, depth, width, height);
    std::vector<png_bytep> row_pointers=getRowPointers(image);

    png_read_image(png_ptr, row_pointers.data());

    fclose(file);
    return true;
}

bool savePng(ImageWrapper image, const char *filename)
{
    int y;

    FILE *file=fopen(filename, "wb");
    
    if(!file)
        return false;

    png_structp png=png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if(!png)
        return false;

    png_infop info=png_create_info_struct(png);
    
    if(!info)
        return false;

    if(setjmp(png_jmpbuf(png)))
        return false;

    png_init_io(png, file);

    png_byte color_type=getPngColorType(image.format());
    png_byte bit_depth=getPngBitDepth(image.depth());

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
        png,
        info,
        image.width(), image.height(),
        bit_depth,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    std::vector<png_bytep> row_pointers=getRowPointers(image);

    png_write_image(png, row_pointers.data());
    png_write_end(png, NULL);

    fclose(file);
    return false;
}

}//namespace cvlib