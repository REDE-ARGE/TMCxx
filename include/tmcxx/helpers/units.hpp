/************************************************************
 *  Project : TMCxx
 *  File    : units
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 *
 *  @noter Strong type implementation.
 ************************************************************/

#ifndef TMCXX_COMMON_UNITS_HPP
#define TMCXX_COMMON_UNITS_HPP

#include <compare>
#include <cstdint>
#include <ostream>
#include <type_traits>

namespace tmcxx::units {
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T> && std::three_way_comparable<T>;

template<typename Tag, Arithmetic T>
struct Quantity
{
    T value{};

    constexpr Quantity() = default;

    constexpr explicit Quantity(T val) noexcept
        : value{val}
    {
    }

    constexpr auto operator<=>(const Quantity&) const noexcept = default;

    [[nodiscard]] friend constexpr auto operator+(const Quantity& lhs, const Quantity& rhs) noexcept -> Quantity
    {
        return Quantity{lhs.value + rhs.value};
    }

    [[nodiscard]] friend constexpr auto operator-(const Quantity& lhs, const Quantity& rhs) noexcept -> Quantity
    {
        return Quantity{lhs.value - rhs.value};
    }

    [[nodiscard]] friend constexpr auto operator*(const Quantity& quantity, T scalar) noexcept -> Quantity
    {
        return Quantity{quantity.value * scalar};
    }

    [[nodiscard]] friend constexpr auto operator*(T scalar, const Quantity& quantity) noexcept -> Quantity
    {
        return Quantity{scalar * quantity.value};
    }

    [[nodiscard]] friend constexpr auto operator/(const Quantity& quantity, T scalar) noexcept -> Quantity
    {
        assert(scalar != T{0} && "Division by zero in Quantity");
        return Quantity{quantity.value / scalar};
    }

    [[nodiscard]] friend constexpr auto operator/(const Quantity& lhs, const Quantity& rhs) noexcept -> T
    {
        if (constexpr auto zero_value{T{0}}; zero_value == rhs.value) [[unlikely]]
        {
            return zero_value;
        }

        return lhs.value / rhs.value;
    }

    constexpr auto operator+=(const Quantity& rhs) noexcept -> Quantity&
    {
        value += rhs.value;
        return *this;
    }

    constexpr auto operator-=(const Quantity& rhs) noexcept -> Quantity&
    {
        value -= rhs.value;
        return *this;
    }

    constexpr auto operator-() const noexcept -> Quantity
    {
        return Quantity{-value};
    }

    constexpr auto operator+() const noexcept -> Quantity
    {
        return Quantity{+value};
    }

    [[nodiscard]] constexpr auto operator*() const noexcept -> const T&
    {
        return value;
    }

    [[nodiscard]] constexpr auto raw() const noexcept -> T
    {
        return value;
    }

