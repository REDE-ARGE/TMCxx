/************************************************************
 *  Project : TMCxx
 *  File    : tmc5160_bus
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 11.12.2025
 ************************************************************/

#ifndef TMCXX_TMC5160_BUS_HPP
#define TMCXX_TMC5160_BUS_HPP

#include "tmcxx/base/concepts.hpp"
#include "tmcxx/features/core_communicator.hpp"
#include "tmcxx/helpers/error.hpp"

#include <cstdint>

namespace tmcxx::detail {

/**
 * @brief SPI bus abstraction layer for TMC5160.
 *
 * Wraps CoreCommunicator to provide a cleaner interface for register access.
 *
 * @tparam TSpi SPI device type satisfying hal::SpiDevice concept.
 */
template<core::concepts::SpiDevice TSpi>
class TMC5160Bus {
  public:
    /**
     * @brief Construct bus with SPI device reference.
     *
     * @param spi Reference to SPI device (must outlive this object).
     */
    explicit TMC5160Bus(TSpi& spi)
        : m_core{spi}
    {
    }

    /**
     * @brief Write a 32-bit value to a register.
     *
     * @tparam Reg Register type from tmc5160_registers.hpp.
     * @param value Value to write.
     * @return Result<void> (Success or ErrorCode).
     */
    template<typename Reg>
    [[nodiscard]] helpers::result_t<void> write(uint32_t value)
    {
        return m_core.template write<Reg>(value);
    }

    /**
     * @brief Read a 32-bit value from a register.
     *
     * @tparam Reg Register type from tmc5160_registers.hpp.
     * @return Register value or error.
     */
    template<typename Reg>
    [[nodiscard]] helpers::result_t<uint32_t> read()
    {
        return m_core.template read<Reg>();
    }

    /**
     * @brief Write a value to a specific field within a register.
     *
     * Uses shadow register for read-modify-write operation.
     *
     * @tparam Field Field type (e.g., IHOLD_IRUN::i_run_t).
     * @param value Field value to write.
     * @return Result<void> (Success or ErrorCode).
     */
    template<typename Field>
    [[nodiscard]] helpers::result_t<void> write_field(uint32_t value)
    {
        return m_core.template write_field<Field>(value);
    }

    /**
     * @brief Get mutable reference to underlying CoreCommunicator.
     *
     * @return Reference to CoreCommunicator.
     */
    [[nodiscard]] features::CoreCommunicator<TSpi>& core() noexcept
    {
        return m_core;
    }

    /**
     * @brief Get const reference to underlying CoreCommunicator.
     *
     * @return Const reference to CoreCommunicator.
     */
    [[nodiscard]] const features::CoreCommunicator<TSpi>& core() const noexcept
    {
        return m_core;
    }

  private:
    features::CoreCommunicator<TSpi> m_core;
};

} // namespace tmcxx::detail

#endif // TMCXX_TMC5160_BUS_HPP
