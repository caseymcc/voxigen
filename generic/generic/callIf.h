namespace generic
{

template<typename _Function, typename=int>
struct callIf
{
    template<typename _Ret, typename ..._Args>
    void operator()(_Args ...args)
    {}
};

template <typename _Function>
struct callIf<_Function, decltype(&_Function, 0)>
{
    template<typename ..._Args>
    auto operator()(_Args ...args) -> decltype(_Function(args...))
    {
        return _Function(args...);
    }
};

}//namespace generic
