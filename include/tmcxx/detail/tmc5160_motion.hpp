/************************************************************
 *  Project : TMCxx
 *  File    : tmc5160_motion
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 08.12.2025
 ************************************************************/

#ifndef TMCXX_TMC5160_MOTION_HPP
#define TMCXX_TMC5160_MOTION_HPP

#include "tmcxx/chips/tmc5160_registers.hpp"
#include "tmcxx/features/converter.hpp"
#include "tmcxx/helpers/error.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>

namespace tmcxx::detail {

constexpr uint32_t vactual_sign_bit{0x800000U};
constexpr uint32_t vactual_sign_extension{0xFF000000U};
/**
 * @brief Motion control interface for TMC5160.
 *
 * Handles velocity, position, ramp, and acceleration settings.
 *
 * @tparam Bus Bus type (e.g., TMC5160Bus<TSpi>).
 */
template<typename Bus>
class TMC5160Motion {
  public:
    /**
     * @brief Construct motion controller.
     *
     * @param bus Reference to bus (must outlive this object).
     * @param converter Reference to unit converter.
     */
    TMC5160Motion(Bus& bus, features::Converter& converter)
        : m_bus{bus}
        , m_converter{converter}
    {
    }

    /**
     * @brief Deleted copy ctor and copy assign operator.
     *
     * @note not-declared move assign operator and move ctor.
     */
    TMC5160Motion(const TMC5160Motion&) = delete;
    TMC5160Motion& operator=(const TMC5160Motion&) = delete;

