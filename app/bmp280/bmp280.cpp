#include "bmp280.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>

namespace BMP280 {

    BMP280::BMP280(SPIDevice&& spi_device, Config const& config) noexcept :
        spi_device_{std::forward<SPIDevice>(spi_device)}
    {
        this->initialize(config);
    }

    BMP280::~BMP280() noexcept
    {
        this->deinitialize();
    }

    std::optional<float> BMP280::get_temperature() noexcept
    {
        if (!this->initialized_) {
            return std::optional<float>{std::nullopt};
        }

        this->set_mode(Mode::FORCED);
        while (this->get_mode() == Mode::SLEEP) {
        }

        std::int32_t adc_T = this->get_temp_registers().temp >> 4;
        std::int32_t var1 = ((((adc_T >> 3) - ((std::int32_t)this->t1_ << 1))) * ((std::int32_t)this->t2_)) >> 11;
        std::int32_t var2 =
            (((((adc_T >> 4) - ((std::int32_t)this->t1_)) * ((adc_T >> 4) - ((std::int32_t)this->t1_))) >> 12) *
             ((std::int32_t)this->t3_)) >>
            14;
        this->t_fine_ = var1 + var2;

        return std::optional<float>{((this->t_fine_ * 5 + 128) >> 8) / 100};
    }

