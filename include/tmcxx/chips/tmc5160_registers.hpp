/************************************************************
 *  Project : TMCxx
 *  File    : tmc5160_registers
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 ************************************************************/

#ifndef TMCXX_CHIPS_TMC5160_REGISTERS_HPP
#define TMCXX_CHIPS_TMC5160_REGISTERS_HPP

#include "tmcxx/base/concepts.hpp"
#include "tmcxx/base/register_base.hpp"

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace tmcxx::chip::tmc5160 {

/**
 * @brief TMC5160 Register Map (Datasheet Rev 1.17)
 */
enum class RegAddress : uint8_t {
    // --- General Configuration Registers ---
    GCONF = 0x00U,         // Global Configuration
    GSTAT = 0x01U,         // Global Status Flags
    IFCNT = 0x02U,         // Interface Transmission Counter
    SLAVECONF = 0x03U,     // Slave Configuration
    IOIN = 0x04U,          // Input / Output Reads
    OUTPUT = 0x04U,        // Output Settings (Write only)
    X_COMPARE = 0x05U,     // Position Comparison Register
    OTP_PROG = 0x06U,      // OTP Programming
    OTP_READ = 0x07U,      // OTP Read
    FACTORY_CONF = 0x08U,  // Factory Configuration (Clock trim)
    SHORT_CONF = 0x09U,    // Short Circuit Configuration
    DRV_CONF = 0x0AU,      // Driver Configuration
    GLOBAL_SCALER = 0x0BU, // Global Scaling of Motor Current
    OFFSET_READ = 0x0CU,   // Offset Read

    // --- Velocity Dependent Driver Feature Control ---
    IHOLD_IRUN = 0x10U, // Driver Current Control
    TPOWERDOWN = 0x11U, // Delay before power down
    TSTEP = 0x12U,      // Actual time between two microsteps
    TPWMTHRS = 0x13U,   // Upper velocity for StealthChop voltage PWM mode
    TCOOLTHRS = 0x14U,  // Lower velocity for CoolStep and StallGuard
    THIGH = 0x15U,      // Velocity threshold for high speed mode

    // --- Ramp Generator Motion Control Registers ---
    RAMPMODE = 0x20U,  // Driving Mode (Velocity, Position, Hold)
    XACTUAL = 0x21U,   // Actual Motor Position
    VACTUAL = 0x22U,   // Actual Motor Velocity
    VSTART = 0x23U,    // Motor Start Velocity
    A1 = 0x24U,        // First Acceleration between VSTART and V1
    V1 = 0x25U,        // First Velocity Threshold
    AMAX = 0x26U,      // Second Acceleration between V1 and VMAX
    VMAX = 0x27U,      // Target Velocity (in Velocity mode) or Max Velocity (in Pos mode)
    DMAX = 0x28U,      // Second Deceleration between VMAX and V1
    D1 = 0x2AU,        // First Deceleration between V1 and VSTOP
    VSTOP = 0x2BU,     // Stop Velocity (Near Zero)
    TZEROWAIT = 0x2CU, // Wait time after ramp down
    XTARGET = 0x2DU,   // Target Position (in Position mode)

    // --- Ramp Generator Driver Feature Control ---
    VDCMIN = 0x33U,    // Velocity threshold for automatic commutation switch
    SW_MODE = 0x34U,   // Switch Mode Configuration (Limit Switches)
    RAMP_STAT = 0x35U, // Ramp Status
    XLATCH = 0x36U,    // Latched Position on Latch Input

    // --- Encoder Registers ---
    ENCMODE = 0x38U,    // Encoder Configuration
    X_ENC = 0x39U,      // Actual Encoder Position
    ENC_CONST = 0x3AU,  // Encoder Constant (Microsteps per encoder tick)
    ENC_STATUS = 0x3BU, // Encoder Status
    ENC_LATCH = 0x3CU,  // Latched Encoder Position

    // --- Microstepping Control Registers ---
    MSLUT_0 = 0x60U,    // Microstep Table Entries 0-31
    MSLUT_1 = 0x61U,    // ...
    MSLUT_2 = 0x62U,    // ...
    MSLUT_3 = 0x63U,    // ...
    MSLUT_4 = 0x64U,    // ...
    MSLUT_5 = 0x65U,    // ...
    MSLUT_6 = 0x66U,    // ...
    MSLUT_7 = 0x67U,    // Microstep Table Entries 224-255
    MSLUTSEL = 0x68U,   // LUT Segment Selection
    MSLUTSTART = 0x69U, // LUT Start Selection
    MSCNT = 0x6AU,      // Actual Microstep Counter
    MSCURACT = 0x6BU,   // Actual Microstep Current

    // --- Driver Register (Chopper & PWM) ---
    CHOPCONF = 0x6CU,   // Chopper Configuration
    COOLCONF = 0x6DU,   // CoolStep Smart Current Control
    DCCTRL = 0x6EU,     // DC Step Control
    DRV_STATUS = 0x6FU, // Driver Status Flags (Stallguard, temp, etc.)
    PWMCONF = 0x70U,    // StealthChop PWM Config
    PWM_SCALE = 0x71U,  // Actual PWM Amplitude Scaler
    PWM_AUTO = 0x72U,   // Automatic PWM Amplitude Control
    LOST_STEPS = 0x73U, // Lost Steps Counter
};

/**
 * @brief TMC Ramp Mode types.
 *
 * @note Register address: 0x20, check description for details.
 */
enum class RampModeType : uint32_t {
    POSITIONING = 0U,  // Target position move
    VELOCITY_POS = 1U, // Positive velocity mode
    VELOCITY_NEG = 2U, // Negative velocity mode
    HOLD = 3U          // Hold mode
};

/**
 * @brief Register Address enum under type alias.
 */
using reg_address_undertype_t = std::underlying_type_t<RegAddress>;

// ========================================================================
// READ-WRITE REGISTERS (Detailed Field Definitions)
// ========================================================================

template<core::concepts::Register... Args>
struct RegisterTuple
{
    template<reg_address_undertype_t SearchAddr>
    [[nodiscard]] static consteval std::size_t find_index() noexcept
    {
        std::size_t idx{};
        std::size_t result{sizeof...(Args)};

        (((Args::address == SearchAddr ? result = idx : 0), idx++), ...);

        return result;
    }

    template<RegAddress Addr>
    [[nodiscard]] constexpr const auto& get() const noexcept
    {
        constexpr std::size_t idx{find_index<static_cast<reg_address_undertype_t>(Addr)>()};

        static_assert(idx < sizeof...(Args), "Related register not found in the tuple!");

        return std::get<idx>(fields);
    }

    std::tuple<std::decay_t<Args>...> fields;
};

/**
 * @brief Global Configuration Flags (0x00)
 * Reference: Datasheet Page 32, Section 6.1
 */
struct GCONF : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::GCONF), core::Access::RW>
{
  private:
    // Bit positions (Magic Numbers Hidden Here)
    static constexpr uint8_t p_recalibrate{0};
    static constexpr uint8_t p_fast_standstill{1};
    static constexpr uint8_t p_en_pwm_mode{2};
    static constexpr uint8_t p_multistep_filt{3};
    static constexpr uint8_t p_shaft{4};
    static constexpr uint8_t p_diag0_error{5};
    static constexpr uint8_t p_diag0_otpw{6};
    static constexpr uint8_t p_diag0_stall{7};
    static constexpr uint8_t p_diag1_stall{8};
    static constexpr uint8_t p_diag1_index{9};
    static constexpr uint8_t p_diag1_onstate{10};
    static constexpr uint8_t p_diag1_steps_skipped{11};
    static constexpr uint8_t p_diag0_int_push_pull{12};
    static constexpr uint8_t p_diag1_pos_comp_push_pull{13};
    static constexpr uint8_t p_small_hysteresis{14};
    static constexpr uint8_t p_stop_enable{15};
    static constexpr uint8_t p_direct_mode{16};

