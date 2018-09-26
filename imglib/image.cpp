#include "imglib/image.h"
#include <unordered_map>

namespace imglib
{

struct FormatDetails
{
    const char *name;
    size_t channels;
};

static std::unordered_map<Format, FormatDetails> FormatDetailsMap=
{
    {Format::Binary,{"Binary", traits::channels<Format::Binary>()}},
    {Format::GreyScale,{"GreyScale", traits::channels<Format::GreyScale>()}},
    {Format::RA,{"RA", traits::channels<Format::RA>()}},
    {Format::RGB,{"RGB", traits::channels<Format::RGB>()}},
    {Format::RGBA,{"RGBA", traits::channels<Format::RGBA>()}},
    {Format::BGRA,{"BGRA", traits::channels<Format::BGRA>()}},
    {Format::YUV,{"YUV", traits::channels<Format::YUV>()}}
};

struct DepthDetails
{
    const char *name;
    size_t bits;
};

static std::unordered_map<Depth, DepthDetails> DepthDetailsMap=
{
    {Depth::Bit1,{"1 bit", traits::sizeOfDepth<Depth::Bit1>()}},
    {Depth::Bit8,{"8 bit", traits::sizeOfDepth<Depth::Bit8>()}},
    {Depth::Bit10,{"10 bit", traits::sizeOfDepth<Depth::Bit10>()}},
    {Depth::Bit12,{"12 bit", traits::sizeOfDepth<Depth::Bit12>()}},
    {Depth::Bit14,{"14 bit", traits::sizeOfDepth<Depth::Bit14>()}},
    {Depth::Bit16,{"16 bit", traits::sizeOfDepth<Depth::Bit16>()}},
    {Depth::Float,{"Float", traits::sizeOfDepth<Depth::Float>()}},
    {Depth::Double,{"Double", traits::sizeOfDepth<Depth::Double>()}}//,
//    {Depth::Packed,{"Packed", traits::sizeOfDepth<Depth::Packed>()}}
};


const char *formatToString(Format format)
{
    auto iter=FormatDetailsMap.find(format);

    if(iter==FormatDetailsMap.end())
        return "Unknown";
    return iter->second.name;
}

Format formatFromString(const char *name)
{
    for(auto &iter:FormatDetailsMap)
    {
        if(strcmp(iter.second.name, name) == 0)
            return iter.first;
    }
    return Format::Binary;
}

size_t formatToChannels(Format format)
{
    auto iter=FormatDetailsMap.find(format);

    if(iter==FormatDetailsMap.end())
        return 0;
    return iter->second.channels;
}

const char *depthToString(Depth depth)
{
    auto iter=DepthDetailsMap.find(depth);

    if(iter==DepthDetailsMap.end())
        return "Unknown";
    return iter->second.name;
}

Depth depthFromString(const char *name)
{
    for(auto &iter:DepthDetailsMap)
    {
        if(strcmp(iter.second.name, name)==0)
            return iter.first;
    }
    return Depth::Bit1;
}

size_t depthToBits(Depth depth)
{
    auto iter=DepthDetailsMap.find(depth);

    if(iter==DepthDetailsMap.end())
        return 0;
    return iter->second.bits;
}

template<Format _Format>
size_t sizeOfImageDepth(Depth depth)
{
    switch(depth)
    {
    case Depth::Bit1:
        return traits::sizeOf<_Format, Depth::Bit1>();
    case Depth::Bit8:
        return traits::sizeOf<_Format, Depth::Bit8>();
    case Depth::Bit10:
        return traits::sizeOf<_Format, Depth::Bit10>();
    case Depth::Bit12:
        return traits::sizeOf<_Format, Depth::Bit12>();
    case Depth::Bit14:
        return traits::sizeOf<_Format, Depth::Bit14>();
    case Depth::Bit16:
        return traits::sizeOf<_Format, Depth::Bit16>();
    case Depth::Float:
        return traits::sizeOf<_Format, Depth::Float>();
    case Depth::Double:
        return traits::sizeOf<_Format, Depth::Double>();
    }
    return 0;
}

size_t sizeOfPixel(Format format, Depth depth)
{
    switch(format)
    {
    case Format::Binary:
        return sizeOfImageDepth<Format::Binary>(depth);
        break;
    case Format::GreyScale:
        return sizeOfImageDepth<Format::GreyScale>(depth);
        break;
    case Format::RA:
        return sizeOfImageDepth<Format::RA>(depth);
        break;
    case Format::RGB:
        return sizeOfImageDepth<Format::RGB>(depth);
        break;
    case Format::RGBA:
        return sizeOfImageDepth<Format::RGBA>(depth);
        break;
    }
    return 0;
}

}//namespace imglib
