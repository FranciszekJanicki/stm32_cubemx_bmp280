#ifndef BMP280_REGISTERS_HPP
#define BMP280_REGISTERS_HPP

#include <cstdint>

#define packed __attribute__((__packed__))

namespace BMP280 {

    enum struct RA : std::uint8_t {
        CALIB00 = 0x88,
        ID = 0xD0,
        RESET = 0xE0,
        STATUS = 0xF3,
        CTRL_MEAS = 0xF4,
        CONFIG = 0xF5,
        PRESS_MSB = 0xF7,
        PRESS_LSB = 0xF8,
        PRESS_XLSB = 0xF9,
        TEMP_MSB = 0xFA,
        TEMP_LSB = 0xFB,
        TEMP_XLSB = 0xFC,
    };

    struct CALIB {
    } packed;

    struct ID {
        std::uint8_t chip_id : 8;
    } packed;

    struct RESET {
        std::uint8_t reset : 8;
    } packed;

    struct STATUS {
        std::uint8_t : 4;
        std::uint8_t measuring : 1;
        std::uint8_t : 2;
        std::uint8_t im_update : 1;
    } packed;

    struct CTRL_MEAS {
        std::uint8_t osrs_t : 3;
        std::uint8_t osrs_p : 3;
        std::uint8_t mode : 2;
    } packed;

    struct CONFIG {
        std::uint8_t spi3w_en : 1;
        std::uint8_t : 1;
        std::uint8_t filter : 3;
        std::uint8_t t_sb : 3;
    } packed;

    struct PRESS {
        std::uint16_t press : 16;
    } packed;

    struct PRESS_X {
        std::uint8_t press_x : 4;
        std::uint8_t : 4;
    } packed;

    struct TEMP {
        std::uint16_t temp : 16;
    } packed;

    struct TEMP_X {
        std::uint8_t temp_x : 4;
        std::uint8_t : 4;
    } packed;

    struct Config {
        CTRL_MEAS ctrl_meas{};
        CONFIG config{};
    };

}; // namespace BMP280

#undef packed

#endif // BMP280_REGISTERS_HPP