    std::optional<std::int32_t> BMP280::get_pressure() noexcept
    {
        if (!this->initialized_) {
            return std::optional<std::int32_t>{std::nullopt};
        }

        [[maybe_unused]] this->get_temperature();

        std::int32_t adc_P = this->get_press_registers().press >> 4;
        std::int64_t var1 = ((int64_t)this->t_fine_) - 128000;
        std::int64_t var2 = var1 * var1 * (int64_t)this->p6_;
        var2 += ((var1 * (int64_t)this->p5_) << 17);
        var2 += (((int64_t)this->p4_) << 35);
        var1 = ((var1 * var1 * (int64_t)this->p3_) >> 8) + ((var1 * (int64_t)this->p2_) << 12);
        var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)this->p1_) >> 33;
        assert(var1 != 0);

        std::int64_t p = 1048576 - adc_P;
        p = (((p << 31) - var2) * 3125) / var1;
        var1 = (((int64_t)this->p9_) * (p >> 13) * (p >> 13)) >> 25;
        var2 = (((int64_t)this->p8_) * p) >> 19;

        return std::optional<std::int32_t>{((p + var1 + var2) >> 8) + (((int64_t)this->p7_) << 4) / 256};
    }

    std::optional<float> BMP280::get_altitude(std::int32_t const sea_level_pa) noexcept
    {
        return this->get_pressure().transform([sea_level_pa](std::int32_t const pressure) {
            return 44330.0F * (1.0F - std::pow(pressure / sea_level_pa, 0.1903F));
        });
    }

    std::uint8_t BMP280::read_byte(std::uint8_t const reg_address) const noexcept
    {
        return this->spi_device_.read_byte(reg_address);
    }

    void BMP280::write_byte(std::uint8_t const reg_address, std::uint8_t const byte) const noexcept
    {
        this->spi_device_.write_byte(reg_address, byte);
    }

    void BMP280::initialize(Config const& config) noexcept
    {
        if (this->is_valid_device_id()) {
            this->device_reset();
            this->set_config_register(config.config);
            this->set_ctrl_meas_register(config.ctrl_meas);
            this->initialized_ = true;
        }
    }

    void BMP280::deinitialize() noexcept
    {
        if (this->is_valid_device_id()) {
            this->device_reset();
            this->initialized_ = false;
        }
    }

    void BMP280::read_digits() noexcept
    {
        this->t1_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_T1)));
        this->t2_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_T2)));
        this->t3_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_T3)));
        this->p1_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P1)));
        this->p2_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P2)));
        this->p3_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P3)));
        this->p4_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P4)));
        this->p5_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P5)));
        this->p6_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P6)));
        this->p7_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P7)));
        this->p8_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P8)));
        this->p9_ = std::bit_cast<std::int16_t>(this->read_bytes<2UL>(std::to_underlying(Digit::DIG_P9)));
    }

    void BMP280::set_mode(Mode const mode) const noexcept
    {
        auto ctrl_meas = this->get_ctrl_meas_register();
        ctrl_meas.mode = std::to_underlying(mode);
        this->set_ctrl_meas_register(ctrl_meas);
    }

    Mode BMP280::get_mode() const noexcept
    {
        return std::bit_cast<Mode>(this->get_ctrl_meas_register().mode);
    }

    std::uint8_t BMP280::get_device_id() const noexcept
    {
        return std::bit_cast<std::uint8_t>(this->get_id_register());
    }

    bool BMP280::is_valid_device_id() const noexcept
    {
        return this->get_device_id() == DEVICE_ID;
    }

    void BMP280::device_reset() const noexcept
    {
        this->set_reset_register(RESET{.reset = 0x00});
    }

    CALIB BMP280::get_calib_register(std::uint8_t const num) const noexcept
    {
        return std::bit_cast<CALIB>(this->read_byte(std::to_underlying(RA::CALIB00) + num));
    }

    void BMP280::set_calib_register(std::uint8_t const num, CALIB const calib) const noexcept
    {
        this->write_byte(std::to_underlying(RA::CALIB00) + num, std::bit_cast<std::uint8_t>(calib));
    }

    RESET BMP280::get_reset_register() const noexcept
    {
        return std::bit_cast<RESET>(this->read_byte(std::to_underlying(RA::RESET)));
    }

    void BMP280::set_reset_register(RESET const reset) const noexcept
    {
        this->write_byte(std::to_underlying(RA::RESET), std::bit_cast<std::uint8_t>(reset));
    }

    CONFIG BMP280::get_config_register() const noexcept
    {
        return std::bit_cast<CONFIG>(this->read_byte(std::to_underlying(RA::CONFIG)));
    }

    void BMP280::set_config_register(CONFIG const config) const noexcept
    {
        this->write_byte(std::to_underlying(RA::CONFIG), std::bit_cast<std::uint8_t>(config));
    }

    void BMP280::set_ctrl_meas_register(CTRL_MEAS const ctrl_meas) const noexcept
    {
        this->write_byte(std::to_underlying(RA::CTRL_MEAS), std::bit_cast<std::uint8_t>(ctrl_meas));
    }

    ID BMP280::get_id_register() const noexcept
    {
        return std::bit_cast<ID>(this->read_byte(std::to_underlying(RA::ID)));
    }

    STATUS BMP280::get_status_register() const noexcept
    {
        return std::bit_cast<STATUS>(this->read_byte(std::to_underlying(RA::STATUS)));
    }

    CTRL_MEAS BMP280::get_ctrl_meas_register() const noexcept
    {
        return std::bit_cast<CTRL_MEAS>(this->read_byte(std::to_underlying(RA::CTRL_MEAS)));
    }

    PRESS BMP280::get_press_registers() const noexcept
    {
        return std::bit_cast<PRESS>(this->read_bytes<2UL>(std::to_underlying(RA::PRESS_MSB)));
    }

    PRESS_X BMP280::get_press_x_register() const noexcept
    {
        return std::bit_cast<PRESS_X>(this->read_byte(std::to_underlying(RA::PRESS_XLSB)));
    }

    TEMP BMP280::get_temp_registers() const noexcept
    {
        return std::bit_cast<TEMP>(this->read_bytes<2UL>(std::to_underlying(RA::TEMP_MSB)));
    }

    TEMP_X BMP280::get_temp_x_register() const noexcept
    {
        return std::bit_cast<TEMP_X>(this->read_byte(std::to_underlying(RA::TEMP_XLSB)));
    }

}; // namespace BMP280