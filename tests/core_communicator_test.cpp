#include <gtest/gtest.h>

#include "mocks/mock_spi.hpp"

#include "tmcxx/chips/tmc5160_registers.hpp"
#include "tmcxx/features/core_communicator.hpp"
#include "tmcxx/helpers/error.hpp"

namespace tmcxx::features::test {

using namespace chip::tmc5160;

struct MockVolatileRegister : core::RegisterAddress<0x7F, core::Access::RW, core::Volatile>
{
    using mock_field_t = core::Field<MockVolatileRegister, 0, 8>;
};

class CoreCommunicatorTest : public ::testing::Test {
  protected:
    void SetUp() override
    {
        spi.reset();
    }

    ::tmcxx::test::MockSpi spi;
    CoreCommunicator<::tmcxx::test::MockSpi> comm{spi};
};

TEST_F(CoreCommunicatorTest, WriteReturnsTrueOnSuccess)
{
    EXPECT_TRUE(comm.write<VMAX>(1000U));
}

TEST_F(CoreCommunicatorTest, WriteReturnsFalseOnFailure)
{
    spi.set_next_transfer_failure(true);
    EXPECT_FALSE(comm.write<VMAX>(1000U));
}

TEST_F(CoreCommunicatorTest, WriteSelectsAndDeselectsChip)
{
    EXPECT_TRUE(comm.write<VMAX>(1000U));

    EXPECT_EQ(spi.get_select_count(), 1U);
    EXPECT_EQ(spi.get_deselect_count(), 1U);
    EXPECT_FALSE(spi.is_selected());
}

TEST_F(CoreCommunicatorTest, WriteUsesCorrectAddress)
{
    EXPECT_TRUE(comm.write<VMAX>(1234U));

    const auto& tx{spi.get_last_transaction()};
    constexpr uint8_t expected_addr{0x80U | VMAX::address};
    EXPECT_EQ(tx.tx_data[0], expected_addr);
}

TEST_F(CoreCommunicatorTest, WriteTransmitsValueBigEndian)
{
    constexpr uint32_t value{0x12345678U};
    EXPECT_TRUE(comm.write<VMAX>(value));

    const auto& tx{spi.get_last_transaction()};
    EXPECT_EQ(tx.tx_data[1], 0x12U);
    EXPECT_EQ(tx.tx_data[2], 0x34U);
    EXPECT_EQ(tx.tx_data[3], 0x56U);
    EXPECT_EQ(tx.tx_data[4], 0x78U);
}

TEST_F(CoreCommunicatorTest, WriteSends5Bytes)
{
    EXPECT_TRUE(comm.write<VMAX>(0U));

    const auto& tx{spi.get_last_transaction()};
    EXPECT_EQ(tx.tx_data.size(), 5U);
}

TEST_F(CoreCommunicatorTest, WriteToZeroValue)
{
    EXPECT_TRUE(comm.write<VMAX>(0U));

    const auto& tx{spi.get_last_transaction()};
    EXPECT_EQ(tx.tx_data[1], 0x00U);
    EXPECT_EQ(tx.tx_data[2], 0x00U);
    EXPECT_EQ(tx.tx_data[3], 0x00U);
    EXPECT_EQ(tx.tx_data[4], 0x00U);
}

TEST_F(CoreCommunicatorTest, WriteToMaxValue)
{
    EXPECT_TRUE(comm.write<VMAX>(0xFFFFFFFFU));

    const auto& tx{spi.get_last_transaction()};
    EXPECT_EQ(tx.tx_data[1], 0xFFU);
    EXPECT_EQ(tx.tx_data[2], 0xFFU);
    EXPECT_EQ(tx.tx_data[3], 0xFFU);
    EXPECT_EQ(tx.tx_data[4], 0xFFU);
}

TEST_F(CoreCommunicatorTest, ReadSelectsAndDeselectsTwice)
{
    spi.set_register_value(XACTUAL::address, 0x12345678U);
    [[maybe_unused]] auto val{comm.read<XACTUAL>()};

    EXPECT_EQ(spi.get_select_count(), 2U);
    EXPECT_EQ(spi.get_deselect_count(), 2U);
}

TEST_F(CoreCommunicatorTest, ReadUsesCorrectAddress)
{
    spi.set_register_value(XACTUAL::address, 0U);
    [[maybe_unused]] auto val = comm.read<XACTUAL>();

    const auto& transactions{spi.get_transactions()};
    ASSERT_GE(transactions.size(), 1U);

    constexpr uint8_t expected_addr{XACTUAL::address & 0x7FU};
    EXPECT_EQ(transactions[0].tx_data[0], expected_addr);
}

TEST_F(CoreCommunicatorTest, ReadReturnsCorrectValue)
{
    constexpr uint32_t expected{0xDEADBEEFU};
    spi.set_register_value(XACTUAL::address, expected);

    const auto result{comm.read<XACTUAL>()};

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expected);
}

