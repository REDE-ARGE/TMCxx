#ifndef EXAMPLES_COMMON_DUMMY_SPI_HPP
#define EXAMPLES_COMMON_DUMMY_SPI_HPP

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <span>

class DummySpi {
  public:
    bool transfer(std::span<const uint8_t> tx_data, std::span<uint8_t> rx_data, [[maybe_unused]] uint32_t timeout_ms = 100)
    {
        std::cout << "[SPI] TX: ";
        for (const auto byte: tx_data)
        {
            std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        std::cout << "\n";

        std::ranges::fill(rx_data, 0x00);

        return true;
    }

    void select() noexcept
    {
        std::cout << "[SPI] CS Low\n";
    }

    void deselect() noexcept
    {
        std::cout << "[SPI] CS High\n";
    }
};

#endif // EXAMPLES_COMMON_DUMMY_SPI_HPP
