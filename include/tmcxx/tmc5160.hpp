/************************************************************
 *  Project : TMCxx
 *  File    : tmc5160
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 ************************************************************/

#ifndef TMCXX_TMC5160_HPP
#define TMCXX_TMC5160_HPP

#include "tmcxx/chips/tmc5160_registers.hpp"
#include "tmcxx/detail/tmc5160_bus.hpp"
#include "tmcxx/detail/tmc5160_motion.hpp"
#include "tmcxx/detail/tmc5160_register_access.hpp"
#include "tmcxx/features/converter.hpp"
#include "tmcxx/features/core_communicator.hpp"
#include "tmcxx/helpers/constants.hpp"
#include "tmcxx/helpers/error.hpp"
#include "tmcxx/vendor/expected.hpp"

#include <cstdint>

namespace tmcxx {

[[nodiscard]] bool is_all_ok(core::concepts::ExpectedLike auto&&... args) noexcept
{
    return (... && args.has_value());
}

template<core::concepts::SpiDevice TSpi>
class TMC5160 {
    /**
     * @brief Register address enum type alias.
     */
    using regs_t = chip::tmc5160::RegAddress;

    /**
     * @brief Bus type alias.
     */
    using bus_t = detail::TMC5160Bus<TSpi>;

    /**
     * @brief Motion type alias.
     */
    using motion_t = detail::TMC5160Motion<bus_t>;

    /**
     * @brief Register access type alias.
     */
    using regs_access_t = detail::TMC5160RegisterAccess<bus_t>;

  public:
    struct Settings
    {
        units::frequency_t f_clk_hz{helpers::constant::default_clock_freq};
        units::resistance_t r_sense{helpers::constant::default_r_sense_ohms};
        units::microsteps_t full_steps{helpers::constant::default_full_steps};

        units::current_t run_current{};
        units::current_t hold_current{};
        uint8_t hold_delay{};
        uint8_t power_down_delay{};

        units::rpm_t v_start{};
        units::rpm_t v_stop{};
        units::rpm_t v_1{};
        units::rpm_t v_max{};

        units::acceleration_t a_1{};
        units::acceleration_t a_max{};
        units::acceleration_t d_max{};
        units::acceleration_t d_1{};

        bool stealth_chop_enabled{};

        uint8_t toff{};
        uint8_t hstrt{};
        int8_t hend{};
        uint8_t tbl{};
    };

