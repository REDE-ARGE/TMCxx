#include <gtest/gtest.h>

#include "../include/tmcxx/base/register_base.hpp"
#include "tmcxx/chips/tmc5160_registers.hpp"

namespace tmcxx::chip::tmc5160::test {

class FieldMaskTest : public ::testing::Test {
};

TEST_F(FieldMaskTest, SingleBitFieldMask)
{
    using field_t = GCONF::recalibrate_t;

    EXPECT_EQ(field_t::mask, 0b1U);
    EXPECT_EQ(field_t::shift, 0U);
}

TEST_F(FieldMaskTest, MultiBitFieldMask)
{
    using field_t = IHOLD_IRUN::i_hold_t;

    EXPECT_EQ(field_t::mask, 0b11111U);
    EXPECT_EQ(field_t::shift, 0U);
}

TEST_F(FieldMaskTest, ShiftedFieldMask)
{
    using field_t = IHOLD_IRUN::i_run_t;

    EXPECT_EQ(field_t::mask, 0b11111'00000000U);
    EXPECT_EQ(field_t::shift, 8U);
}

TEST_F(FieldMaskTest, IholdDelayMask)
{
    using field_t = IHOLD_IRUN::i_hold_delay_t;

    constexpr uint32_t expected_mask{0xFU << 16U};
    EXPECT_EQ(field_t::mask, expected_mask);
    EXPECT_EQ(field_t::shift, 16U);
}

TEST_F(FieldMaskTest, ChopconfToffMask)
{
    using field_t = CHOPCONF::toff_t;

    EXPECT_EQ(field_t::mask, 0b1111U);
    EXPECT_EQ(field_t::shift, 0U);
}

TEST_F(FieldMaskTest, ChopconfMresMask)
{
    using field_t = CHOPCONF::mres_t;

    constexpr uint32_t expected_mask{0xFU << 24U};
    EXPECT_EQ(field_t::mask, expected_mask);
    EXPECT_EQ(field_t::shift, 24U);
}

class FieldExtractTest : public ::testing::Test {
};

TEST_F(FieldExtractTest, ExtractZeroFieldFromZeroRegister)
{
    constexpr uint32_t reg_val{0x00000000U};
    constexpr auto result{IHOLD_IRUN::i_hold_t::extract(reg_val)};
    EXPECT_EQ(result, 0U);
}

TEST_F(FieldExtractTest, ExtractMaxFieldValue)
{
    constexpr uint32_t reg_val{0x1FU};
    constexpr auto result{IHOLD_IRUN::i_hold_t::extract(reg_val)};
    EXPECT_EQ(result, 31U);
}

TEST_F(FieldExtractTest, ExtractShiftedField)
{
    constexpr uint32_t reg_val{0x00001000U};
    constexpr auto result{IHOLD_IRUN::i_run_t::extract(reg_val)};
    EXPECT_EQ(result, 16U);
}

TEST_F(FieldExtractTest, ExtractIgnoresOtherBits)
{
    constexpr uint32_t reg_val{0xFFFFFFFFU};
    constexpr auto result{IHOLD_IRUN::i_hold_t::extract(reg_val)};
    EXPECT_EQ(result, 31U);
}

TEST_F(FieldExtractTest, ExtractFromMixedRegister)
{
    constexpr uint32_t i_hold{10U};
    constexpr uint32_t i_run{20U};
    constexpr uint32_t i_hold_delay{5U};

    constexpr uint32_t reg_val{i_hold | (i_run << 8U) | (i_hold_delay << 16U)};

    EXPECT_EQ(IHOLD_IRUN::i_hold_t::extract(reg_val), i_hold);
    EXPECT_EQ(IHOLD_IRUN::i_run_t::extract(reg_val), i_run);
    EXPECT_EQ(IHOLD_IRUN::i_hold_delay_t::extract(reg_val), i_hold_delay);
}

class RegisterAddressTest : public ::testing::Test {
};

TEST_F(RegisterAddressTest, GconfAddress)
{
    EXPECT_EQ(GCONF::address, 0x00U);
}

TEST_F(RegisterAddressTest, IholdIrunAddress)
{
    EXPECT_EQ(IHOLD_IRUN::address, 0x10U);
}

TEST_F(RegisterAddressTest, ChopconfAddress)
{
    EXPECT_EQ(CHOPCONF::address, 0x6CU);
}

TEST_F(RegisterAddressTest, VmaxAddress)
{
    EXPECT_EQ(VMAX::address, 0x27U);
}

TEST_F(RegisterAddressTest, XactualAddress)
{
    EXPECT_EQ(XACTUAL::address, 0x21U);
}

class RegisterAccessTest : public ::testing::Test {
};

TEST_F(RegisterAccessTest, GconfIsRW)
{
    EXPECT_EQ(GCONF::access, core::Access::RW);
}

TEST_F(RegisterAccessTest, IholdIrunIsRW)
{
    EXPECT_EQ(IHOLD_IRUN::access, core::Access::RW);
}

TEST_F(RegisterAccessTest, VactualIsRO)
{
    EXPECT_EQ(VACTUAL::access, core::Access::RO);
}

TEST_F(RegisterAccessTest, XtargetIsWO)
{
    EXPECT_EQ(XTARGET::access, core::Access::WO);
}

TEST_F(RegisterAccessTest, GlobalScalerIsWO)
{
    EXPECT_EQ(GLOBAL_SCALER::access, core::Access::WO);
}

TEST_F(RegisterAccessTest, DrvStatusIsRO)
{
    EXPECT_EQ(DRV_STATUS::access, core::Access::RO);
}

class RegisterTupleTest : public ::testing::Test {
};

TEST_F(RegisterTupleTest, GetGconfByAddress)
{
    const auto& reg{register_tuple.get<RegAddress::GCONF>()};
    EXPECT_EQ(std::decay_t<decltype(reg)>::address, 0x00U);
}

TEST_F(RegisterTupleTest, GetIholdIrunByAddress)
{
    const auto& reg{register_tuple.get<RegAddress::IHOLD_IRUN>()};
    EXPECT_EQ(std::decay_t<decltype(reg)>::address, 0x10U);
}

TEST_F(RegisterTupleTest, GetChopconfByAddress)
{
    const auto& reg{register_tuple.get<RegAddress::CHOPCONF>()};
    EXPECT_EQ(std::decay_t<decltype(reg)>::address, 0x6CU);
}

TEST(VolatileTest, XactualIsRW)
{
    EXPECT_EQ(XACTUAL::access, core::Access::RW);
}

TEST(VolatileTest, VactualIsRO)
{
    EXPECT_EQ(VACTUAL::access, core::Access::RO);
}

TEST(VolatileTest, GconfIsNotRO)
{
    EXPECT_NE(GCONF::access, core::Access::RO);
}

TEST(VolatileTest, ChopconfIsRW)
{
    EXPECT_EQ(CHOPCONF::access, core::Access::RW);
}

TEST(RampModeTest, PositioningValue)
{
    EXPECT_EQ(static_cast<uint32_t>(RampModeType::POSITIONING), 0U);
}

TEST(RampModeTest, VelocityPosValue)
{
    EXPECT_EQ(static_cast<uint32_t>(RampModeType::VELOCITY_POS), 1U);
}

TEST(RampModeTest, VelocityNegValue)
{
    EXPECT_EQ(static_cast<uint32_t>(RampModeType::VELOCITY_NEG), 2U);
}

TEST(RampModeTest, HoldValue)
{
    EXPECT_EQ(static_cast<uint32_t>(RampModeType::HOLD), 3U);
}

TEST(ConstexprTest, FieldMaskIsConstexpr)
{
    constexpr auto mask{IHOLD_IRUN::i_run_t::mask};
    static_assert(mask == 0x1F00U);
    EXPECT_EQ(mask, 0x1F00U);
}

TEST(ConstexprTest, FieldShiftIsConstexpr)
{
    constexpr auto shift{IHOLD_IRUN::i_run_t::shift};
    static_assert(shift == 8U);
    EXPECT_EQ(shift, 8U);
}

TEST(ConstexprTest, ExtractIsConstexpr)
{
    constexpr uint32_t reg_val{0x00001000U};
    constexpr auto extracted{IHOLD_IRUN::i_run_t::extract(reg_val)};
    static_assert(extracted == 16U);
    EXPECT_EQ(extracted, 16U);
}

TEST(ConstexprTest, RegisterTupleIsConstexpr)
{
    [[maybe_unused]] constexpr auto& gconf{register_tuple.get<RegAddress::GCONF>()};
    static_assert(std::decay_t<decltype(gconf)>::address == 0x00U);
    SUCCEED();
}

} // namespace tmcxx::chip::tmc5160::test