    friend auto operator<<(std::ostream& oss, const Quantity& quantity) -> std::ostream&
    {
        return oss << quantity.value;
    }
};

/**
 * @brief Velocity(speed) strong type alias.
 * @note Registers: VMAX, VSTART, VSTOP, V1, VACTUAL, TPWMTHRS, TCOOLTHRS, THIGH, VDCMIN
 */
using rpm_t = Quantity<struct RpmTag, float>;
using pps_t = Quantity<struct PpsTag, float>;

/**
 * @brief Acceleration strong type alias.
 *
 * @notes Registers: AMAX, DMAX, A1, D1
 */
using acceleration_t = Quantity<struct AccelerationTag, float>;

/**
 * @brief Position strong type alias.
 *
 * @note Registers: XACTUAL, XTARGET, XLATCH, ENC_LATCH
 */
using microsteps_t = Quantity<struct MicrostepsTag, int32_t>;

/**
 * @brief Current strong type alias.
 *
 * @note Registers: IHOLD, IRUN
 */
using current_t = Quantity<struct CurrentTag, float>;

/**
 * @brief Time strong type alias.
 *
 * @note Registers: TZEROWAIT, TPOWERDOWN, IHOLDDELAY
 */
using time_duration_t = Quantity<struct TimeTag, float>;

/**
 * @brief Frequency strong type alias
 *
 * @note Config: Clock Frequency
 */
using frequency_t = Quantity<struct FrequencyTag, float>; // Hertz

/**
 * @brief Resistance strong type alias.
 *
 * @note Config: r_sense
 */
using resistance_t = Quantity<struct ResistanceTag, float>; // Ohms

/**
 * @brief Voltage strong type alias.
 *
 * @note Config: v_fs(Full Scale Voltage) or v_supply
 */
using voltage_t = Quantity<struct VoltageTag, float>; // Volts

/**
 * @brief Ratio/Factor strong type alias.
 *
 * @note Registers: GLOBAL_SCALER(0-256), PWM_GRAD, PWM_OFS
 */
using factor_t = Quantity<struct FactorTag, float>;

namespace literals {
// --- Velocity ---
consteval auto operator"" _rpm(long double val) noexcept -> rpm_t
{
    return rpm_t{static_cast<float>(val)};
}
consteval auto operator"" _rpm(unsigned long long val) noexcept -> rpm_t
{
    return rpm_t{static_cast<float>(val)};
}

consteval auto operator"" _pps(long double val) noexcept -> pps_t
{
    return pps_t{static_cast<float>(val)};
}
consteval auto operator"" _pps(unsigned long long val) noexcept -> pps_t
{
    return pps_t{static_cast<float>(val)};
}
// --- Current ---
consteval auto operator"" _A(long double val) noexcept -> current_t
{
    return current_t{static_cast<float>(val)};
}
consteval auto operator"" _A(unsigned long long val) noexcept -> current_t
{
    return current_t{static_cast<float>(val)};
}

consteval auto operator"" _mA(long double val) noexcept -> current_t
{
    return current_t{static_cast<float>(val) / 1000.0f};
}
consteval auto operator"" _mA(unsigned long long val) noexcept -> current_t
{
    return current_t{static_cast<float>(val) / 1000.0f};
}

// --- Position ---
consteval auto operator"" _steps(unsigned long long val) noexcept -> microsteps_t
{
    return microsteps_t{static_cast<int32_t>(val)};
}

// --- Acceleration ---
consteval auto operator"" _pps2(unsigned long long val) noexcept -> acceleration_t
{
    return acceleration_t{static_cast<float>(val)};
}
consteval auto operator"" _pps2(long double val) noexcept -> acceleration_t
{
    return acceleration_t{static_cast<float>(val)};
}

// --- Time ---
consteval auto operator"" _s(long double val) noexcept -> time_duration_t
{
    return time_duration_t{static_cast<float>(val)};
}
consteval auto operator"" _s(unsigned long long val) noexcept -> time_duration_t
{
    return time_duration_t{static_cast<float>(val)};
}

consteval auto operator"" _ms(long double val) noexcept -> time_duration_t
{
    return time_duration_t{static_cast<float>(val) / 1000.0f};
}
consteval auto operator"" _ms(unsigned long long val) noexcept -> time_duration_t
{
    return time_duration_t{static_cast<float>(val) / 1000.0f};
}

consteval auto operator"" _us(long double val) noexcept -> time_duration_t
{
    return time_duration_t{static_cast<float>(val) / 1'000'000.0f};
}
consteval auto operator"" _us(unsigned long long val) noexcept -> time_duration_t
{
    return time_duration_t{static_cast<float>(val) / 1'000'000.0f};
}

// --- Frequency ---
consteval auto operator"" _Hz(unsigned long long val) noexcept -> frequency_t
{
    return frequency_t{static_cast<float>(val)};
}
consteval auto operator"" _MHz(long double val) noexcept -> frequency_t
{
    return frequency_t{static_cast<float>(val) * 1'000'000.0f};
}

// --- Resistance ---
consteval auto operator"" _Ohm(long double val) noexcept -> resistance_t
{
    return resistance_t{static_cast<float>(val)};
}
consteval auto operator"" _mOhm(long double val) noexcept -> resistance_t
{
    return resistance_t{static_cast<float>(val) / 1000.0f};
}

// --- Factor/Ratio Literals ---
consteval auto operator"" _factor(long double val) noexcept -> factor_t
{
    return factor_t{static_cast<float>(val)};
}
consteval auto operator"" _percent(long double val) noexcept -> factor_t
{
    return factor_t{static_cast<float>(val) / 100.0f};
}
consteval auto operator"" _percent(unsigned long long val) noexcept -> factor_t
{
    return factor_t{static_cast<float>(val) / 100.0f};
}
} // namespace literals
} // namespace tmcxx::units

#endif // TMCXX_COMMON_UNITS_HPP