  public:
    /**
     * @brief Zero crossing recalibration during driver disable.
     * 1: Enable recalibration via DRV_ENN or TOFF setting.
     */
    using recalibrate_t = core::Field<GCONF, p_recalibrate>;

    /**
     * @brief Timeout for step execution until standstill detection.
     * 0: Normal time (2^20 clocks), 1: Short time (2^18 clocks)
     */
    using fast_standstill_t = core::Field<GCONF, p_fast_standstill>;

    /**
     * @brief StealthChop voltage PWM mode enabled.
     * 1: Enabled (depending on velocity thresholds).
     */
    using en_pwm_mode_t = core::Field<GCONF, p_en_pwm_mode>;

    /**
     * @brief Enable step input filtering for StealthChop.
     * 1: Optimization for external step source enabled (default=1).
     */
    using multistep_filt_t = core::Field<GCONF, p_multistep_filt>;

    /**
     * @brief Inverse motor direction.
     * 1: Motor direction inverted.
     */
    using shaft_t = core::Field<GCONF, p_shaft>;

    /**
     * @brief Enable DIAG0 active on driver errors.
     * (Over temperature or short to GND).
     */
    using diag0_error_t = core::Field<GCONF, p_diag0_error>;

    /**
     * @brief Enable DIAG0 active on over temperature pre-warning (otpw).
     */
    using diag0_otpw_t = core::Field<GCONF, p_diag0_otpw>;
};

