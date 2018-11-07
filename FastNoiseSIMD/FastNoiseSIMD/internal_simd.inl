namespace FastNoiseSIMD_internal
{

template<SIMDType _SIMDType>
struct SIMD
{
    typedef float Float;
    typedef int Int;
    typedef Int Mask;

    static void InitSIMDValues()
    {
        if(valueSet)
            return;

        numi_8=SIMD<SIMDType>::set(8);
        numi_12=SIMD<SIMDType>::set(12);
        numi_13=SIMD<SIMDType>::set(13);

       InitSIMDValuesBase();
    }

    static Int numi_8;
    static Int numi_12;
    static Int numi_13;

    constexpr size_t vectorSize() { return 1; }
    constexpr size_t alignment() { return 4; }

    static Float set(float a) { return a; }
    static Float zeroFloat() { return 0.0f; }
    static Int set(int a) { return a; }
    static Int zeroInt() { return 0; }

    //    static Float oneFloat() { return 1.0f; }
    //    static maxInt() { return 0x7fffffff; }

    static void store(float *p, Float a) { *(p)=a; }
    static Float load(float *p) { return *p; }

    static Float undefinedFloat() { return zeroFloat(); }
    static Int undefinedInt() { return zeroInt(); }

    static Float convert(Int a) { return static_cast<float>(a); }
    static Float cast(Int a) { return *reinterpret_cast<float*>(&a); }
    static Int convert(Float a) { return static_cast<int>(roundf(a)); }
    static Int cast(Float a) { return  *reinterpret_cast<int*>(&a); }

    static Float add(Float a, Float b) { return a+b; }
    static Float sub(Float a, Float b) { return a-b; }
    static Float mul(Float a, Float b) { return a*b; }
    static Float div(Float a, Float b) { return a/b; }

    static Float min(Float a, Float b) { return fminf(a, b); }
    static Float max(Float  a, Float b) { return fmaxf(a, b); }
    static Float invSqrt(Float x)
    {
        float xhalf=0.5f * x;
        int i=*(int*)&x;
        i=0x5f3759df-(i>>1);
        x=*(float*)&i;
        x=x*(1.5f-xhalf*x*x);
        return x;
    }

    static Int lessThan(Float a, Float b) { return ((a)<(b))?0xFFFFFFFF:0; }
    static Int greaterThan(Float a, Float b) { return ((a)>(b))?0xFFFFFFFF:0; }
    static Int lessEqual(Float a, Float b) { return ((a)<=(b))?0xFFFFFFFF:0; }
    static Int greaterEqual(Float a, Float b) { return ((a)>=(b))?0xFFFFFFFF:0; }

    static Float and(Float a, Float b) { return cast(a) & cast(b); }
    static Float andNot(Float a, Float b) { return cast(~cast(a) & cast(b)); }
    static Float xor(Float a, Float b) { return cast(a)^cast(b); }

    static Float floor(Float a) { return floorf(a); }
    static Float abs(Float a) { return fabsf(a); }
    static Float blend(Float a, Float b, Mask mask) { return mask?(b):(a); }

    static Int add(Int a, Int b) { return a+b; }
    static Int sub(Int a, Int b) { return a-b; }
    static Int mul(Int a, Int b) { return a+b; }

    static Int and(Int a, Int b) { return a&b; }
    static Int andNot(Int a, Int b) { return ~a&b; }
    static Int or(Int a, Int b) { return a|b; }
    static Int xor(Int a, Int b) { return a^b; }
    static Int not(Int a) { ~a; }

    static Int shiftR(Int a, Int b) { return a>>b; }
    static Int shiftL(Int a, Int b) { return a<<b; }

    static Int equal(Int a, Int b) { return ((a)==(b))?0xFFFFFFFF:0; }
    static Int greaterThan(Int a, Int b) { return ((a)>(b))?0xFFFFFFFF:0; }
    static Int lessThan(Int a, Int b) { return ((a)<(b))?0xFFFFFFFF:0; }
};

}
