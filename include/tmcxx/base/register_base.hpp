/************************************************************
 *  Project : TMCxx
 *  File    : register_base
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 ************************************************************/

#ifndef TMCXX_CORE_REGISTER_BASE_HPP
#define TMCXX_CORE_REGISTER_BASE_HPP

#include <cstdint>

namespace tmcxx::core {

/**
 * @brief Register access type enumeration.
 */
enum class Access : uint8_t {
    RO, // READ-ONLY
    WO, // WRITE-ONLY
    RW  // READ-WRITE
};

/**
 * @brief Base structure for register address definition.
 *
 * @tparam Addr Register address (0-127).
 * @tparam Acc Access type (RO, WO, RW).
 * @tparam UnitType Underlying data type (default: uint32_t).
 */
template<uint8_t Addr, Access Acc = Access::RW, typename UnitType = uint32_t>
struct RegisterAddress
{
    static constexpr uint8_t address{Addr};
    static constexpr Access access{Acc};

    using type_t = UnitType;
};

/**
 * @brief Structure for register field definition.
 *
 * @tparam RegAddr Parent register type.
 * @tparam StartBit Starting bit position (0-31).
 * @tparam Length Bit length of the field.
 * @tparam UnitType Underlying data type.
 */
template<typename RegAddr, uint8_t StartBit, uint8_t Length = 1U, typename UnitType = uint32_t>
struct Field
{
    using register_t = RegAddr;
    using type_t = UnitType;

    static constexpr auto max_bit_len{32U};

    static_assert(StartBit + Length <= max_bit_len, "Field exceeds 32-bit register width");
    static constexpr uint32_t mask{((uint32_t{1U} << Length) - 1U << StartBit)};

    static constexpr uint8_t shift{StartBit};

    uint32_t value{};

    /**
     * @brief Construct field with value.
     *
     * @param value Field value (will be shifted and masked).
     */
    explicit constexpr Field(uint32_t value)
        : value((value << shift) & mask)
    {
    }

    /**
     * @brief Extract field value from register.
     *
     * @param reg_val Full register value.
     *
     * @return Extracted field value.
     */
    [[nodiscard]] static constexpr uint32_t extract(uint32_t reg_val) noexcept
    {
        return (reg_val & mask) >> shift;
    }
};

/**
 * @brief Marker type for volatile registers.
 *
 * Registers inheriting from this are always read from hardware.
 */
struct Volatile
{
};

} // namespace tmcxx::core

#endif // TMCXX_CORE_REGISTER_BASE_HPP
