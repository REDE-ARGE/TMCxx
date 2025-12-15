#include <gtest/gtest.h>

#include <algorithm>
#include <ranges>

#include "mocks/mock_spi.hpp"
#include "tmcxx/tmc5160.hpp"

namespace tmcxx::test {

using namespace units::literals;

class TMC5160IntegrationTest : public ::testing::Test {
  protected:
    MockSpi spi;
    TMC5160<MockSpi>::Settings settings;

    void SetUp() override
    {
        spi.reset();
    }
};

TEST_F(TMC5160IntegrationTest, ApplyDefaultConfigurationSequence)
{
    TMC5160 driver{spi, settings};

    const auto result{driver.apply_default_configuration()};

    ASSERT_TRUE(result.has_value()) << "Configuration should succeed";

    const auto& txs{spi.get_transactions()};

    auto has_write{[&txs](uint8_t addr, uint32_t val) {
        const auto ret{std::ranges::any_of(txs, [&](const auto& elm) {
            return elm.is_write_operation() && elm.get_address() == addr && elm.get_write_value() == val;
        })};

        return ret;
    }};

    EXPECT_TRUE(has_write(0x2B, 100)) << "VSTOP write missing";
    EXPECT_TRUE(has_write(0x26, 10000)) << "AMAX write missing";
    EXPECT_TRUE(has_write(0x11, 10)) << "TWPOWER_DOWN write missing";

    const auto is_write_operation{[](const auto& elm) {
        return elm.is_write_operation();
    }};

    const auto ramp_mode_is_last{std::ranges::any_of(txs | std::views::filter(is_write_operation), [](const auto& elm) {
        return elm.is_write_operation() && elm.get_address() == 0x20 && elm.get_write_value() == 0;
    })};

    EXPECT_TRUE(ramp_mode_is_last) << "RAMPMODE must be the final configuration step";
}

TEST_F(TMC5160IntegrationTest, ApplySettingsFailsOnSpiError)
{
    TMC5160 driver{spi, settings};

    spi.set_next_transfer_failure(true);

    const auto result{driver.apply_settings()};

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), helpers::ErrorCode::SPI_TRANSFER_FAILED);
}

TEST_F(TMC5160IntegrationTest, MoveToSequence)
{
    TMC5160 driver{spi, settings};
    spi.reset();

    const auto result{driver.move_to(1000_steps, 500_rpm)};
    ASSERT_TRUE(result.has_value());

    const auto& txs{spi.get_transactions()};

    bool ramp_found{false};
    bool vstart_found{false};
    bool xtarget_found{false};

    const auto check_is_write_operation{[](const auto& elm) {
        return elm.is_write_operation();
    }};

    for (const auto& tx: txs | std::views::filter(check_is_write_operation))
    {
        if (tx.get_address() == 0x20 && tx.get_write_value() == 0)
        {
            ramp_found = true;
        }

        if (ramp_found && tx.get_address() == 0x23)
        {
            vstart_found = true;
        }

        if (vstart_found && tx.get_address() == 0x2D && tx.get_write_value() == 1000)
        {
            xtarget_found = true;
        }
    }

    EXPECT_TRUE(ramp_found) << "RAMPMODE write missing";
    EXPECT_TRUE(vstart_found) << "VSTART write missing";
    EXPECT_TRUE(xtarget_found) << "XTARGET write missing";
}

} // namespace tmcxx::test
