#include <gtest/gtest.h>

#include "tmcxx/features/converter.hpp"

namespace tmcxx::features::test {

using namespace units::literals;

class ConverterTest : public ::testing::Test {
  protected:
    static constexpr auto default_clock{12.0_MHz};
    static constexpr auto default_steps{units::microsteps_t{200}};
    static constexpr auto default_rsense{75.0_mOhm};

    Converter converter{default_clock, default_steps, default_rsense};
};

TEST_F(ConverterTest, RpmToVmaxZeroReturnsZero)
{
    const auto result{converter.rpm_to_vmax(0.0_rpm)};
    EXPECT_EQ(result, 0U);
}

TEST_F(ConverterTest, RpmToVmaxPositiveValue)
{
    const auto result{converter.rpm_to_vmax(60.0_rpm)};

    EXPECT_GT(result, 70000U);
    EXPECT_LT(result, 75000U);
}

TEST_F(ConverterTest, RpmToVmaxHighSpeed)
{
    const auto result{converter.rpm_to_vmax(3000.0_rpm)};
    EXPECT_GT(result, 0U);
}

TEST_F(ConverterTest, RpmToVmaxIsMonotonic)
{
    const auto slow{converter.rpm_to_vmax(10.0_rpm)};
    const auto medium{converter.rpm_to_vmax(100.0_rpm)};
    const auto fast{converter.rpm_to_vmax(1000.0_rpm)};

    EXPECT_LT(slow, medium);
    EXPECT_LT(medium, fast);
}

TEST_F(ConverterTest, VmaxToRpmZeroReturnsZero)
{
    const auto result{converter.vmax_to_rpm(0U)};
    EXPECT_FLOAT_EQ(result.raw(), 0.0f);
}

TEST_F(ConverterTest, VmaxToRpmRoundTrip)
{
    const auto original{120.0_rpm};
    const auto vmax{converter.rpm_to_vmax(original)};
    const auto back{converter.vmax_to_rpm(vmax)};

    EXPECT_NEAR(back.raw(), original.raw(), 1.0f);
}

TEST_F(ConverterTest, VmaxToRpmIsMonotonic)
{
    const auto slow{converter.vmax_to_rpm(1000U)};
    const auto fast{converter.vmax_to_rpm(100000U)};

    EXPECT_LT(slow.raw(), fast.raw());
}

TEST_F(ConverterTest, CurrentToCsZeroReturnsZero)
{
    const auto result{converter.current_to_cs(0.0_A)};
    EXPECT_EQ(result, 0U);
}

TEST_F(ConverterTest, CurrentToCsMaxReturns31)
{
    const auto result{converter.current_to_cs(10.0_A)};
    EXPECT_EQ(result, 31U);
}

TEST_F(ConverterTest, CurrentToCsNormalRange)
{
    const auto result{converter.current_to_cs(1.5_A)};
    EXPECT_GE(result, 10U);
    EXPECT_LE(result, 20U);
}

TEST_F(ConverterTest, CurrentToCsIsMonotonic)
{
    const auto low{converter.current_to_cs(0.5_A)};
    const auto mid{converter.current_to_cs(1.0_A)};
    const auto high{converter.current_to_cs(2.0_A)};

    EXPECT_LE(low, mid);
    EXPECT_LE(mid, high);
}

TEST_F(ConverterTest, CurrentToCsNegativeClampedToZero)
{
    const auto result{converter.current_to_cs(units::current_t{-1.0f})};
    EXPECT_EQ(result, 0U);
}

TEST_F(ConverterTest, AccelToRegisterZeroReturnsMinimum)
{
    const auto result{converter.accel_to_register(0.0_pps2)};
    EXPECT_EQ(result, 1U);
}

TEST_F(ConverterTest, AccelToRegisterNormalValue)
{
    const auto result{converter.accel_to_register(1000.0_pps2)};
    EXPECT_GT(result, 0U);
    EXPECT_LE(result, 65535U);
}

TEST_F(ConverterTest, AccelToRegisterMaxClamped)
{
    const auto result{converter.accel_to_register(units::acceleration_t{1e10f})};
    EXPECT_EQ(result, 65535U);
}

TEST_F(ConverterTest, AccelToRegisterIsMonotonic)
{
    const auto slow{converter.accel_to_register(100.0_pps2)};
    const auto fast{converter.accel_to_register(10000.0_pps2)};

    EXPECT_LT(slow, fast);
}

TEST_F(ConverterTest, DurationToTzerowaitZeroReturnsZero)
{
    const auto result{converter.duration_to_tzerowait(0.0_s)};
    EXPECT_EQ(result, 0U);
}

TEST_F(ConverterTest, DurationToTzerowaitNormalValue)
{
    const auto result{converter.duration_to_tzerowait(100.0_ms)};
    EXPECT_GT(result, 0U);
}

TEST_F(ConverterTest, DurationToTzerowaitMaxClamped)
{
    const auto result{converter.duration_to_tzerowait(1000.0_s)};
    EXPECT_EQ(result, 65535U);
}

TEST_F(ConverterTest, DurationToTzerowaitIsMonotonic)
{
    const auto short_dur{converter.duration_to_tzerowait(10.0_ms)};
    const auto long_dur{converter.duration_to_tzerowait(100.0_ms)};

    EXPECT_LT(short_dur, long_dur);
}

TEST(ConverterConstexprTest, CanBeUsedAtCompileTime)
{
    constexpr Converter conv{12.0_MHz, units::microsteps_t{200}, 75.0_mOhm};

    constexpr auto vmax{conv.rpm_to_vmax(60.0_rpm)};
    static_assert(vmax > 0, "VMAX should be positive at compile time");

    constexpr auto cs{conv.current_to_cs(1.0_A)};
    static_assert(cs <= 31, "CS should be at most 31");

    SUCCEED();
}

class ConverterClockTest : public ::testing::TestWithParam<float> {
  protected:
    [[nodiscard]] units::frequency_t get_clock() const noexcept
    {
        return units::frequency_t{GetParam()};
    }
};

TEST_P(ConverterClockTest, VmaxScalesWithClock)
{
    const auto clock{get_clock()};
    Converter conv{clock, units::microsteps_t{200}, 75.0_mOhm};

    const auto vmax{conv.rpm_to_vmax(100.0_rpm)};

    EXPECT_GT(vmax, 0U);
}

INSTANTIATE_TEST_SUITE_P(ClockFrequencies,
    ConverterClockTest,
    ::testing::Values(8'000'000.0f,
        12'000'000.0f,
        16'000'000.0f
        ));

class ConverterRsenseTest : public ::testing::TestWithParam<float> {
};

TEST_P(ConverterRsenseTest, CurrentScalesWithRsense)
{
    const auto rsense_mohm{GetParam()};
    const auto rsense{units::resistance_t{rsense_mohm / 1000.0f}};

    Converter conv{12.0_MHz, units::microsteps_t{200}, rsense};

    const auto cs{conv.current_to_cs(1.0_A)};

    EXPECT_LE(cs, 31U);
}

INSTANTIATE_TEST_SUITE_P(SenseResistors,
    ConverterRsenseTest,
    ::testing::Values(50.0f,
        75.0f,
        100.0f,
        150.0f
        ));

} // namespace tmcxx::features::test
