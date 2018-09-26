#ifndef _cvlib_slideWindow_h_
#define _cvlib_slideWindow_h_

namespace cvlib
{

template<typename _DataType>
void minimumSlidingWindow(const std::vector<_DataType> &data, std::vector<_DataType> &output, size_t windowSize)
{
    if(output.size()<data.size())
        output.resize(data.size());
    slidingWindow<_DataType, std::greater_equal<_DataType>>(data.data(), output.data(), data.size(), windowSize);
}

template<typename _DataType>
std::vector<_DataType> minimumSlidingWindow(const std::vector<_DataType> &data, size_t windowSize)
{
    std::vector<_DataType> output(data.size());

    slidingWindow<_DataType, std::greater_equal<_DataType>>(data.data(), output.data(), data.size(), windowSize);
    return output;
}

template<typename _DataType>
std::vector<_DataType> minimumSlidingWindow(_DataType const *data, _DataType *output, size_t dataSize, size_t windowSize)
{
    std::vector<_DataType> output(data.size());

    slidingWindow<_DataType, std::greater_equal<_DataType>>(data, output, dataSize, windowSize);
    return output;
}

template<typename _DataType>
void maximumSlidingWindow(const std::vector<_DataType> &data, std::vector<_DataType> &output, size_t windowSize)
{
    if(output.size()<data.size())
        output.resize(data.size());
    slidingWindow<_DataType, std::less_equal<_DataType>>(data.data(), output.data(), data.size(), windowSize);
}

template<typename _DataType>
std::vector<_DataType> maximumSlidingWindow(const std::vector<_DataType> &data, size_t windowSize)
{
    std::vector<_DataType> output(data.size());

    slidingWindow<_DataType, std::less_equal<_DataType>>(data.data(), output.data(), data.size(), windowSize);
    return output;
}

template<typename _DataType>
std::vector<_DataType> maximumSlidingWindow(_DataType const *data, _DataType *output, size_t dataSize, size_t windowSize)
{
    std::vector<_DataType> output(data.size());

    slidingWindow<_DataType, std::less_equal<_DataType>>(data, output, dataSize, windowSize);
    return output;
}

template<typename _DataType, typename _Comparator>
void slidingWindow(_DataType const *data, _DataType *output, size_t dataSize, size_t windowSize, _Comparator compare=std::greater_equal<_DataType>)
{
    std::deque<std::pair<_DataType, size_t>> window;
    size_t offset=windowSize/2;
    size_t endSlide=dataSize-offset;

    //slide in
    for(size_t i=0; i<offset; ++i)
    {
        while(!window.empty()&&(compare(window.back().first, data[i])))
            window.pop_back();

        window.push_back(std::make_pair(data[i], i));
    }

    //slide window
    for(size_t i=offset; i<endSlide; ++i)
    {
        while(!window.empty()&&(window.back().first>=data[i]))
            window.pop_back();

        window.push_back(std::make_pair(data[i], i));

        while(window.front().second<=i-windowSize)
            window.pop_front();

        output[i-offset]=window.front().first;
    }

    //slide out
    for(size_t i=endSlide; i<dataSize; ++i)
    {
        while(window.front().second<=i-windowSize)
            window.pop_front();

        output[i-offset]=window.front().first;
    }
}

template<typename _DataType>
void minimumSlidingWindow(const std::vector<_DataType> &data, std::vector<_DataType> &output, size_t dimX, size_t dimY, size_t windowSize)
{
    if(output.size()<data.size())
        output.resize(data.size());
    slidingWindow<_DataType>(data.data(), output.data(), data.size(), windowSize, std::greater_equal<_DataType>);
}

template<typename _DataType>
std::vector<_DataType> minimumSlidingWindow(const std::vector<_DataType> &data, size_t dimX, size_t dimY, size_t windowSize)
{
    std::vector<_DataType> output(data.size());

    slidingWindow<_DataType>(data.data(), output.data(), data.size(), windowSize, std::greater_equal<_DataType>);
    return output;
}

template<typename _DataType>
std::vector<_DataType> minimumSlidingWindow(_DataType const *data, _DataType *output, size_t dimX, size_t dimY, size_t windowSize)
{
    std::vector<_DataType> output(data.size());

    slidingWindow<_DataType>(data, output, dimX, dimY, windowSize, std::greater_equal<_DataType>);
    return output;
}


template<typename _DataType>
void maximumSlidingWindow(const std::vector<_DataType> &data, std::vector<_DataType> &output, size_t dimX, size_t dimY, size_t windowSize)
{
    if(output.size()<data.size())
        output.resize(data.size());
    slidingWindow<_DataType>(data.data(), output.data(), data.size(), windowSize, std::less_equal<_DataType>);
}

template<typename _DataType>
std::vector<_DataType> maximumSlidingWindow(const std::vector<_DataType> &data, size_t dimX, size_t dimY, size_t windowSize)
{
    std::vector<_DataType> output(data.size());

    slidingWindow<_DataType>(data.data(), output.data(), data.size(), windowSize, std::less_equal<_DataType>);
    return output;
}

template<typename _DataType>
void maximumSlidingWindow(_DataType const *data, _DataType *output, size_t dimX, size_t dimY, size_t windowSize)
{
    slidingWindow<_DataType>(data, output, dimX, dimY, windowSize, std::less_equal<_DataType>());
}

template<typename _DataType, typename _Comparator>
void slidingWindow(_DataType const *data, _DataType *output, size_t dimX, size_t dimY, size_t windowSize, _Comparator compare)
{
    std::deque<std::pair<_DataType, size_t> > window;
    size_t offset=windowSize/2;
    size_t endYSlide=dimY-offset;
    size_t endXSlide=dimX-offset;

    //slide in
    for(size_t j=0; j<offset; j++)
    {
        _DataType const *localData=&data[j*dimX];

        for(size_t i=0; i<offset; ++i)
        {
            while(!window.empty()&&(window.back().first>=localData[i]))
                window.pop_back();

            window.push_back(std::make_pair(localData[i], j));
        }
    }


    //slide window
    size_t dimXOffset=offset;

    for(size_t j=offset; j<endYSlide; j++)
    {
        _DataType const *localData=&data[j*dimX];
        _DataType *localOutput=&output[j*dimX];

        for(size_t i=dimXOffset; i<dimXOffset+windowSize; ++i)
        {
            while(!window.empty()&&(compare(window.back().first, localData[i])))
                window.pop_back();

            window.push_back(std::make_pair(localData[i], j));

            while(window.front().second<=j-windowSize)
                window.pop_front();

            localOutput[j-offset]=window.front().first;
        }

        ++dimXOffset;

        if(dimXOffset>endXSlide)
            continue;
    }

    //slide out
    for(size_t j=endYSlide; j<dimY; j++)
    {
        _DataType *localOutput=&output[j*dimX];

        while(window.front().second<=j-windowSize)
            window.pop_front();

        localOutput[j-offset]=window.front().first;
    }
}

}//namespace cvlib

#endif //_cvlib_slideWindow_h_