/**
 * @brief Driver Current Control (0x10)
 * Reference: Datasheet Page 38, Section 6.2
 */
struct IHOLD_IRUN
    : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::IHOLD_IRUN), core::Access::RW>
{
  private:
    // Positions and Lengths
    static constexpr uint8_t p_i_hold{0};
    static constexpr uint8_t l_i_hold{5};
    static constexpr uint8_t p_i_run{8};
    static constexpr uint8_t l_i_run{5};
    static constexpr uint8_t p_delay{16};
    static constexpr uint8_t l_delay{4};

  public:
    /**
     * @brief Standstill current (0=1/32...31=32/32).
     * Sets the motor current when in standstill.
     */
    using i_hold_t = core::Field<IHOLD_IRUN, p_i_hold, l_i_hold>;

    /**
     * @brief Motor run current (0=1/32...31=32/32).
     * Sets the motor current when running.
     */
    using i_run_t = core::Field<IHOLD_IRUN, p_i_run, l_i_run>;

    /**
     * @brief Power down delay.
     * Controls the number of clock cycles for motor power down after standstill.
     * Range: 0..15 (Time = Delay * 2^18 clocks)
     */
    using i_hold_delay_t = core::Field<IHOLD_IRUN, p_delay, l_delay>;
};

/**
 * @brief Chopper Configuration (0x6C)
 * Reference: Datasheet Page 51, Section 6.5.2
 */
struct CHOPCONF : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::CHOPCONF), core::Access::RW>
{
  private:
    static constexpr uint8_t p_toff{0U};
    static constexpr uint8_t l_toff{4U};
    static constexpr uint8_t p_hstrt{4U};
    static constexpr uint8_t l_hstrt{3U};
    static constexpr uint8_t p_hend{7U};
    static constexpr uint8_t l_hend{4U};
    static constexpr uint8_t p_tbl{15U};
    static constexpr uint8_t l_tbl{2U};
    static constexpr uint8_t p_chm{14U};
    static constexpr uint8_t p_mres{24U};
    static constexpr uint8_t l_mres{4U};
    static constexpr uint8_t p_intpol{28U};
    static constexpr uint8_t p_dedge{29U};
    static constexpr uint8_t p_diss2g{30U};
    static constexpr uint8_t p_diss2vs{31U};

  public:
    /**
     * @brief Off time and driver enable.
     * 0: Driver disable, all bridges off.
     * 1..15: Off time setting (N_clk = 24 + 32*TOFF).
     */
    using toff_t = core::Field<CHOPCONF, p_toff, l_toff>;

    /**
     * @brief Hysteresis start value.
     * Offset from HEND. (HSTRT = 1...8). Adds to HEND.
     */
    using hstrt_t = core::Field<CHOPCONF, p_hstrt, l_hstrt>;

