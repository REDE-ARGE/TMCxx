/************************************************************
 *  Project : TMCxx
 *  File    : constants
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 ************************************************************/

#ifndef TMCXX_HELPERS_CONSTANTS_HPP
#define TMCXX_HELPERS_CONSTANTS_HPP

#include <cstdint>

#include "tmcxx/helpers/units.hpp"

namespace tmcxx::helpers::constant {
using units::literals::operator""_MHz;
using units::literals::operator""_steps;
using units::literals::operator""_mOhm;

/**
 * @brief Write access bit mask.
 */
inline constexpr uint8_t tmc_write_bit{0x80U};

/**
 * @brief Read access bit mask.
 */
inline constexpr uint8_t tmc_read_bit{0x00U};

/**
 * @brief Default internal clock frequency.
 *
 * @note Most TMC5160 modules use a 12 MHz crystal or internal OSC.
 *
 * Reference: Datasheet Page 119, Section 26.
 */
inline constexpr auto default_clock_freq{12.0_MHz};

/**
 * @brief Full step value.
 */
inline constexpr auto default_full_steps{200_steps};

/**
 * @brief TMC register count.
 */
inline constexpr uint8_t tmc_register_count{128U};

/**
 * @brief Default sense resistor value ($R_{SENSE}$) in Ohms.
 *
 * This value is used to convert physical current (Amperes) to the internal
 * Current Scale (CS) value.
 *
 * - Default: 0.075 Ohm (Standard for TMC5160-EVAL board).
 * - Range: Typically 0.022 to 0.15 Ohm depending on target current.
 *
 * @note This must match the hardware resistor to ensure correct current limits.
 * Reference: Datasheet Page 74, Section 9 "Selecting Sense Resistors"[cite: 2146].
 */
inline constexpr auto default_r_sense_ohms{75.0_mOhm};

} // namespace tmcxx::helpers::constant

#endif // TMCXX_HELPERS_CONSTANTS_HPP
