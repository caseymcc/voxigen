#include "randomcolor.h"
#include <functional>
#include <cassert>
using namespace RandomColor;
RandomColorGenerator::Colormap RandomColorGenerator::colorDictionary = [=]() -> RandomColorGenerator::Colormap {
    return RandomColorGenerator::loadColorBounds();
}();

RandomColorGenerator::Colormap RandomColorGenerator::loadColorBounds() {
    Colormap cmap_tmp;
    auto defineColor = [=, &cmap_tmp](HUENAMES name, const rangetype & hueRange, const std::vector<rangetype> & lowerBounds) {
        int sMin = std::get<0>(lowerBounds[0]);
        int sMax = std::get<0>(lowerBounds[lowerBounds.size() - 1]);
        int bMin = std::get<1>(lowerBounds[lowerBounds.size() - 1]);
        int bMax = std::get<1>(lowerBounds[0]);
        std::map<COLORDICT, std::vector<rangetype>> tmp;
        tmp[COLORDICT::HEU_RANGE]         = std::vector<rangetype>({hueRange});
        tmp[COLORDICT::LOW_BOUNDS]        = lowerBounds;
        tmp[COLORDICT::STATURATION_RANGE] = std::vector<rangetype>({std::make_tuple(sMin, sMax)});
        tmp[COLORDICT::BRIGHTNESS_RANGE]  = std::vector<rangetype>({std::make_tuple(bMin, bMax)});
        cmap_tmp[name]                    = tmp;
    };
    defineColor(
        HUENAMES::MONOCHROME,
        std::make_tuple(0, 0),
        {std::make_tuple(0, 0), std::make_tuple(100, 0)});

    defineColor(
        HUENAMES::RED,
        std::make_tuple(-26, 18),
        {std::make_tuple(20, 100), std::make_tuple(30, 92), std::make_tuple(40, 89), std::make_tuple(50, 85), std::make_tuple(60, 78), std::make_tuple(70, 70), std::make_tuple(80, 60), std::make_tuple(90, 55), std::make_tuple(100, 50)});

    defineColor(
        HUENAMES::ORANGE,
        std::make_tuple(19, 46),
        {std::make_tuple(20, 100), std::make_tuple(30, 93), std::make_tuple(40, 88), std::make_tuple(50, 86), std::make_tuple(60, 85), std::make_tuple(70, 70), std::make_tuple(100, 70)});

    defineColor(
        HUENAMES::YELLOW,
        std::make_tuple(47, 62),
        {std::make_tuple(25, 100), std::make_tuple(40, 94), std::make_tuple(50, 89), std::make_tuple(60, 86), std::make_tuple(70, 84), std::make_tuple(80, 82), std::make_tuple(90, 80), std::make_tuple(100, 75)});

    defineColor(
        HUENAMES::GREEN,
        std::make_tuple(63, 178),
        {std::make_tuple(30, 100), std::make_tuple(40, 90), std::make_tuple(50, 85), std::make_tuple(60, 81), std::make_tuple(70, 74), std::make_tuple(80, 64), std::make_tuple(90, 50), std::make_tuple(100, 40)});

    defineColor(
        HUENAMES::BLUE,
        std::make_tuple(179, 257),
        {std::make_tuple(20, 100), std::make_tuple(30, 86), std::make_tuple(40, 80), std::make_tuple(50, 74), std::make_tuple(60, 60), std::make_tuple(70, 52), std::make_tuple(80, 44), std::make_tuple(90, 39), std::make_tuple(100, 35)});

    defineColor(
        HUENAMES::PURPLE,
        std::make_tuple(258, 282),
        {std::make_tuple(20, 100), std::make_tuple(30, 87), std::make_tuple(40, 79), std::make_tuple(50, 70), std::make_tuple(60, 65), std::make_tuple(70, 59), std::make_tuple(80, 52), std::make_tuple(90, 45), std::make_tuple(100, 42)});

    defineColor(
        HUENAMES::PINK,
        std::make_tuple(283, 334),
        {std::make_tuple(20, 100), std::make_tuple(30, 90), std::make_tuple(40, 86), std::make_tuple(60, 84), std::make_tuple(80, 80), std::make_tuple(90, 75), std::make_tuple(100, 73)});
    return cmap_tmp;
}
//------------------------------------------------------------------------------
std::tuple<int, int> RandomColorGenerator::getHueRange(int colorInput) {
    int number = colorInput;
    if (colorInput < 360 && number > 0) {
        return std::make_tuple(number, number);
    }
    return std::make_tuple(0, 360);
}
//------------------------------------------------------------------------------
std::tuple<int, int> RandomColorGenerator::getHueRange_s(HUENAMES colorInput) {
    if (colorDictionary.find(colorInput) != colorDictionary.end()) {
        auto color = colorDictionary[colorInput];
        if (color.find(COLORDICT::HEU_RANGE) != color.end()) {
            return color[COLORDICT::HEU_RANGE][0];
        }
    }
    return std::make_tuple<int, int>(0, 360);
}
//------------------------------------------------------------------------------
int RandomColorGenerator::randomWithin(const std::tuple<int, int> & range) {
    if (options.seed == 0) { //TODO: Modified from nullptr
        return int(floor(std::get<0>(range) + Random() * (std::get<1>(range) + 1 - std::get<0>(range))));
    }
    else {
        //Seeded random algorithm from http://indiegamr.com/generate-repeatable-random-numbers-in-js/
        double max   = std::get<1>(range); //||1
        double min   = std::get<0>(range); //||0
        min          = min < 0 ? 0 : min;
        options.seed = size_t(options.seed * 9301 + 49297) % 233280;
        double rnd   = options.seed / 233280.0;
        auto   r     = floor(min + rnd * (max - min));
        assert(r >= 0);
        return int(r);
    }
}
//------------------------------------------------------------------------------
int RandomColorGenerator::pickHue(const Options & options) {
    rangetype hueRange;
    if (options.hue_name == HUENAMES::UNSET) {
        hueRange = getHueRange(options.hue);
    }
    else {
        hueRange = getHueRange_s(options.hue_name);
    }
    auto hue = randomWithin(hueRange);

    // Instead of storing red as two seperate ranges,
    // we group them, using negative numbers
    if (hue < 0) {
        hue = 360 + hue;
    }
    return hue;
}
//------------------------------------------------------------------------------
std::map<RandomColorGenerator::COLORDICT, std::vector<rangetype>> RandomColorGenerator::getColorInfo(int hue) {
    // Maps red colors to make picking hue easier
    if (hue >= 334 && hue <= 360) {
        hue -= 360;
    }

    for (const auto & color : colorDictionary) {
        auto tmp  = color.second;
        auto tmp2 = tmp[COLORDICT::HEU_RANGE][0];
        if (tmp2 != std::make_tuple(0, 0) &&
            hue >= std::get<0>(tmp2) &&
            hue <= std::get<1>(tmp2)) {
            return color.second;
        }
    }
    assert(false);
    //just to avoid a warning;
    return std::map<COLORDICT, std::vector<rangetype>>();
}

