/************************************************************
 *  Project : TMCxx
 *  File    : concepts
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 ************************************************************/

#ifndef TMCXX_CORE_CONCEPTS_HPP
#define TMCXX_CORE_CONCEPTS_HPP

#include "register_base.hpp"

#include <concepts>
#include <cstdint>
#include <span>

namespace tmcxx::core::concepts {

/**
 * @brief Concept for register types.
 *
 * Requires address and access static members.
 */
template<typename T>
concept Register = requires {
    { T::address } -> std::convertible_to<uint8_t>;
    { T::access } -> std::convertible_to<Access>;
};

/**
 * @brief Concept for register field types.
 *
 * Requires mask, shift, and parent register type.
 */
template<typename T>
concept Field = requires {
    typename T::register_t;
    { T::mask } -> std::convertible_to<uint32_t>;
    { T::shift } -> std::convertible_to<uint8_t>;
};

template<class T>
concept ExpectedLike = requires(T t) {
    typename std::remove_cvref_t<T>::value_type;
    typename std::remove_cvref_t<T>::error_type;

    { t.has_value() } -> std::convertible_to<bool>;
    { t.error() } -> std::convertible_to<typename std::remove_cvref_t<T>::error_type>;
};
/**
 * @brief SPI Device Concept.
 *
 * @tparam T SPI Class.
 */
template<typename T>
concept SpiDevice =
    requires(T device, std::span<const uint8_t> tx_data, std::span<uint8_t> rx_data, uint32_t timeout_ms) {
        { device.transfer(tx_data, rx_data, timeout_ms) } -> std::same_as<bool>;
        { device.select() } -> std::same_as<void>;
        { device.deselect() } -> std::same_as<void>;
    };

/**
 * @brief Concept for writable registers (WO or RW).
 */
template<typename T>
concept WritableRegister = Register<T> && (T::access != Access::RO);

/**
 * @brief Concept for volatile registers.
 */
template<typename T>
concept VolatileRegister = Register<T> && std::is_same_v<typename T::type_t, Volatile>;

/**
 * @brief Concept for readable registers (RO or RW).
 */
template<typename T>
concept ReadableRegister = Register<T> && (T::access != Access::WO);

/**
 * @brief Concept for writable fields.
 *
 * Field must belong to a writable register.
 */
template<typename T>
concept WritableField = Field<T> && WritableRegister<typename T::register_t>;

/**
 * @brief
 */
template<typename T>
concept ReadableField = Field<T> && ReadableRegister<typename T::register_t>;
} // namespace tmcxx::core::concepts

#endif // TMCXX_CORE_CONCEPTS_HPP
