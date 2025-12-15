/************************************************************
 *  Project : rede-firmware
 *  File    : tmc_register_builder
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 28.11.2025
 ************************************************************/

#ifndef REDE_FIRMWARE_TMC_REGISTER_BUILDER_HPP
#define REDE_FIRMWARE_TMC_REGISTER_BUILDER_HPP

#include "tmcxx/helpers/units.hpp"
#include "tmcxx/tmc5160.hpp"

namespace tmcxx::helpers::builder {

/**
 * @brief Fluent builder pattern for TMC5160 configuration.
 *
 * @tparam TSpi SPI device type satisfying hal::SpiDevice concept.
 *
 * @code
 * auto motor = TMC5160Builder{spi}
 *     .clock_frequency(12.0_MHz)
 *     .sense_resistor(75.0_mOhm)
 *     .run_current(1.5_A)
 *     .build();
 * @endcode
 */
template<core::concepts::SpiDevice TSpi>
class TMC5160Builder {
  public:
    /**
     * @brief Construct builder with the SPI device.
     *
     * @param spi_device Reference to an SPI device.
     */
    explicit constexpr TMC5160Builder(TSpi& spi_device)
        : m_spi{spi_device}
    {
    }

    /**
     * @brief Construct builder with the SPI device and initial settings.
     *
     * @param spi_device Reference to an SPI device.
     * @param settings Initial configuration settings.
     */
    constexpr TMC5160Builder(TSpi& spi_device, const typename TMC5160<TSpi>::Settings& settings)
        : m_spi{spi_device}
        , m_config{settings}
    {
    }

    /**
     * @brief Set clock frequency (default: 12 MHz).
     */
    constexpr TMC5160Builder& clock_frequency(units::frequency_t val) noexcept
    {
        m_config.f_clk_hz = val;
        return *this;
    }

    /**
     * @brief Set motor full steps per revolution (default: 200).
     */
    constexpr TMC5160Builder& full_steps(units::microsteps_t val) noexcept
    {
        m_config.full_steps = val;
        return *this;
    }

    /**
     * @brief Set sense resistor value (default: 75 mOhm).
     */
    constexpr TMC5160Builder& sense_resistor(units::resistance_t val) noexcept
    {
        m_config.r_sense = val;
        return *this;
    }

    /**
     * @brief Set motor run current in Amperes.
     */
    constexpr TMC5160Builder& run_current(units::current_t val) noexcept
    {
        m_config.run_current = val;
        return *this;
    }

    /**
     * @brief Set motor hold current in Amperes.
     */
    constexpr TMC5160Builder& hold_current(units::current_t val) noexcept
    {
        m_config.hold_current = val;
        return *this;
    }

    /**
     * @brief Set delay before switching to hold current (0-15).
     */
    constexpr TMC5160Builder& hold_delay(uint8_t val) noexcept
    {
        m_config.hold_delay = val;
        return *this;
    }

    /**
     * @brief Set power down delay (0-255).
     */
    constexpr TMC5160Builder& power_down_delay(uint8_t val) noexcept
    {
        m_config.power_down_delay = val;
        return *this;
    }

    /**
     * @brief Set motor start velocity.
     */
    constexpr TMC5160Builder& v_start(units::rpm_t val) noexcept
    {
        m_config.v_start = val;
        return *this;
    }

    /**
     * @brief Set motor maximum velocity.
     */
    constexpr TMC5160Builder& v_max(units::rpm_t val) noexcept
    {
        m_config.v_max = val;
        return *this;
    }

    /**
     * @brief Set motor stop velocity.
     */
    constexpr TMC5160Builder& v_stop(units::rpm_t val) noexcept
    {
        m_config.v_stop = val;
        return *this;
    }

    /**
     * @brief Set velocity for ramp transition (V1).
     */
    constexpr TMC5160Builder& v_transition(units::rpm_t val) noexcept
    {
        m_config.v_1 = val;
        return *this;
    }

    /**
     * @brief Set initial acceleration (A1).
     */
    constexpr TMC5160Builder& a_start(units::acceleration_t val) noexcept
    {
        m_config.a_1 = val;
        return *this;
    }

    /**
     * @brief Set maximum acceleration (AMAX).
     */
    constexpr TMC5160Builder& a_max(units::acceleration_t val) noexcept
    {
        m_config.a_max = val;
        return *this;
    }

    /**
     * @brief Set maximum deceleration (DMAX).
     */
    constexpr TMC5160Builder& d_max(units::acceleration_t val) noexcept
    {
        m_config.d_max = val;
        return *this;
    }

    /**
     * @brief Set final deceleration (D1).
     */
    constexpr TMC5160Builder& d_stop(units::acceleration_t val) noexcept
    {
        m_config.d_1 = val;
        return *this;
    }

    /**
     * @brief Enable or disable StealthChop mode.
     */
    constexpr TMC5160Builder& stealth_chop_enabled(bool value) noexcept
    {
        m_config.stealth_chop_enabled = value;

        return *this;
    }

    /**
     * @brief Set TOFF (off-time setting, 0-15).
     */
    constexpr TMC5160Builder& toff(uint8_t val) noexcept
    {
        m_config.toff = val;
        return *this;
    }

    /**
     * @brief Set chopper hysteresis parameters.
     */
    constexpr TMC5160Builder& hysteresis(uint8_t start, int8_t end) noexcept
    {
        m_config.hstrt = start;
        m_config.hend = end;
        return *this;
    }

    /**
     * @brief Set blank time (0-3).
     */
    constexpr TMC5160Builder& blank_time(uint8_t val) noexcept
    {
        m_config.tbl = val;
        return *this;
    }

    /**
     * @brief Build and return the configuration settings.
     *
     * @return Configured Settings object.
     */
    [[nodiscard]] constexpr TMC5160<TSpi>::Settings build_settings() const noexcept
    {
        return m_config;
    }

    /**
     * @brief Build and return a configured TMC5160 instance.
     *
     * @return Configured TMC5160 object.
     */
    [[nodiscard]] constexpr TMC5160<TSpi> build() const noexcept
    {
        return TMC5160<TSpi>{m_spi, m_config};
    }

  private:
    TSpi& m_spi;
    TMC5160<TSpi>::Settings m_config;
};

} // namespace tmcxx::helpers::builder

#endif // REDE_FIRMWARE_TMC_REGISTER_BUILDER_HPP