//------------------------------------------------------------------------------
int RandomColorGenerator::pickSaturation(int hue, const Options & options) {
    if (options.luminosity == LUMINOSITY::RANDOM) {
        return randomWithin(std::make_tuple(0, 100));
    }
    if (options.hue_name == HUENAMES::MONOCHROME) {
        return 0;
    }
    auto saturationRange = getSaturationRange(hue);

    int sMin = std::get<0>(saturationRange);
    int sMax = std::get<1>(saturationRange);

    switch (options.luminosity) {
    case LUMINOSITY::BRIGHT:
        sMin = 55;
        break;

    case LUMINOSITY::DARK:
        sMin = sMax - 10;
        break;

    case LIGHT:
        sMax = 55;
        break;
    }

    return randomWithin(std::make_tuple(sMin, sMax));
}
//------------------------------------------------------------------------------
std::tuple<int, int, int> RandomColorGenerator::HSVtoRGB(std::tuple<int, int, int> hsv) {
    // this doesn"t work for the values of 0 and 360
    // here"s the hacky fix
    double h = (double) std::get<0>(hsv);
    if (h == 0) {
        h = 1;
    }
    if (h == 360) {
        h = 359;
    }

    // Rebase the h,s,v values
    h        = h / 360.0;
    double s = std::get<1>(hsv) / 100.0,
           v = std::get<2>(hsv) / 100.0;
    assert(h >= 0 && s >= 0 && v >= 0);
    int h_i = int(floor(h * 6));
    double
        f = h * 6 - h_i,
        p = v * (1 - s),
        q = v * (1 - f * s),
        t = v * (1 - (1 - f) * s),
        r = 256,
        g = 256,
        b = 256;

    switch (h_i) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
        r = v;
        g = p;
        b = q;
        break;
    }
    return std::make_tuple(int(floor(r * 255)), int(floor(g * 255)), int(floor(b * 255)));
}

//------------------------------------------------------------------------------
int RandomColorGenerator::pickBrightness(int H, int S, const Options & options) {
    int bMin = getMinimumBrightness(H, S);
    int bMax = 100;

    switch (options.luminosity) {
    case LUMINOSITY::DARK:
        bMax = bMin + 20;
        break;

    case LUMINOSITY::LIGHT:
        bMin = (bMax + bMin) / 2;
        break;

    case LUMINOSITY::RANDOM:
        bMin = 0;
        bMax = 100;
        break;
    }
    return randomWithin(std::make_tuple(bMin, bMax));
}
//------------------------------------------------------------------------------
int RandomColorGenerator::getMinimumBrightness(int H, int S) {
    auto lowerBounds = getColorInfo(H)[COLORDICT::LOW_BOUNDS];
    for (int i = 0; i < int(lowerBounds.size()) - 1; i++) {
        int s1 = std::get<0>(lowerBounds[i]),
            v1 = std::get<1>(lowerBounds[i]);

        int s2 = std::get<0>(lowerBounds[i + 1]),
            v2 = std::get<1>(lowerBounds[i + 1]);

        if (S >= s1 && S <= s2) {
            int m = (v2 - v1) / (s2 - s1),
                b = v1 - m * s1;
            return m * S + b;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
std::vector<std::tuple<int, int, int>> RandomColorGenerator::randomColors(int count) {
    // Check if we need to generate multiple colors
    std::vector<std::tuple<int, int, int>> colors;
    for (auto i = 0; i < count; i++) {
        colors.push_back(randomColorRGB());
    }
    return colors;
};
//------------------------------------------------------------------------------
std::tuple<int, int, int> RandomColorGenerator::randomColorRGB() {
    // Check if we need to generate multiple colors

    int H, S, B;
    // First we pick a hue (H)
    H = pickHue(options);

    // Then use H to determine saturation (S)
    S = pickSaturation(H, options);

    // Then use S and H to determine brightness (B).
    B = pickBrightness(H, S, options);
    options.seed += 1;
    return HSVtoRGB(std::make_tuple(H, S, B));
};
//------------------------------------------------------------------------------
