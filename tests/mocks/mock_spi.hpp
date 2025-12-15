#ifndef TMCXX_TESTS_MOCK_SPI_HPP
#define TMCXX_TESTS_MOCK_SPI_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

#include "tmcxx/base/concepts.hpp"

namespace tmcxx::test {

struct SpiTransaction
{
    std::vector<uint8_t> tx_data;
    std::vector<uint8_t> rx_data;
    bool is_write{false};

    [[nodiscard]] uint8_t get_address() const
    {
        if (tx_data.empty())
        {
            return 0;
        }
        return tx_data[0] & 0x7FU;
    }

    [[nodiscard]] bool is_write_operation() const
    {
        if (tx_data.empty())
        {
            return false;
        }
        return (tx_data[0] & 0x80U) != 0;
    }

    [[nodiscard]] uint32_t get_write_value() const
    {
        if (tx_data.size() < 5)
        {
            return 0;
        }
        return (static_cast<uint32_t>(tx_data[1]) << 24U) | (static_cast<uint32_t>(tx_data[2]) << 16U) |
               (static_cast<uint32_t>(tx_data[3]) << 8U) | (static_cast<uint32_t>(tx_data[4]));
    }
};

class MockSpi {
  public:
    MockSpi() = default;

    bool transfer(std::span<const uint8_t> tx_data, std::span<uint8_t> rx_data, [[maybe_unused]] uint32_t timeout_ms)
    {
        if (m_next_transfer_fails)
        {
            m_next_transfer_fails = false;
            return false;
        }

        SpiTransaction transaction{};
        transaction.tx_data.assign(tx_data.begin(), tx_data.end());

        if (!m_pending_response.empty())
        {
            const std::size_t copy_size{std::min(rx_data.size(), m_pending_response.size())};
            std::copy_n(m_pending_response.begin(), copy_size, rx_data.begin());
            transaction.rx_data.assign(rx_data.begin(), rx_data.end());
        }

        const bool is_write{!tx_data.empty() && (tx_data[0] & 0x80U) != 0};
        transaction.is_write = is_write;

        m_transactions.push_back(transaction);

        if (!is_write && !tx_data.empty())
        {
            const uint8_t addr{static_cast<uint8_t>(tx_data[0] & 0x7FU)};
            prepare_response(addr);
        }

        return true;
    }

    void select()
    {
        m_selected = true;
        m_select_count++;
    }

    void deselect()
    {
        m_selected = false;
        m_deselect_count++;
    }

    void set_register_value(uint8_t address, uint32_t value)
    {
        m_register_values[address] = value;
    }

    [[nodiscard]] const std::vector<SpiTransaction>& get_transactions() const noexcept
    {
        return m_transactions;
    }

    [[nodiscard]] const SpiTransaction& get_last_transaction() const noexcept
    {
        return m_transactions.back();
    }

    [[nodiscard]] std::size_t get_transaction_count() const noexcept
    {
        return m_transactions.size();
    }

    void clear_transactions() noexcept
    {
        m_transactions.clear();
    }

    [[nodiscard]] bool is_selected() const noexcept
    {
        return m_selected;
    }

    [[nodiscard]] std::size_t get_select_count() const noexcept
    {
        return m_select_count;
    }

    [[nodiscard]] std::size_t get_deselect_count() const noexcept
    {
        return m_deselect_count;
    }

    void reset()
    {
        m_transactions.clear();
        m_register_values.fill(0);
        m_pending_response.clear();
        m_selected = false;
        m_select_count = 0;
        m_deselect_count = 0;
    }

    [[nodiscard]] std::vector<SpiTransaction> find_writes_to(uint8_t address) const
    {
        std::vector<SpiTransaction> result{};
        for (const auto& transaction: m_transactions)
        {
            if (transaction.is_write_operation() && transaction.get_address() == address)
            {
                result.push_back(transaction);
            }
        }
        return result;
    }

    [[nodiscard]] std::optional<uint32_t> get_last_written_value(uint8_t address) const
    {
        for (const auto& m_transaction: std::ranges::reverse_view(m_transactions))
        {
            if (m_transaction.is_write_operation() && m_transaction.get_address() == address)
            {
                return m_transaction.get_write_value();
            }
        }
        return std::nullopt;
    }

    void set_next_transfer_failure(bool fail)
    {
        m_next_transfer_fails = fail;
    }

  private:
    void prepare_response(uint8_t address)
    {
        m_pending_response.clear();
        m_pending_response.push_back(0x00U);

        const uint32_t value{m_register_values[address]};
        m_pending_response.push_back(static_cast<uint8_t>((value >> 24U) & 0xFFU));
        m_pending_response.push_back(static_cast<uint8_t>((value >> 16U) & 0xFFU));
        m_pending_response.push_back(static_cast<uint8_t>((value >> 8U) & 0xFFU));
        m_pending_response.push_back(static_cast<uint8_t>(value & 0xFFU));
    }

    std::vector<SpiTransaction> m_transactions;
    std::array<uint32_t, 128> m_register_values{};
    std::vector<uint8_t> m_pending_response;
    bool m_selected{false};
    std::size_t m_select_count{0};
    std::size_t m_deselect_count{0};
    bool m_next_transfer_fails{false};
};

static_assert(core::concepts::SpiDevice<MockSpi>, "MockSpi must satisfy SpiDevice concept");

} // namespace tmcxx::test

#endif // TMCXX_TESTS_MOCK_SPI_HPP