    /**
     * @brief TMC5160 Constructor
     *
     * @param spi_device SPI Device.
     * @param settings Clock frequency.
     */
    explicit TMC5160(TSpi& spi_device, const Settings& settings)
        : m_bus{spi_device}
        , m_converter{settings.f_clk_hz, settings.full_steps, settings.r_sense}
        , m_motion{m_bus, m_converter}
        , m_regs{m_bus}
        , m_settings{settings}
    {
    }
    /**
     * @brief Apply the default configuration (safe hardcoded values).
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> apply_default_configuration() noexcept
    {
        if (!is_all_ok(m_bus.template write<chip::tmc5160::VSTOP>(100U),
                m_bus.template write<chip::tmc5160::V1>(40'000U),
                m_bus.template write<chip::tmc5160::AMAX>(10'000U),
                m_bus.template write<chip::tmc5160::DMAX>(10'000U),
                m_bus.template write<chip::tmc5160::A1>(2'000U),
                m_bus.template write<chip::tmc5160::D1>(10'000U),
                m_bus.template write<chip::tmc5160::TWPOWER_DOWN>(10U),

                m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_hold_delay_t>(6U),
                m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_hold_t>(4U),
                m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_run_t>(16U),

                m_bus.template write_field<chip::tmc5160::CHOPCONF::toff_t>(3U),
                m_bus.template write_field<chip::tmc5160::CHOPCONF::hstrt_t>(4U),
                m_bus.template write_field<chip::tmc5160::CHOPCONF::hend_t>(1U),
                m_bus.template write_field<chip::tmc5160::CHOPCONF::tbl_t>(2U),

                m_bus.template write<chip::tmc5160::XTARGET>(0U))) [[unlikely]]
        {
            return tl::unexpected(helpers::ErrorCode::SPI_TRANSFER_FAILED);
        }

        return m_bus.template write<chip::tmc5160::RAMPMODE>(
            static_cast<uint32_t>(chip::tmc5160::RampModeType::POSITIONING));
    }

    /**
     * @brief Apply the settings from the Settings struct.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> apply_settings() noexcept
    {
        using namespace units::literals;

        if (const auto res{m_bus.template write<chip::tmc5160::RAMPMODE>(
                static_cast<uint32_t>(chip::tmc5160::RampModeType::POSITIONING))};
            !res) [[unlikely]]
        {
            return res;
        }

        if (const auto res{m_motion.set_start_speed(m_settings.v_start)}; !res) [[unlikely]]
        {
            return res;
        }
        if (const auto res{m_motion.set_stop_velocity(m_settings.v_stop)}; !res) [[unlikely]]
        {
            return res;
        }
        if (const auto res{m_motion.set_ramp_transition_velocity(m_settings.v_1)}; !res) [[unlikely]]
        {
            return res;
        }
        if (const auto res{m_motion.set_max_velocity(m_settings.v_max)}; !res) [[unlikely]]
        {
            return res;
        }

        if (auto res{
                m_motion.set_advanced_acceleration(m_settings.a_1, m_settings.a_max, m_settings.d_max, m_settings.d_1)};
            !res) [[unlikely]]
        {
            return res;
        }

        const auto run_current_val{m_converter.current_to_cs(m_settings.run_current)};

        if (const auto hold_current_val{m_converter.current_to_cs(m_settings.hold_current)};
            !is_all_ok(m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_run_t>(run_current_val),
                m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_hold_t>(hold_current_val),
                m_bus.template write_field<chip::tmc5160::IHOLD_IRUN::i_hold_delay_t>(m_settings.hold_delay)))
        {
            return tl::unexpected(helpers::ErrorCode::SPI_TRANSFER_FAILED);
        }

        if (const auto res{m_bus.template write<chip::tmc5160::TWPOWER_DOWN>(m_settings.power_down_delay)}; !res)
            [[unlikely]]
        {
            return res;
        }

        if (!is_all_ok(m_bus.template write_field<chip::tmc5160::CHOPCONF::toff_t>(m_settings.toff),
                m_bus.template write_field<chip::tmc5160::CHOPCONF::hstrt_t>(m_settings.hstrt),
                m_bus.template write_field<chip::tmc5160::CHOPCONF::hend_t>(m_settings.hend),
                m_bus.template write_field<chip::tmc5160::CHOPCONF::tbl_t>(m_settings.tbl)))
        {
            return tl::unexpected(helpers::ErrorCode::SPI_TRANSFER_FAILED);
        }

        if (const auto res{m_motion.set_stealth_chop(m_settings.stealth_chop_enabled)}; !res) [[unlikely]]
        {
            return res;
        }

        if (!is_all_ok(m_bus.template write<chip::tmc5160::XTARGET>(0U),
                m_bus.template write<chip::tmc5160::XACTUAL>(0U))) [[unlikely]]
        {
            return tl::unexpected(helpers::ErrorCode::SPI_TRANSFER_FAILED);
        }

        return {};
    }

    /**
     * @brief Stop the motor.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> stop()
    {
        return m_motion.stop();
    }

    /**
     * @brief Continuously rotates the motor at a certain speed (Velocity Mode).
     *
     * @param velocity RPM
     *
     * @note A positive value rotates forward, a negative value rotates backward.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> rotate(units::rpm_t velocity)
    {
        return m_motion.rotate(velocity);
    }

    /**
     * @brief Sets the global current scaling factor.
     *
     * @param factor: A ratio between 0.0 (0%) and 1.0 (100%).
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_global_scaling(units::factor_t factor)
    {
        return m_motion.set_global_scaling(factor);
    }

    /**
     * @brief Move motor to target position.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> move_to(units::microsteps_t step, units::rpm_t max_speed)
    {
        return m_motion.move_to(step, max_speed);
    }

    /**
     * @brief Set ramp generator mode.
     *
     * @param mode Positioning, velocity positive/negative, or hold.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_ramp_mode(chip::tmc5160::RampModeType mode)
    {
        return m_motion.set_ramp_mode(mode);
    }

    /**
     * @brief Enable or disable StealthChop mode.
     *
     * @param value True to enable, false to disable.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_stealth_chop(bool value)
    {
        return m_motion.set_stealth_chop(value);
    }

    /**
     * @brief Get the actual motor position.
     *
     * @return int32_t or nullopt.
     */
    /**
     * @brief Get the actual motor position.
     *
     * @return Result<int32_t>.
     */
    [[nodiscard]] helpers::result_t<int32_t> get_actual_motor_position()
    {
        return m_motion.get_actual_motor_position();
    }

    /**
     * @brief Read current speed.
     * @return Result<rpm_t>.
     */
    [[nodiscard]] helpers::result_t<units::rpm_t> get_actual_velocity()
    {
        return m_motion.get_actual_velocity();
    }

