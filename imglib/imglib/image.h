#ifndef _imglib_image_h_
#define _imglib_image_h_

#include <stdint.h>
#include <memory>
#include <type_traits>

#include "imglib_export.h"
#include "imglib/location.h"

#ifdef IMGLIB_USE_COMPILETIME_TYPE
#include <ctti/type_id.hpp>
#endif//IMGLIB_USE_COMPILETIME_TYPE

namespace imglib
{

enum class Format
{
    Unknown,
    Binary,
    GreyScale, //Grey
    RA, //Red, Alpha
    RGB, //Red, Green, Blue
    RGBA, //Red, Green, Blue, Alpha
    BGRA, //Blue, Green, Red, Alpha
    YUV
};

IMGLIB_EXPORT const char *formatToString(Format format);
IMGLIB_EXPORT Format formatFromString(const char *name);
IMGLIB_EXPORT size_t formatToChannels(Format format);

enum class Depth
{
    Unknown,
    Bit1,
    Bit8,
    Bit10,
    Bit12,
    Bit14,
    Bit16,
    Float,
    Double,
    Packed //expecting several pack formats
};

IMGLIB_EXPORT const char *depthToString(Depth depth);
IMGLIB_EXPORT Depth depthFromString(const char *name);
IMGLIB_EXPORT size_t depthToBits(Depth depth);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Image Concept
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//all functions in this library are templated with a concept of an image that provides the following function in 
//the imglib namespace, see simpleImage.h for an example of how to use.
//namespace imglib
//{
  template<typename _Image>
  Format format(const _Image &){return Format::Unknown;}

  template<typename _Image>
  Depth depth(const _Image &){return Depth::Unknown;}
  
  template<typename _Image>
  Location location(const _Image &){return Location::System;}
  
  template<typename _Image>
  size_t width(const _Image &){return 0;}
  
  template<typename _Image>
  size_t height(const _Image &){return 0;}
  
  template<typename _Image>
  size_t stride(const _Image &){return 0;}
  
  template<typename _Image>
  size_t nativeId(const _Image &){return 0;}

  template<typename _Image>
  uint8_t *data(_Image &){return nullptr;}
  
  template<typename _Image>
  size_t dataSize(const _Image &){return 0;}

  template<typename _Image>
  bool resize(_Image &image, Format format, Depth depth, size_t width, size_t height){return false;}
//}

//Polymorphic image concept that is used internally in the library to wrap template types
//for some library functions, obviously adds overhead but allows some 3rd-party libraries
//to be hidden from the user and allows the templated image concept to be used.
class ImageWrapper
{
public:
    template<typename _Type>
    ImageWrapper(_Type &value):m_self(new ImageModel<_Type>(value)) {}
    ImageWrapper(const ImageWrapper &that):m_self(that.m_self) {}

#ifdef IMGLIB_USE_COMPILETIME_TYPE
    ctti::type_id_t typeId() const { return m_self->typeId(); }
#endif//IMGLIB_USE_COMPILETIME_TYPE
    void *object() const { m_self->object(); }

    Format format() const { return m_self->format(); }
    Depth depth() const { return m_self->depth(); }
    Location location() const { return m_self->location(); };

    size_t width() const { return m_self->width(); }
    size_t height() const { return m_self->height(); }
    size_t stride() const { return m_self->stride(); }

    size_t nativeId() const { return m_self->nativeId(); }
    uint8_t *data() const { return m_self->data(); }
    size_t dataSize() const { return m_self->dataSize(); }

    bool resize(Format format, Depth depth, size_t width, size_t height) const { return m_self->resize(format, depth, width, height); }

private:
    struct ImageConcept
    {
        virtual ~ImageConcept()=default;

#ifdef IMGLIB_USE_COMPILETIME_TYPE
        virtual ctti::type_id_t typeId() const=0;
#endif//IMGLIB_USE_COMPILETIME_TYPE
        virtual void *object() const=0;

        virtual Format format() const=0;
        virtual Depth depth() const=0;
        virtual Location location() const=0;

        virtual size_t width() const=0;
        virtual size_t height() const=0;
        virtual size_t stride() const=0;

        virtual size_t nativeId() const=0;
        virtual uint8_t *data() const=0;
        virtual size_t dataSize() const=0;

        virtual bool resize(Format format, Depth depth, size_t width, size_t height) const=0;
    };

    template<typename _Type>
    struct ImageModel:ImageConcept
    {
        ImageModel(_Type &value):m_data(value) {}

#ifdef IMGLIB_USE_COMPILETIME_TYPE
        ctti::type_id_t typeId() const { return ctti::type_id<_Type>(); }
#endif//IMGLIB_USE_COMPILETIME_TYPE
        void *object() const override { return (void *)&m_data; }

        Format format() const override { return imglib::format(m_data); }
        Depth depth() const override { return imglib::depth(m_data); }
        Location location() const override { return imglib::location(m_data); }

        size_t width() const override { return imglib::width(m_data); }
        size_t height() const override { return imglib::height(m_data); }
        size_t stride() const override { return imglib::stride(m_data); }

        size_t nativeId() const override { return imglib::nativeId(m_data); }
        uint8_t *data() const override { return imglib::data(const_cast<typename std::remove_const<_Type>::type &>(m_data)); }
        size_t dataSize() const override { return imglib::dataSize(m_data); }