    /**
     * @brief Stop motor immediately by setting VMAX to zero.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> stop()
    {
        constexpr uint32_t zero_val{0U};
        return m_bus.template write<chip::tmc5160::VMAX>(zero_val);
    }

    /**
     * @brief Continuously rotate motor at specified velocity.
     *
     * @param velocity Target velocity in RPM (negative = reverse).
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> rotate(units::rpm_t velocity)
    {
        constexpr float zero_val{0.f};
        const auto ramp_res{(zero_val <= velocity.raw()) ? set_ramp_mode(chip::tmc5160::RampModeType::VELOCITY_POS)
                                                         : set_ramp_mode(chip::tmc5160::RampModeType::VELOCITY_NEG)};

        return ramp_res.and_then([this, velocity]() -> helpers::result_t<void> {
            const units::rpm_t abs_vel{(velocity.raw() < zero_val) ? std::negate{}(velocity) : velocity};
            return set_max_velocity(abs_vel);
        });
    }

    /**
     * @brief Set global current scaling factor.
     *
     * @param factor Scaling ratio (0.0 = 0%, 1.0 = 100%).
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_global_scaling(units::factor_t factor)
    {
        using namespace units::literals;

        constexpr auto min_factor{0.0_factor};
        constexpr auto max_factor{1.0_factor};

        const auto safe_factor{std::clamp(factor, min_factor, max_factor)};

        constexpr auto full_scale_reference{256.0_factor};

        const auto register_val{static_cast<uint32_t>(safe_factor.raw() * full_scale_reference.raw())};

        return m_bus.template write<chip::tmc5160::GLOBAL_SCALER>(register_val);
    }

    /**
     * @brief Move motor to absolute position.
     *
     * @param step Target position in microsteps.
     * @param max_speed Maximum velocity during movement.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> move_to(units::microsteps_t step, units::rpm_t max_speed)
    {
        return set_ramp_mode(chip::tmc5160::RampModeType::POSITIONING)
            .and_then([this, max_speed] {
                return set_start_speed(max_speed);
            })
            .and_then([this, step] {
                return m_bus.template write<chip::tmc5160::XTARGET>(static_cast<uint32_t>(step.raw()));
            });
    }

    /**
     * @brief Set ramp generator mode.
     *
     * @param mode Positioning, velocity positive/negative, or hold.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_ramp_mode(chip::tmc5160::RampModeType mode)
    {
        return m_bus.template write<chip::tmc5160::RAMPMODE>(static_cast<uint32_t>(mode));
    }

    /**
     * @brief Enable or disable StealthChop mode.
     *
     * @param state True to enable, false to disable.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_stealth_chop(bool state)
    {
        constexpr uint32_t disable_value{0U};
        constexpr uint32_t enable_value{1U};

        const uint32_t write_value{state ? enable_value : disable_value};

        return m_bus.template write_field<chip::tmc5160::CHOPCONF::chm_t>(write_value);
    }

    /**
     * @brief Set maximum velocity (VMAX register).
     *
     * @param max_velocity Maximum velocity in RPM.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_max_velocity(units::rpm_t max_velocity)
    {
        const auto vmax_reg{m_converter.rpm_to_vmax(max_velocity)};
        return m_bus.template write<chip::tmc5160::VMAX>(vmax_reg);
    }

    /**
     * @brief Set motor start velocity (VSTART register).
     *
     * @param start_speed Start velocity in RPM.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_start_speed(units::rpm_t start_speed)
    {
        return m_bus.template write<chip::tmc5160::VSTART>(m_converter.rpm_to_vmax(start_speed));
    }

    /**
     * @brief Set ramp transition velocity (V1 register).
     *
     * @param speed Velocity threshold between A1/AMAX acceleration.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_ramp_transition_velocity(units::rpm_t speed)
    {
        return m_bus.template write<chip::tmc5160::V1>(m_converter.rpm_to_vmax(speed));
    }

    /**
     * @brief Set stop velocity (VSTOP register).
     *
     * @param speed Velocity near standstill (minimum 1).
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_stop_velocity(units::rpm_t speed)
    {
        constexpr uint32_t min_value{1U};
        const auto val{std::max(min_value, m_converter.rpm_to_vmax(speed))};
        return m_bus.template write<chip::tmc5160::VSTOP>(val);
    }

    /**
     * @brief Set symmetric acceleration and deceleration.
     *
     * @param acceleration Acceleration value.
     * @param decel Deceleration value.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_linear_acceleration(
        units::acceleration_t acceleration, units::acceleration_t decel)
    {
        const uint32_t a_val{m_converter.accel_to_register(acceleration)};
        constexpr uint32_t min_value{1U};
        constexpr uint32_t max_value{65'535U};
        const uint32_t d_val{std::clamp(m_converter.accel_to_register(decel), min_value, max_value)};

        return m_bus.template write<chip::tmc5160::AMAX>(a_val)
            .and_then([this, d_val]() {
                return m_bus.template write<chip::tmc5160::DMAX>(d_val);
            })
            .and_then([this, d_val]() {
                return m_bus.template write<chip::tmc5160::D1>(d_val);
            })
            .and_then([this, a_val]() {
                return m_bus.template write<chip::tmc5160::A1>(a_val);
            });
    }

    /**
     * @brief Set S-curve acceleration with four phases.
     *
     * @param start_accel Initial acceleration (A1).
     * @param max_accel Maximum acceleration (AMAX).
     * @param max_decel Maximum deceleration (DMAX).
     * @param stop_decel Final deceleration (D1).
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_advanced_acceleration(units::acceleration_t start_accel,
        units::acceleration_t max_accel,
        units::acceleration_t max_decel,
        units::acceleration_t stop_decel)
    {
        constexpr uint32_t min_d1_value{1U};
        constexpr uint32_t max_d1_value{65'535U};

        const uint32_t clamped_d1_val{
            std::clamp(m_converter.accel_to_register(stop_decel), min_d1_value, max_d1_value)};

        return m_bus.template write<chip::tmc5160::A1>(m_converter.accel_to_register(start_accel))
            .and_then([this, max_accel] {
                return m_bus.template write<chip::tmc5160::AMAX>(m_converter.accel_to_register(max_accel));
            })
            .and_then([this, max_decel] {
                return m_bus.template write<chip::tmc5160::DMAX>(m_converter.accel_to_register(max_decel));
            })
            .and_then([this, clamped_d1_val] {
                return m_bus.template write<chip::tmc5160::D1>(clamped_d1_val);
            });
    }

    /**
     * @brief Set wait time at standstill before direction change.
     *
     * @param duration Wait duration in seconds.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_standstill_wait(units::time_duration_t duration)
    {
        const uint32_t val{m_converter.duration_to_tzerowait(duration)};
        return m_bus.template write<chip::tmc5160::TZEROWAIT>(val);
    }

    /**
     * @brief Set current motor position (XACTUAL register).
     *
     * @param value Position in microsteps.
     * @note Only modify when homing. In positioning mode, this starts motion.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_actual_motor_position(units::microsteps_t value)
    {
        return m_bus.template write<chip::tmc5160::XACTUAL>(static_cast<uint32_t>(value.raw()));
    }

    /**
     * @brief Set motor run current (IRUN).
     *
     * @param current Run current in Amperes.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_irun(units::current_t current)
    {
        const uint8_t irun_val{m_converter.current_to_cs(current)};

        return m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_run_t>(irun_val);
    }

    /**
     * @brief Set motor hold current (IHOLD).
     *
     * @param current Standstill current in Amperes.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_ihold(units::current_t current)
    {
        return m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_hold_t>(m_converter.current_to_cs(current))
            .and_then([this] {
                constexpr uint32_t hold_delay_default_value{6U};
                return m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_hold_delay_t>(hold_delay_default_value);
            });
    }

    /**
     * @brief Sets the motor acceleration.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_acceleration(units::acceleration_t acceleration)
    {
        return m_bus.template write<chip::tmc5160::AMAX>(m_converter.accel_to_register(acceleration));
    }

    /**
     * @brief Get current motor position.
     *
     * @return Result<int32_t> Position in microsteps (signed) or error.
     */
    [[nodiscard]] helpers::result_t<int32_t> get_actual_motor_position()
    {
        return m_bus.template read<chip::tmc5160::XACTUAL>().map([](uint32_t val) {
            return static_cast<int32_t>(val);
        });
    }

    /**
     * @brief Get current motor velocity.
     *
     * @return Result<rpm_t> Absolute velocity in RPM or error.
     */
    [[nodiscard]] helpers::result_t<units::rpm_t> get_actual_velocity()
    {
        return m_bus.template read<chip::tmc5160::VACTUAL>().map([this](uint32_t val) {
            auto signed_val{static_cast<int32_t>(val)};
            if (val & vactual_sign_bit)
            {
                signed_val |= vactual_sign_extension;
            }
            return m_converter.vmax_to_rpm(static_cast<uint32_t>(std::abs(signed_val)));
        });
    }

  private:
    Bus& m_bus;
    features::Converter& m_converter;
};

} // namespace tmcxx::detail

#endif // TMCXX_TMC5160_MOTION_HPP