    /**
     * @brief Set motor run current (IRUN).
     *
     * @param current Run current in Amperes.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_irun(units::current_t current)
    {
        return m_motion.set_irun(current);
    }

    /**
     * @brief Set motor hold current (IHOLD).
     *
     * @param current Standstill current in Amperes.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_ihold(units::current_t current)
    {
        return m_motion.set_ihold(current);
    }

    /**
     * @brief Sets the motor acceleration.
     *
     * @param acceleration Defines the desired acceleration value for the motor,
     *                     represented as a quantity of type units::acceleration_t.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_acceleration(units::acceleration_t acceleration)
    {
        return m_motion.set_acceleration(acceleration);
    }

    /**
     * @brief Sets the motor's starting speed.
     *
     * This function configures the starting speed of the motor by calculating the corresponding
     * value needed for the proper register (`VStart`) and writing it via the SPI device.
     *
     * @param start_speed The desired starting speed expressed in revolutions per minute (rpm).
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_start_speed(units::rpm_t start_speed)
    {
        return m_motion.set_start_speed(start_speed);
    }

    /**
     * @brief Sets the ramp transition velocity of the motor.
     *
     * This method configures the transition velocity for the ramp profile
     * by converting the provided speed value into a format compatible with the
     * TMC5160 register and transmitting it via the SPI device.
     *
     * @param speed Desired ramp transition velocity, represented in revolutions per minute (rpm).
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_ramp_transition_velocity(units::rpm_t speed)
    {
        return m_motion.set_ramp_transition_velocity(speed);
    }

    /**
     * @brief Sets the maximum velocity for the motor.
     *
     * Configures the motor driver's `VMax` register with an appropriate value
     * derived from the specified maximum velocity in revolutions per minute (rpm).
     *
     * @param max_velocity The desired maximum velocity for the motor, expressed in `units::rpm_t`.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_max_velocity(units::rpm_t max_velocity)
    {
        return m_motion.set_max_velocity(max_velocity);
    }

    /**
     * @brief Sets the stop velocity of the motor.
     *
     * Converts the specified stop velocity into the appropriate register value
     * and writes it to the TMC5160 `VStop` register using the SPI device.
     *
     * @param speed The desired stop velocity for the motor, expressed in `units::rpm_t`.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_stop_velocity(units::rpm_t speed)
    {
        return m_motion.set_stop_velocity(speed);
    }

    /**
     * @brief Sets linear acceleration and deceleration for the motor.
     *
     * Configures the motor's acceleration and deceleration values by converting them
     * into the corresponding register values and writing them to the TMC5160 registers.
     *
     * @param acceleration The desired linear acceleration represented as `units::acceleration_t`.
     * @param decel The desired linear deceleration represented as `units::acceleration_t`.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_linear_acceleration(
        units::acceleration_t acceleration, units::acceleration_t decel)
    {
        return m_motion.set_linear_acceleration(acceleration, decel);
    }

    /**
     * @brief Configures the advanced acceleration settings for the TMC5160.
     *
     * @param start_accel The initial acceleration value.
     * @param max_accel The maximum acceleration value during motion.
     * @param max_decel The maximum deceleration value during slowing down.
     * @param stop_decel The deceleration value applied during stopping.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_advanced_acceleration(units::acceleration_t start_accel,
        units::acceleration_t max_accel,
        units::acceleration_t max_decel,
        units::acceleration_t stop_decel)
    {
        return m_motion.set_advanced_acceleration(start_accel, max_accel, max_decel, stop_decel);
    }

    /**
     * @brief Set wait time at standstill before direction change.
     *
     * @param duration Wait duration.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_standstill_wait(units::time_duration_t duration)
    {
        return m_motion.set_standstill_wait(duration);
    }

    /**
     * @brief Set the XACTUAL register.
     *
     * @param value Motor position value(-2^31 - +(2^31) - 1)
     *
     * @note This value normally should only be modified, when homing the drive. In positioning mode, modifying the
     * register content will start a motion.
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_actual_motor_position(units::microsteps_t value)
    {
        return m_motion.set_actual_motor_position(value);
    }

    /**
     *
     * @return
     */
    [[nodiscard]] helpers::result_t<std::array<uint32_t, helpers::constant::tmc_register_count>> get_all_registers()
    {
        return m_regs.get_all_registers();
    }

    /**
     *
     * @param reg_address
     *
     * @return
     */
    [[nodiscard]] helpers::result_t<uint32_t> get_register_value(chip::tmc5160::RegAddress reg_address)
    {
        return m_regs.get_register_value(reg_address);
    }

    /**
     *
     * @param reg_address
     * @param value
     * @return Result<void>.
     */
    [[nodiscard]] helpers::result_t<void> set_register_value(chip::tmc5160::RegAddress reg_address, uint32_t value)
    {
        return m_regs.set_register_value(reg_address, value);
    }

  private:
    /**
     * @brief Tested default datas for running step motor.
     */

    bus_t m_bus{};
    features::Converter m_converter;
    motion_t m_motion{};
    regs_access_t m_regs{};
    Settings m_settings{};
};

} // namespace tmcxx

#endif // TMCXX_TMC5160_HPP
