/************************************************************
 *  Project : TMCxx
 *  File    : core_communicator
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 21.11.2025
 ************************************************************/

#ifndef TMCXX_CORE_COMMUNICATOR_HPP
#define TMCXX_CORE_COMMUNICATOR_HPP

#include "tmcxx/base/concepts.hpp"
#include "tmcxx/base/register_base.hpp"
#include "tmcxx/helpers/constants.hpp"
#include "tmcxx/helpers/error.hpp"

#include <array>
#include <concepts>

namespace tmcxx::features {

/**
 * @brief Low-level SPI communication layer for TMC5160.
 *
 * Implements the 40-bit datagram protocol per TMC5160 datasheet.
 * Maintains shadow register cache for read-modify-write operations.
 *
 * @tparam TSpi SPI driver type satisfying hal::SpiDevice concept.
 */
template<core::concepts::SpiDevice TSpi>
class CoreCommunicator {
  public:
    explicit CoreCommunicator(TSpi& spi)
        : m_spi_device{spi}
    {
    }

    /**
     * @brief Write a value to register.
     *
     * @note If register is WO or RW firstly update the "Shadow" copy.
     * @return Result<void> (Success or ErrorCode).
     */
    template<typename RegType>
    requires core::concepts::WritableRegister<RegType>
    [[nodiscard]] helpers::result_t<void> write(std::unsigned_integral auto value)
    {
        m_register_cache[RegType::address] = value;

        return write_raw(RegType::address, value);
    }

    /**
     * @brief Write a field within a register (read-modify-write).
     *
     * Uses shadow cache to avoid SPI read operation.
     * @return Result<void> (Success or ErrorCode).
     */
    template<typename FieldType>
    requires core::concepts::WritableField<FieldType>
    [[nodiscard]] helpers::result_t<void> write_field(uint32_t field_val)
    {
        uint32_t current{m_register_cache[FieldType::register_t::address]};

        current &= ~FieldType::mask;
        current |= (field_val << FieldType::shift) & FieldType::mask;

        return write<typename FieldType::register_t>(current);
    }

    /**
     * @brief Read a register value.
     * @return Register value if successful, error otherwise.
     */
    template<typename RegType>
    requires core::concepts::Register<RegType>
    [[nodiscard]] helpers::result_t<uint32_t> read()
    {
        if constexpr (constexpr bool is_ro{(RegType::access == core::Access::RO)};
            core::concepts::VolatileRegister<RegType> || is_ro)
        {
            return read_raw(RegType::address);
        }
        else
        {
            return m_register_cache[RegType::address];
        }
    }

    /**
     * @brief Read the field register.
     * @return Field value if successful, nullopt otherwise.
     */
    template<typename FieldType>
    requires core::concepts::ReadableField<FieldType>
    [[nodiscard]] helpers::result_t<uint32_t> read_field()
    {
        const auto reg_val{read<typename FieldType::register_t>()};

        if (!reg_val.has_value()) [[unlikely]]
        {
            return tl::unexpected(reg_val.error());
        }

        return (*reg_val & FieldType::mask) >> FieldType::shift;
    }

    /**
     * @brief Get shadow register value.
     *
     * @param addr Register the address (0-127).
     * @return Cached value, or 0 if address invalid.
     */
    [[nodiscard]] helpers::result_t<uint32_t> get_shadow(uint8_t addr) const
    {
        if (addr >= m_register_cache.size()) [[unlikely]]
        {
            return tl::make_unexpected(helpers::ErrorCode::REGISTER_ACCESS_FAILED);
        }

        return m_register_cache[addr];
    }

  private:
    /**
     * @brief SPI Device member.
     */
    TSpi& m_spi_device;

    /**
     * @brief Register list.
     */
    std::array<uint32_t, helpers::constant::tmc_register_count> m_register_cache{};

    // LOW LEVEL SPI IMPLEMENTATION (Datasheet 4.1)

    static constexpr std::size_t rx_tx_buffer_size{5ULL};
    using rx_tx_buffer_t = std::array<uint8_t, rx_tx_buffer_size>;

    helpers::result_t<void> write_raw(uint8_t addr, uint32_t val)
    {
        rx_tx_buffer_t tx_buffer{};
        rx_tx_buffer_t rx_buffer{};

        const std::byte dummy{std::byte{addr} | std::byte{0x80U}};

        tx_buffer[0] = std::to_integer<uint8_t>(dummy);

        static constexpr std::size_t data_bytes_count{4ULL};
        static constexpr std::size_t bits_per_byte{8ULL};
        static constexpr uint32_t byte_mask{0xFFU};

        for (std::size_t idx{}; idx < data_bytes_count; idx++)
        {
            tx_buffer[1 + idx] = (val >> (bits_per_byte * (3 - idx))) & byte_mask;
        }

        if (const auto res{transfer(tx_buffer, rx_buffer)}; !res)
        {
            return res;
        }

        return {};
    }

    [[nodiscard]] helpers::result_t<uint32_t> read_raw(uint8_t addr)
    {
        rx_tx_buffer_t tx_buffer{};
        rx_tx_buffer_t rx_buffer{};

        const std::byte dummy{std::byte{addr} & std::byte{0x7F}};

        tx_buffer[0] = std::to_integer<uint8_t>(dummy);

        if (const auto res{transfer(tx_buffer, rx_buffer)}; !res) [[unlikely]]
        {
            return tl::unexpected(res.error());
        }

        tx_buffer[0] = 0;

        if (const auto res{transfer(tx_buffer, rx_buffer)}; !res) [[unlikely]]
        {
            return tl::unexpected(res.error());
        }

        // rx[0] -> SPI Status Byte
        // rx[1..4] -> 32-bit data (Big Endian)

        uint32_t result{};

        constexpr std::size_t iter_count{4ULL}; // buffer size is 5
        for (std::size_t idx{}; idx < iter_count; ++idx)
        {
            const std::size_t shift_count{(8 * (3 - idx))};
            const std::size_t value{(rx_buffer[1 + idx])};
            result |= (value << shift_count);
        }
        return result;
    }

    /**
     * @brief RAII for transfer data on SPI.
     */
    struct SPISelectGuard
    {
        TSpi& spi_device;

        explicit SPISelectGuard(TSpi& device)
            : spi_device{device}
        {
            spi_device.select();
        }

        ~SPISelectGuard()
        {
            spi_device.deselect();
        }

        /**
         * @brief Not declared move ctor & assign operator.
         */
        SPISelectGuard(const SPISelectGuard&) = delete;
        SPISelectGuard& operator=(const SPISelectGuard&) = delete;
    };

    [[nodiscard]] helpers::result_t<void> transfer(const rx_tx_buffer_t& tx_buffer, rx_tx_buffer_t& rx_buffer) noexcept(
        noexcept(m_spi_device.transfer(tx_buffer, rx_buffer, rx_tx_buffer_size)))
    {
        SPISelectGuard guard{m_spi_device};

        if (!m_spi_device.transfer(tx_buffer, rx_buffer, rx_tx_buffer_size))
        {
            return tl::unexpected(helpers::ErrorCode::SPI_TRANSFER_FAILED);
        }
        return {};
    }
};

} // namespace tmcxx::features

#endif // TMCXX_CORE_COMMUNICATOR_HPP