        bool resize(Format format, Depth depth, size_t width, size_t height) const override { return imglib::resize(const_cast<typename std::remove_const<_Type>::type &>(m_data), format, depth, width, height); }

        _Type &m_data;
    };

    std::shared_ptr<const ImageConcept> m_self;
};

namespace traits
{

template<Format _Format>
constexpr size_t channels();

template<>
constexpr size_t channels<Format::Binary>() { return 1; }
template<>
constexpr size_t channels<Format::GreyScale>() { return 1; }
template<>
constexpr size_t channels<Format::RA>() { return 2; }
template<>
constexpr size_t channels<Format::RGB>() { return 3; }
template<>
constexpr size_t channels<Format::RGBA>() { return 4; }
template<>
constexpr size_t channels<Format::BGRA>() { return 4; }
template<>
constexpr size_t channels<Format::YUV>() { return 3; }

template<Depth _Depth>
constexpr size_t sizeOfDepth();

template<>
constexpr size_t sizeOfDepth<Depth::Bit1>() { return 1; }

template<>
constexpr size_t sizeOfDepth<Depth::Bit8>() { return sizeof(uint8_t); }

template<>
constexpr size_t sizeOfDepth<Depth::Bit10>() { return sizeof(uint16_t); }

template<>
constexpr size_t sizeOfDepth<Depth::Bit12>() { return sizeof(uint16_t); }

template<>
constexpr size_t sizeOfDepth<Depth::Bit14>() { return sizeof(uint16_t); }

template<>
constexpr size_t sizeOfDepth<Depth::Bit16>() { return sizeof(uint16_t); }

template<>
constexpr size_t sizeOfDepth<Depth::Float>() { return sizeof(float); }

template<>
constexpr size_t sizeOfDepth<Depth::Double>() { return sizeof(double); }

template<Format _Format, Depth _Depth>
struct sizeOfFormatDepth { static constexpr size_t value=channels<_Format>()*sizeOfDepth<_Depth>(); };

template<Format _Format, Depth _Depth>
size_t sizeOf() {return sizeOfFormatDepth<_Format, _Depth>::value;};

} //namespace traits

IMGLIB_EXPORT size_t sizeOfPixel(Format format, Depth depth);

namespace utils
{

template<Format _Format, Depth _Depth>
bool comparePixel(uint8_t *src1Data, uint8_t *src2Data);

template<>
inline bool comparePixel<Format::Binary, Depth::Bit1>(uint8_t *src1Data, uint8_t *src2Data) { return (*src1Data)==(*src2Data); }

template<>
inline bool comparePixel<Format::Binary, Depth::Bit8>(uint8_t *src1Data, uint8_t *src2Data) { return (*src1Data)==(*src2Data); }

template<>
inline bool comparePixel<Format::GreyScale, Depth::Bit8>(uint8_t *src1Data, uint8_t *src2Data) { return (*src1Data)==(*src2Data); }

template<>
inline bool comparePixel<Format::GreyScale, Depth::Float>(uint8_t *src1Data, uint8_t *src2Data) { return (*(float *)(src1Data))==(*(float *)(src2Data)); }

template<>
inline bool comparePixel<Format::RA, Depth::Bit8>(uint8_t *src1Data, uint8_t *src2Data) { return (src1Data[0]==src2Data[0])&&(src1Data[1]==src2Data[1]); }

template<>
inline bool comparePixel<Format::RGB, Depth::Bit8>(uint8_t *src1Data, uint8_t *src2Data) { return (src1Data[0]==src2Data[0])&&(src1Data[1]==src2Data[1])&&(src1Data[2]==src2Data[2]); }

template<>
inline bool comparePixel<Format::RGBA, Depth::Bit8>(uint8_t *src1Data, uint8_t *src2Data) { return (src1Data[0]==src2Data[0])&&(src1Data[1]==src2Data[1])&&(src1Data[2]==src2Data[2])&&(src1Data[3]==src2Data[3]); }

template<>
inline bool comparePixel<Format::RA, Depth::Float>(uint8_t *src1Data, uint8_t *src2Data)
{
    float *src1Float=(float *)src1Data;
    float *src2Float=(float *)src2Data; 
    return (src1Float[0]==src2Float[0])&&(src1Float[1]==src2Float[1]);
}

template<>
inline bool comparePixel<Format::RGB, Depth::Float>(uint8_t *src1Data, uint8_t *src2Data)
{
    float *src1Float=(float *)src1Data;
    float *src2Float=(float *)src2Data;
    return (src1Float[0]==src2Float[0])&&(src1Float[1]==src2Float[1])&&(src1Float[2]==src2Float[2]);
}

template<>
inline bool comparePixel<Format::RGBA, Depth::Float>(uint8_t *src1Data, uint8_t *src2Data)
{
    float *src1Float=(float *)src1Data;
    float *src2Float=(float *)src2Data;
    return (src1Float[0]==src2Float[0])&&(src1Float[1]==src2Float[1])&&(src1Float[2]==src2Float[2])&&(src1Float[3]==src2Float[3]);
}

} //namespace utils

}//namespace imglib

#endif //_imglib_image_h_