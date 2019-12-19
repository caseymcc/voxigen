#ifndef RANDOMCOLOR_H
#define RANDOMCOLOR_H
#include <cmath>
#include <vector>
#include <string>
#include <tuple>
#include <map>
namespace RandomColor {
enum LUMINOSITY {
    RANDOM,
    BRIGHT,
    LIGHT,
    DARK
};
enum HUENAMES {
    UNSET,
    MONOCHROME,
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE,
    PINK
};
struct Options {
    int        hue;
    HUENAMES   hue_name;
    size_t     seed;
    LUMINOSITY luminosity;
};
using rangetype = std::tuple<int, int>;
class RandomColorGenerator {
    private:
    enum COLORDICT {
        HEU_RANGE,
        LOW_BOUNDS,
        STATURATION_RANGE,
        BRIGHTNESS_RANGE
    };
    using Colormap = std::map<HUENAMES, std::map<COLORDICT, std::vector<rangetype>>>;
    static Colormap colorDictionary;
    //--------------------------------------------------------------------------
    private:
    //--------------------------------------------------------------------------
    static double Random() {
        return ((double) rand() / (RAND_MAX));
    }
    //--------------------------------------------------------------------------
    rangetype getSaturationRange(int hue) {
        return getColorInfo(hue)[COLORDICT::STATURATION_RANGE][0];
    }
    //--------------------------------------------------------------------------
    static std::tuple<int, int> getHueRange(int colorInput);
    //--------------------------------------------------------------------------
    static std::tuple<int, int> getHueRange_s(HUENAMES colorInput);
    //--------------------------------------------------------------------------
    int randomWithin(const std::tuple<int, int> & range);
    //--------------------------------------------------------------------------
    int pickHue(const Options & options);
    //--------------------------------------------------------------------------
    std::map<COLORDICT, std::vector<rangetype>> getColorInfo(int hue);
    //--------------------------------------------------------------------------
    int pickSaturation(int hue, const Options & options);
    //--------------------------------------------------------------------------
    int getMinimumBrightness(int H, int S);
    //--------------------------------------------------------------------------
    int pickBrightness(int H, int S, const Options & options);
    //--------------------------------------------------------------------------
    std::tuple<int, int, int> HSVtoRGB(std::tuple<int, int, int> hsv);

    public:
    Options options;
    static Colormap loadColorBounds();
    //--------------------------------------------------------------------------
    RandomColorGenerator() : options() {
    }
    RandomColorGenerator(const Options & opt) : options(opt) {
    }
    //--------------------------------------------------------------------------
    std::vector<std::tuple<int, int, int>> randomColors(int count = 1);
    //--------------------------------------------------------------------------
    std::tuple<int, int, int> randomColorRGB();
    //--------------------------------------------------------------------------
};
};
#endif