    /**
     * @brief Hysteresis end value.
     * Offset: -3..12.
     */
    using hend_t = core::Field<CHOPCONF, p_hend, l_hend>;

    /**
     * @brief Comparator blank time.
     * Selects the comparator blank time to cover switching events.
     * 0:16clks, 1:24clks, 2:36clks, 3:54clks.
     */
    using tbl_t = core::Field<CHOPCONF, p_tbl, l_tbl>;

    /**
     * @brief Chopper mode selection.
     * 0: Standard mode (SpreadCycle)
     * 1: Constant off time with fast decay.
     */
    using chm_t = core::Field<CHOPCONF, p_chm>;

    /**
     * @brief Microstep resolution.
     * 0: 256 microsteps (Native)
     * 4: 16 microsteps
     * 8: Fullstep
     * Resolution = 256 / (2^MRES)
     */
    using mres_t = core::Field<CHOPCONF, p_mres, l_mres>;

    /**
     * @brief Interpolation to 256 microsteps.
     * 1: Enable interpolation (only for Step/Dir mode).
     */
    using intpol_t = core::Field<CHOPCONF, p_intpol>;

    /**
     * @brief Double edge step pulses.
     * 1: Enable step impulse at both rising and falling edges.
     */
    using double_edge_t = core::Field<CHOPCONF, p_dedge>;
};

/**
 * @brief Switch Mode Configuration (0x34)
 * Reference: Datasheet Page 43, Section 6.3.2.1
 */
struct SW_MODE : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::SW_MODE), core::Access::RW>
{
  private:
    static constexpr uint8_t p_stop_l_enable{0U};
    static constexpr uint8_t p_stop_r_enable{1U};
    static constexpr uint8_t p_sg_stop{10U};
    static constexpr uint8_t p_en_softstop{11U};

  public:
    // Bit 0: stop_l_enable (Automatic motor stop on left switch) [cite: 1490]
    using stop_l_enable_t = core::Field<SW_MODE, p_stop_l_enable>;

    // Bit 1: stop_r_enable (Automatic motor stop on right switch) [cite: 1490]
    using stop_r_enable_t = core::Field<SW_MODE, p_stop_r_enable>;

    // Bit 10: sg_stop (Enable stop by StallGuard2) [cite: 1490]
    using sg_stop_t = core::Field<SW_MODE, p_sg_stop>;

    // Bit 11: en_softstop (0: Hard stop, 1: Soft stop) [cite: 1490]
    using en_softstop_t = core::Field<SW_MODE, p_en_softstop>;
};

/**
 *
 */
struct RAMPMODE : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::RAMPMODE), core::Access::RW>
{
  private:
    static constexpr uint8_t p_ramp_mode{0};
    static constexpr uint8_t l_ramp_mode{2};

  public:
    // Bit 0..1: RAMPMODE
    using mode_t = core::Field<RAMPMODE, p_ramp_mode, l_ramp_mode>;

    using type_t = RampModeType;
};

/**
 * @brief Actual motor position (0x21).
 * Signed 32-bit value. Can be written to set the current position (e.g. after homing).
 * Reference: Datasheet Page 40, Section 6.3.1
 */
struct XACTUAL
    : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::XACTUAL), core::Access::RW, core::Volatile>
{
};

/**
 * @brief Motion Ramp Target Velocity (0x27)
 * Sets the maximum velocity in positioning mode or the target velocity
 * in velocity mode.
 * Reference: Datasheet Page 40, Section 6.3.1
 */
struct VMAX : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::VMAX), core::Access::RW>
{
    //
};

/**
 * @brief Maximum Acceleration (0x26)
 * * This is the second acceleration between V1 and VMAX (velocity mode).
 * Range: 0 ... (2^16)-1
 * Reference: Datasheet Page 40, Section 6.3.1
 */
struct AMAX : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::AMAX), core::Access::RW>
{
};

// ========================================================================
// WRITE-ONLY REGISTERS
// ========================================================================

/**
 * @brief Global Scaler (0x0B)
 * Sets the global current scaling factor (0-255) for motor current.
 * [cite_start]Reference: Datasheet Page 36 [cite: 1446]
 */
