#ifndef BMP280_CONFIG_HPP
#define BMP280_CONFIG_HPP

#include <cstdint>

namespace BMP280 {

    enum struct Resolution : std::uint8_t {
        ULTRALOWPOWER = 1,
        LOWPOWER = 2,
        STANDARD = 3,
        HIGHRES = 4,
        ULTRAHIGHRES = 5,
    };

    enum struct Temperature : std::uint8_t {
        TEMPERATURE_16BIT = 1,
        TEMPERATURE_17BIT = 2,
        TEMPERATURE_18BIT = 3,
        TEMPERATURE_19BIT = 4,
        TEMPERATURE_20BIT = 5,
    };

    enum struct Mode : std::uint8_t {
        SLEEP = 0,
        FORCED = 1,
        NORMAL = 3,
    };

    enum struct Standby : std::uint8_t {
        MS_0_5 = 0,
        MS_10 = 6,
        MS_20 = 7,
        MS_62_5 = 1,
        MS_125 = 2,
        MS_250 = 3,
        MS_500 = 4,
        MS_1000 = 5
    };

    enum struct Filter : std::uint8_t {
        FILTER_OFF = 0,
        FILTER_X2 = 1,
        FILTER_X4 = 2,
        FILTER_X8 = 3,
        FILTER_X16 = 4,
    };

    enum struct Digit : std::uint8_t {
        DIG_T1 = 0x88,
        DIG_T2 = 0x8A,
        DIG_T3 = 0x8C,
        DIG_P1 = 0x8E,
        DIG_P2 = 0x90,
        DIG_P3 = 0x92,
        DIG_P4 = 0x94,
        DIG_P5 = 0x96,
        DIG_P6 = 0x98,
        DIG_P7 = 0x9A,
        DIG_P8 = 0x9C,
        DIG_P9 = 0x9E,
    };

    constexpr std::uint8_t MEASURING{1 << 3};
    constexpr std::uint8_t DEVICE_ID{0x75};

}; // namespace BMP280

#endif // BMP280_CONFIG_HPP