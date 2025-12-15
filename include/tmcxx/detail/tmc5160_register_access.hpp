/************************************************************
 *  Project : TMCxx
 *  File    : tmc5160_register_access
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 08.12.2025
 ************************************************************/

#ifndef TMCXX_TMC5160_REGISTER_ACCESS_HPP
#define TMCXX_TMC5160_REGISTER_ACCESS_HPP

#include "tmcxx/chips/tmc5160_registers.hpp"
#include "tmcxx/helpers/constants.hpp"
#include "tmcxx/helpers/error.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>

namespace tmcxx::detail {

/**
 * @brief Runtime register access interface for TMC5160.
 *
 * Provides lookup-table based register access for dynamic addressing.
 *
 * @tparam Bus Bus type (e.g., TMC5160Bus<TSpi>).
 */
template<typename Bus>
class TMC5160RegisterAccess {
  public:
    using regs_t = chip::tmc5160::RegAddress;

    /**
     * @brief Construct register accessor.
     *
     * @param bus Reference to bus (must outlive this object).
     */
    explicit TMC5160RegisterAccess(Bus& bus)
        : m_bus{bus}
    {
    }

    /**
     * @brief Read a register by compile-time address.
     *
     * @tparam Addr Register address enum value.
     * @return 32-bit register value or error.
     */
    template<regs_t Addr>
    [[nodiscard]] helpers::result_t<uint32_t> get_register()
    {
        [[maybe_unused]] constexpr auto reg_instance{chip::tmc5160::register_tuple.get<Addr>()};
        using reg_type_t = std::remove_cvref_t<decltype(reg_instance)>;

        return m_bus.template read<reg_type_t>();
    }

    /**
     * @brief Read a specific field from a register.
     *
     * @tparam FieldType Field type (e.g., IHOLD_IRUN::i_run_t).
     * @return Extracted field value or error.
     */
    template<typename FieldType>
    [[nodiscard]] helpers::result_t<uint32_t> get_field()
    {
        return m_bus.template read_field<FieldType>();
    }

    /**
     * @brief Read all registers into an array.
     *
     * @return Array of 128 register values if all reads succeed, else error.
     */
    [[nodiscard]] helpers::result_t<std::array<uint32_t, helpers::constant::tmc_register_count>> get_all_registers()
    {
        std::array<uint32_t, helpers::constant::tmc_register_count> registers{};
        std::size_t index{};
        bool success{true};

        std::apply(
            [this, &registers, &index, &success]<typename... Regs>([[maybe_unused]] const Regs&... regs) {
                ((success &=
                     [this, &registers, &index, success]() {
                         if (!success)
                         {
                             return false;
                         }

                         if (const auto read_val{m_bus.template read<std::decay_t<Regs>>()}; read_val)
                         {
                             registers[index++] = *read_val;
                             return true;
                         }
                         return false;
                     }()),
                    ...);
            },
            chip::tmc5160::register_tuple.fields);

        if (false == success)
        {
            return tl::unexpected(helpers::ErrorCode::SPI_TRANSFER_FAILED);
        }

        return registers;
    }

    /**
     * @brief Read a register by runtime address.
     *
     * @param reg_address Register address enum.
     * @return Register value, or error if address invalid.
     */
    [[nodiscard]] helpers::result_t<uint32_t> get_register_value(regs_t reg_address)
    {
        const uint8_t index{static_cast<uint8_t>(reg_address)};

        if (std::cmp_greater_equal(index, register_lookup_table.size())) [[unlikely]]
        {
            return tl::unexpected(helpers::ErrorCode::INVALID_PARAMETER);
        }

        const auto func_ptr{register_lookup_table[index]};
        if (nullptr == func_ptr) [[unlikely]]
        {
            return tl::unexpected(helpers::ErrorCode::INVALID_PARAMETER);
        }

        return std::invoke(func_ptr, this);
    }

    /**
     * @brief Write a register by compile-time address.
     *
     * @tparam Addr Register address enum value.
     *
     * @param value 32-bit value to write.
     * @return Result<void> (Success or ErrorCode).
     */
    template<regs_t Addr>
    [[nodiscard]] helpers::result_t<void> set_register(uint32_t value)
    {
        [[maybe_unused]] constexpr auto reg_instance{chip::tmc5160::register_tuple.get<Addr>()};
        using reg_type_t = std::remove_cvref_t<decltype(reg_instance)>;

        return m_bus.template write<reg_type_t>(value);
    }

    /**
     * @brief Write a register by runtime address.
     *
     * @param reg_address Register address enum.
     * @param value 32-bit value to write.
     *
     * @note Does nothing if address is invalid or register is read-only.
     * @return Result<void> (Success or ErrorCode).
     */
    [[nodiscard]] helpers::result_t<void> set_register_value(regs_t reg_address, uint32_t value)
    {
        const uint8_t index{static_cast<uint8_t>(reg_address)};

        if (std::cmp_greater_equal(index, register_set_lookup_table.size())) [[unlikely]]
        {
            return tl::unexpected(helpers::ErrorCode::INVALID_PARAMETER);
        }

        const auto func_ptr{register_set_lookup_table[index]};
        if (nullptr == func_ptr)
        {
            return tl::unexpected(helpers::ErrorCode::REGISTER_ACCESS_FAILED);
        }

        return std::invoke(func_ptr, this, value);
    }

  private:
    using register_read_method_t = helpers::result_t<uint32_t> (TMC5160RegisterAccess::*)();
    using setter_method_t = helpers::result_t<void> (TMC5160RegisterAccess::*)(uint32_t);

    Bus& m_bus{};

    template<regs_t Addr>
    [[nodiscard]] helpers::result_t<uint32_t> get_register_thunk()
    {
        return get_register<Addr>();
    }

    template<regs_t Addr>
    [[nodiscard]] helpers::result_t<void> set_register_thunk(uint32_t value)
    {
        return set_register<Addr>(value);
    }

    /**
     * @brief Create a lookup table for readable registers.
     */
    static consteval auto create_lookup_table() noexcept
    {
        std::array<register_read_method_t, helpers::constant::tmc_register_count> table{};

        std::apply(
            [&table]<typename... T0>(T0...) {
                ((table[std::decay_t<T0>::address] =
                         &TMC5160RegisterAccess::get_register_thunk<static_cast<regs_t>(std::decay_t<T0>::address)>),
                    ...);
            },
            chip::tmc5160::register_tuple.fields);

        return table;
    }

    /**
     * @brief Create a lookup table for write function (only not RO).
     */
    static consteval auto create_set_lookup_table() noexcept
    {
        std::array<setter_method_t, helpers::constant::tmc_register_count> table{};

        auto fill_entry = [&table]<typename Reg>() {
            using reg_t = std::decay_t<Reg>;

            if constexpr (core::Access::RO != reg_t::access)
            {
                constexpr auto addr{static_cast<regs_t>(reg_t::address)};
                table[reg_t::address] = &TMC5160RegisterAccess::set_register_thunk<addr>;
            }
            else
            {
                table[reg_t::address] = nullptr;
            }
        };

        std::apply(
            [&fill_entry]<typename... Regs>(Regs...) {
                (fill_entry.template operator()<std::decay_t<Regs>>(), ...);
            },
            chip::tmc5160::register_tuple.fields);

        return table;
    }

    static constexpr auto register_lookup_table{create_lookup_table()};
    static constexpr auto register_set_lookup_table{create_set_lookup_table()};
};

} // namespace tmcxx::detail

#endif // TMCXX_TMC5160_REGISTER_ACCESS_HPP