struct GLOBAL_SCALER
    : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::GLOBAL_SCALER), core::Access::WO>
{
};

/**
 * @brief Target Position (0x2D)
 * The target position for ramp mode.
 * [cite_start]Reference: Datasheet Page 41 [cite: 1475]
 */
struct XTARGET : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::XTARGET), core::Access::WO>
{
};

/**
 * @brief Motor Start Velocity (0x23)
 *
 * Initial velocity when motor starts moving.
 * Range: 0 ... (2^18)-1
 */
struct VSTART : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::VSTART), core::Access::WO>
{
};

/**
 * @brief First Acceleration (0x24)
 *
 * Acceleration between VSTART and V1.
 * Range: 0 ... (2^16)-1
 */
struct A1 : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::A1), core::Access::WO>
{
};

/**
 * @brief Acceleration Threshold Velocity (0x25)
 *
 * Velocity threshold for A1 to AMAX transition.
 * Range: 0 ... (2^20)-1
 */
struct V1 : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::V1), core::Access::WO>
{
};

/**
 * @brief Main Deceleration (0x28)
 *
 * Deceleration from VMAX to V1.
 * Range: 0 ... (2^16)-1
 */
struct DMAX : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::DMAX), core::Access::WO>
{
};

/**
 * @brief Second Deceleration (0x2A)
 *
 * Deceleration below V1 until stop.
 * Range: 1 ... (2^16)-1 (never set to 0!)
 */
struct D1 : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::D1), core::Access::WO>
{
};

/**
 * @brief Stop Velocity (0x2B)
 *
 * Final velocity before stop.
 * Range: 1 ... (2^18)-1
 */
struct VSTOP : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::VSTOP), core::Access::WO>
{
};

/**
 * @brief Zero Wait Time (0x2C)
 *
 * Wait time at standstill before direction change.
 * Range: 0 ... (2^16)-1
 */
struct TZEROWAIT : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::TZEROWAIT), core::Access::WO>
{
};

/**
 *@brief
 */
struct TWPOWER_DOWN
    : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::TPOWERDOWN), core::Access::WO>
{
};

/**
 *@brief
 */
struct PWMCONF : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::PWMCONF), core::Access::WO>
{
};

// ========================================================================
// READ-ONLY REGISTERS
// ========================================================================

/**
 * @brief Global Status Flags (0x01)
 * Driver error flags (reset, driver error, uv_cp).
 * Note: Technically R+WC (Read + Write 1 to Clear), treated as RO/RW special.
 * [cite_start]Reference: Datasheet Page 33 [cite: 1433]
 */
struct GSTAT : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::GSTAT), core::Access::RO>
{
};

/**
 * @brief Actual Motor Velocity (0x22)
 * Signed 24-bit value from the internal ramp generator.
 * [cite_start]Reference: Datasheet Page 40 [cite: 1471]
 */
struct VACTUAL
    : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::VACTUAL), core::Access::RO, core::Volatile>
{
};

/**
 * @brief Driver Status (0x6F)
 * StallGuard2 result and driver error flags.
 * [cite_start]Reference: Datasheet Page 56 [cite: 1562]
 */
struct DRV_STATUS
    : core::RegisterAddress<static_cast<reg_address_undertype_t>(RegAddress::DRV_STATUS), core::Access::RO>
{
};

/**
 * @brief Register tuple
 */
inline constexpr RegisterTuple<GCONF,
    IHOLD_IRUN,
    CHOPCONF,
    SW_MODE,
    RAMPMODE,
    XACTUAL,
    VMAX,
    AMAX,
    GLOBAL_SCALER,
    XTARGET,
    VSTART,
    A1,
    V1,
    DMAX,
    D1,
    VSTOP,
    TZEROWAIT,
    TWPOWER_DOWN,
    PWMCONF,
    GSTAT,
    VACTUAL,
    DRV_STATUS>
    register_tuple;

} // namespace tmcxx::chip::tmc5160

#endif // TMCXX_CHIPS_TMC5160_REGISTERS_HPP