TEST_F(CoreCommunicatorTest, ReadReturnsNulloptOnFailure)
{
    spi.set_next_transfer_failure(true);
    const auto result{comm.read<XACTUAL>()};

    EXPECT_FALSE(result.has_value());
}

TEST_F(CoreCommunicatorTest, ReadFromDifferentRegisters)
{
    EXPECT_TRUE(comm.write<VMAX>(0x11111111U));

    spi.set_register_value(XACTUAL::address, 0x22222222U);

    const auto vmax{comm.read<VMAX>()};
    const auto xactual{comm.read<XACTUAL>()};

    ASSERT_TRUE(vmax.has_value());
    ASSERT_TRUE(xactual.has_value());
    EXPECT_EQ(*vmax, 0x11111111U);
    EXPECT_EQ(*xactual, 0x22222222U);

    EXPECT_EQ(spi.get_transaction_count(), 3U);
}

TEST_F(CoreCommunicatorTest, WriteFieldSetsCorrectBits)
{
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_run_t>(16U));

    const auto written{spi.get_last_written_value(IHOLD_IRUN::address)};
    ASSERT_TRUE(written.has_value());

    const auto i_run_extracted{IHOLD_IRUN::i_run_t::extract(*written)};
    EXPECT_EQ(i_run_extracted, 16U);
}

TEST_F(CoreCommunicatorTest, WriteFieldPreservesOtherFields)
{
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_hold_t>(10U));

    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_run_t>(20U));

    const auto written{spi.get_last_written_value(IHOLD_IRUN::address)};
    ASSERT_TRUE(written.has_value());

    EXPECT_EQ(IHOLD_IRUN::i_hold_t::extract(*written), 10U);
    EXPECT_EQ(IHOLD_IRUN::i_run_t::extract(*written), 20U);
}

TEST_F(CoreCommunicatorTest, WriteFieldToMultipleFields)
{
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_hold_t>(5U));
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_run_t>(15U));
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_hold_delay_t>(10U));

    const auto written{spi.get_last_written_value(IHOLD_IRUN::address)};
    ASSERT_TRUE(written.has_value());

    EXPECT_EQ(IHOLD_IRUN::i_hold_t::extract(*written), 5U);
    EXPECT_EQ(IHOLD_IRUN::i_run_t::extract(*written), 15U);
    EXPECT_EQ(IHOLD_IRUN::i_hold_delay_t::extract(*written), 10U);
}

TEST_F(CoreCommunicatorTest, WriteFieldOverwritesPreviousValue)
{
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_run_t>(10U));
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_run_t>(20U));

    const auto written{spi.get_last_written_value(IHOLD_IRUN::address)};
    ASSERT_TRUE(written.has_value());

    EXPECT_EQ(IHOLD_IRUN::i_run_t::extract(*written), 20U);
}

TEST_F(CoreCommunicatorTest, ReadFieldExtractsCorrectly)
{
    const uint32_t reg_val{25U << 8U};

    EXPECT_TRUE(comm.write<IHOLD_IRUN>(reg_val));

    const auto result{comm.read_field<IHOLD_IRUN::i_run_t>()};

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 25U);
}

TEST_F(CoreCommunicatorTest, ReadFieldIgnoresOtherBits)
{
    EXPECT_TRUE(comm.write<IHOLD_IRUN>(0xFFFFFFFFU));

    const auto result{comm.read_field<IHOLD_IRUN::i_hold_t>()};

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 31U);
}

TEST_F(CoreCommunicatorTest, ReadFieldReturnsErrorOnFailure)
{
    spi.set_next_transfer_failure(true);

    const auto result{comm.read_field<MockVolatileRegister::mock_field_t>()};

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), helpers::ErrorCode::SPI_TRANSFER_FAILED);
}

