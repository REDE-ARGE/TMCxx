/************************************************************
 *  Project : TMCxx
 *  File    : converter
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 ************************************************************/

#ifndef TMCXX_FEATURES_CONVERTER_HPP
#define TMCXX_FEATURES_CONVERTER_HPP

#include "tmcxx/helpers/units.hpp"

#include <algorithm>
#include <cstdint>
#include <numbers>

namespace tmcxx::features {

/**
 * @brief Unit conversion utilities for TMC5160.
 *
 * Converts physical units (RPM, Amperes, etc.) to register values.
 * All methods are constexpr for compile-time evaluation.
 */
class Converter {
  public:
    /**
     * @brief Construct converter with motor parameters.
     *
     * @param f_clk Clock frequency (typically 12 MHz).
     * @param full_steps Full steps per revolution (typically 200).
     * @param r_sense_ohm Sense resistor value.
     */
    constexpr Converter(units::frequency_t f_clk, units::microsteps_t full_steps, units::resistance_t r_sense_ohm)
        : m_clock_frequency{f_clk.raw()}
        , m_full_steps{static_cast<float>(full_steps.raw())}
        , m_r_sense{r_sense_ohm.raw()}
    {
    }

    /**
     * @brief Convert RPM to VMAX register value.
     *
     * @param rpm Velocity in revolutions per minute.
     *
     * @return VMAX register value.
     */
    [[nodiscard]] constexpr uint32_t rpm_to_vmax(units::rpm_t rpm) const noexcept
    {
        const double v_hz{(rpm.raw() * m_full_steps * 256.0) / 60.0};

        constexpr float multiplier{static_cast<float>(1ULL << 24)};

        return static_cast<uint32_t>((v_hz * multiplier) / m_clock_frequency);
    }

    /**
     * @brief Convert current to CS (current scale) register value.
     *
     * @param current Motor current in Amperes.
     *
     * @return CS value (0-31).
     */
    [[nodiscard]] constexpr uint8_t current_to_cs(units::current_t current) const noexcept
    {
        constexpr float v_fs{0.325f};

        constexpr float sqrt2{std::numbers::sqrt2_v<float>};
        const float i_max_rms{(v_fs / m_r_sense) / sqrt2};

        const float ratio{current.raw() / i_max_rms};
        constexpr int32_t low{0};
        constexpr int32_t high{31};

        const int32_t cs_value{std::clamp(static_cast<int32_t>((ratio * 32.0f) - 1.0f), low, high)};

        return static_cast<uint8_t>(cs_value);
    }

    /**
     * @brief Convert VMAX register value to RPM.
     *
     * @param vmax VMAX register value.
     *
     * @return Velocity in RPM.
     */
    [[nodiscard]] constexpr units::rpm_t vmax_to_rpm(uint32_t vmax) const noexcept
    {
        constexpr auto scale_factor{static_cast<float>(1ULL << 24)};

        const float v_hz{(static_cast<float>(vmax) * m_clock_frequency) / scale_factor};
        const float rpm_val{(v_hz * 60.0f) / (m_full_steps * 256.0f)};

        return units::rpm_t{rpm_val};
    }

    /**
     * @brief Convert acceleration to AMAX/DMAX register value.
     *
     * @param accel Acceleration in steps per second squared.
     *
     * @return Register value (clamped to 1-65535).
     */
    [[nodiscard]] constexpr uint32_t accel_to_register(units::acceleration_t accel) const noexcept
    {
        const float a_hz_s{accel.raw()};
        constexpr auto constant_factor{static_cast<float>(1ULL << 41)};

        const float numerator{a_hz_s * constant_factor};
        const float denominator{m_clock_frequency * m_clock_frequency};

        const float result{numerator / denominator};

        constexpr float min_val{1.0f};
        constexpr float max_val{65'535.0f};

        const float result_clamp{std::clamp(result, min_val, max_val)};

        return static_cast<uint32_t>(result_clamp);
    }

    /**
     * @brief Convert time duration to TZEROWAIT register value.
     *
     * @param duration Wait time in seconds.
     *
     * @return TZEROWAIT register value.
     */
    [[nodiscard]] constexpr uint32_t duration_to_tzerowait(units::time_duration_t duration) const noexcept
    {
        const float clocks{duration.raw() * m_clock_frequency};

        constexpr auto scaling_factor{static_cast<float>(1ULL << 9)};
        const float reg_val{clocks / scaling_factor};

        constexpr float min_val{0.0f};
        constexpr float max_val{65'535.0f};

        const auto clamped_value{std::clamp(reg_val, min_val, max_val)};

        return static_cast<uint32_t>(clamped_value);
    }

  private:
    float m_clock_frequency{};
    float m_full_steps{};
    float m_r_sense{};
};
} // namespace tmcxx::features

#endif // TMCXX_FEATURES_CONVERTER_HPP