TEST_F(CoreCommunicatorTest, GetShadowReturnsWrittenValue)
{
    constexpr uint32_t value{0x12345678U};
    EXPECT_TRUE(comm.write<VMAX>(value));

    const auto shadow{comm.get_shadow(VMAX::address)};
    ASSERT_TRUE(shadow.has_value());
    EXPECT_EQ(*shadow, value);
}

TEST_F(CoreCommunicatorTest, GetShadowForUnwrittenRegisterReturnsZero)
{
    const auto shadow{comm.get_shadow(VMAX::address)};
    ASSERT_TRUE(shadow.has_value());
    EXPECT_EQ(*shadow, 0U);
}

TEST_F(CoreCommunicatorTest, GetShadowForInvalidAddressReturnsError)
{
    const auto shadow{comm.get_shadow(200U)};

    EXPECT_FALSE(shadow.has_value());
    EXPECT_EQ(shadow.error(), helpers::ErrorCode::REGISTER_ACCESS_FAILED);
}

TEST_F(CoreCommunicatorTest, ShadowUpdatedAfterFieldWrite)
{
    EXPECT_TRUE(comm.write_field<IHOLD_IRUN::i_run_t>(20U));

    const auto shadow{comm.get_shadow(IHOLD_IRUN::address)};
    EXPECT_EQ(IHOLD_IRUN::i_run_t::extract(shadow.value()), 20U);
}

TEST_F(CoreCommunicatorTest, WriteSequenceIsCorrect)
{
    EXPECT_TRUE(comm.write<VMAX>(0x1234U));

    const auto& transactions{spi.get_transactions()};
    ASSERT_EQ(transactions.size(), 1U);
    EXPECT_TRUE(transactions[0].is_write_operation());
}

TEST_F(CoreCommunicatorTest, ReadSequenceHasTwoTransfers)
{
    spi.set_register_value(XACTUAL::address, 0x1234U);
    [[maybe_unused]] auto val = comm.read<XACTUAL>();

    const auto& transactions{spi.get_transactions()};
    ASSERT_EQ(transactions.size(), 2U);

    EXPECT_FALSE(transactions[0].is_write_operation());

    EXPECT_EQ(transactions[1].tx_data[0], 0U);
}

TEST_F(CoreCommunicatorTest, MultipleWritesToSameRegister)
{
    EXPECT_TRUE(comm.write<VMAX>(100U));
    EXPECT_TRUE(comm.write<VMAX>(200U));
    EXPECT_TRUE(comm.write<VMAX>(300U));

    EXPECT_EQ(spi.get_transaction_count(), 3U);

    const auto written{spi.get_last_written_value(VMAX::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 300U);
}

TEST_F(CoreCommunicatorTest, WriteToDifferentRegisters)
{
    EXPECT_TRUE(comm.write<VMAX>(1000U));
    EXPECT_TRUE(comm.write<AMAX>(500U));
    EXPECT_TRUE(comm.write<VSTART>(100U));

    EXPECT_TRUE(spi.get_last_written_value(VMAX::address).has_value());
    EXPECT_TRUE(spi.get_last_written_value(AMAX::address).has_value());
    EXPECT_TRUE(spi.get_last_written_value(VSTART::address).has_value());
}

TEST_F(CoreCommunicatorTest, WriteZeroToAllBits)
{
    EXPECT_TRUE(comm.write<IHOLD_IRUN>(0xFFFFFFFFU));
    EXPECT_TRUE(comm.write<IHOLD_IRUN>(0x00000000U));

    const auto written{spi.get_last_written_value(IHOLD_IRUN::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 0U);
}

TEST_F(CoreCommunicatorTest, WriteToRegisterAtAddress0x00)
{
    EXPECT_TRUE(comm.write<GCONF>(0x12U));

    const auto& tx{spi.get_last_transaction()};
    EXPECT_EQ(tx.tx_data[0], 0x80U);
}

TEST_F(CoreCommunicatorTest, WriteToHighAddressRegister)
{
    EXPECT_TRUE(comm.write<CHOPCONF>(0x12345678U));

    const auto& tx{spi.get_last_transaction()};
    constexpr uint8_t expected{0x80U | 0x6CU};
    EXPECT_EQ(tx.tx_data[0], expected);
}

} // namespace tmcxx::features::test
