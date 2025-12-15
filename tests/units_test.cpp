#include <gtest/gtest.h>

#include "tmcxx/helpers/units.hpp"

namespace tmcxx::units::test {

class QuantityTest : public ::testing::Test {
  protected:
    using test_quantity_t = Quantity<struct TestTag, float>;
};

TEST_F(QuantityTest, DefaultConstructorReturnsZero)
{
    constexpr test_quantity_t q{};
    EXPECT_FLOAT_EQ(q.raw(), 0.0f);
}

TEST_F(QuantityTest, ExplicitConstructorSetsValue)
{
    constexpr test_quantity_t q{42.5f};
    EXPECT_FLOAT_EQ(q.raw(), 42.5f);
}

TEST_F(QuantityTest, AdditionWorks)
{
    constexpr test_quantity_t a{10.0f};
    constexpr test_quantity_t b{5.0f};
    const auto result{a + b};
    EXPECT_FLOAT_EQ(result.raw(), 15.0f);
}

TEST_F(QuantityTest, SubtractionWorks)
{
    constexpr test_quantity_t a{10.0f};
    constexpr test_quantity_t b{3.0f};
    const auto result{a - b};
    EXPECT_FLOAT_EQ(result.raw(), 7.0f);
}

TEST_F(QuantityTest, MultiplicationByScalarWorks)
{
    constexpr test_quantity_t q{5.0f};
    const auto result{q * 3.0f};
    EXPECT_FLOAT_EQ(result.raw(), 15.0f);
}

TEST_F(QuantityTest, ScalarMultiplicationCommutative)
{
    constexpr test_quantity_t q{5.0f};
    const auto result1{q * 3.0f};
    const auto result2{3.0f * q};
    EXPECT_FLOAT_EQ(result1.raw(), result2.raw());
}

TEST_F(QuantityTest, DivisionByScalarWorks)
{
    constexpr test_quantity_t q{15.0f};
    const auto result{q / 3.0f};
    EXPECT_FLOAT_EQ(result.raw(), 5.0f);
}

TEST_F(QuantityTest, DivisionOfQuantitiesReturnsScalar)
{
    constexpr test_quantity_t a{15.0f};
    constexpr test_quantity_t b{3.0f};
    const float result{a / b};
    EXPECT_FLOAT_EQ(result, 5.0f);
}

TEST_F(QuantityTest, UnaryMinusWorks)
{
    constexpr test_quantity_t q{5.0f};
    const auto result{-q};
    EXPECT_FLOAT_EQ(result.raw(), -5.0f);
}

TEST_F(QuantityTest, UnaryPlusWorks)
{
    constexpr test_quantity_t q{5.0f};
    const auto result{+q};
    EXPECT_FLOAT_EQ(result.raw(), 5.0f);
}

TEST_F(QuantityTest, CompoundAdditionWorks)
{
    test_quantity_t q{10.0f};
    q += test_quantity_t{5.0f};
    EXPECT_FLOAT_EQ(q.raw(), 15.0f);
}

TEST_F(QuantityTest, CompoundSubtractionWorks)
{
    test_quantity_t q{10.0f};
    q -= test_quantity_t{3.0f};
    EXPECT_FLOAT_EQ(q.raw(), 7.0f);
}

TEST_F(QuantityTest, EqualityComparison)
{
    constexpr test_quantity_t a{5.0f};
    constexpr test_quantity_t b{5.0f};
    constexpr test_quantity_t c{6.0f};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(QuantityTest, LessThanComparison)
{
    constexpr test_quantity_t a{5.0f};
    constexpr test_quantity_t b{10.0f};

    EXPECT_LT(a, b);
    EXPECT_FALSE(b < a);
}

TEST_F(QuantityTest, GreaterThanComparison)
{
    constexpr test_quantity_t a{10.0f};
    constexpr test_quantity_t b{5.0f};

    EXPECT_GT(a, b);
    EXPECT_FALSE(b > a);
}

TEST_F(QuantityTest, LessThanOrEqualComparison)
{
    constexpr test_quantity_t a{5.0f};
    constexpr test_quantity_t b{5.0f};
    constexpr test_quantity_t c{10.0f};

    EXPECT_LE(a, b);
    EXPECT_LE(a, c);
}

TEST_F(QuantityTest, ThreeWayComparisonWorks)
{
    constexpr test_quantity_t a{5.0f};
    constexpr test_quantity_t b{10.0f};

    EXPECT_TRUE((a <=> b) < 0);
    EXPECT_TRUE((b <=> a) > 0);
    EXPECT_TRUE((a <=> a) == 0);
}

class LiteralsTest : public ::testing::Test {
};

TEST(LiteralsTest, RpmLiteralFloat)
{
    using namespace literals;
    constexpr auto val{100.5_rpm};

    static_assert(std::is_same_v<std::decay_t<decltype(val)>, rpm_t>);
    EXPECT_FLOAT_EQ(val.raw(), 100.5f);
}

TEST(LiteralsTest, RpmLiteralInt)
{
    using namespace literals;
    constexpr auto val{100_rpm};
    EXPECT_FLOAT_EQ(val.raw(), 100.0f);
}

TEST(LiteralsTest, CurrentAmpereLiteral)
{
    using namespace literals;
    constexpr auto val{2.5_A};
    EXPECT_FLOAT_EQ(val.raw(), 2.5f);
}

TEST(LiteralsTest, CurrentMilliAmpereLiteral)
{
    using namespace literals;
    constexpr auto val{500_mA};
    EXPECT_FLOAT_EQ(val.raw(), 0.5f);
}

TEST(LiteralsTest, FrequencyMHzLiteral)
{
    using namespace literals;
    constexpr auto val{12.0_MHz};
    EXPECT_FLOAT_EQ(val.raw(), 12'000'000.0f);
}

TEST(LiteralsTest, ResistanceOhmLiteral)
{
    using namespace literals;
    constexpr auto val{0.075_Ohm};
    EXPECT_FLOAT_EQ(val.raw(), 0.075f);
}

TEST(LiteralsTest, ResistanceMilliOhmLiteral)
{
    using namespace literals;
    constexpr auto val{75.0_mOhm};
    EXPECT_FLOAT_EQ(val.raw(), 0.075f);
}

TEST(LiteralsTest, TimeSecondsLiteral)
{
    using namespace literals;
    constexpr auto val{1.5_s};
    EXPECT_FLOAT_EQ(val.raw(), 1.5f);
}

TEST(LiteralsTest, TimeMillisecondsLiteral)
{
    using namespace literals;
    constexpr auto val{500_ms};
    EXPECT_FLOAT_EQ(val.raw(), 0.5f);
}

TEST(LiteralsTest, TimeMicrosecondsLiteral)
{
    using namespace literals;
    constexpr auto val{1000_us};
    EXPECT_FLOAT_EQ(val.raw(), 0.001f);
}

TEST(LiteralsTest, StepsLiteral)
{
    using namespace literals;
    constexpr auto val{200_steps};
    EXPECT_EQ(val.raw(), 200);
}

TEST(LiteralsTest, FactorLiteral)
{
    using namespace literals;
    constexpr auto val{0.75_factor};
    EXPECT_FLOAT_EQ(val.raw(), 0.75f);
}

TEST(LiteralsTest, PercentLiteral)
{
    using namespace literals;
    constexpr auto val{50_percent};
    EXPECT_FLOAT_EQ(val.raw(), 0.5f);
}

TEST(LiteralsTest, PercentLiteralFloat)
{
    using namespace literals;
    constexpr auto val{75.5_percent};
    EXPECT_FLOAT_EQ(val.raw(), 0.755f);
}

TEST(LiteralsTest, AccelerationLiteral)
{
    using namespace literals;
    constexpr auto val{1000_pps2};
    EXPECT_FLOAT_EQ(val.raw(), 1000.0f);
}

TEST(TypeSafetyTest, DifferentTagsAreIncompatible)
{
    static_assert(!std::is_same_v<rpm_t, current_t>);
    static_assert(!std::is_same_v<rpm_t, acceleration_t>);
    static_assert(!std::is_same_v<current_t, time_duration_t>);
    static_assert(!std::is_same_v<frequency_t, resistance_t>);

    SUCCEED();
}

TEST(TypeSafetyTest, MicrostepsIsInt32Based)
{
    static_assert(std::is_same_v<decltype(microsteps_t{}.raw()), int32_t>);

    using namespace literals;
    constexpr microsteps_t steps{-1000};
    EXPECT_EQ(steps.raw(), -1000);
}

TEST(EdgeCaseTest, ZeroQuantityOperations)
{
    using namespace literals;
    constexpr rpm_t zero{0.0f};
    constexpr rpm_t val{100.0f};

    EXPECT_FLOAT_EQ((zero + val).raw(), 100.0f);
    EXPECT_FLOAT_EQ((val - val).raw(), 0.0f);
    EXPECT_FLOAT_EQ((zero * 100.0f).raw(), 0.0f);
}

TEST(EdgeCaseTest, NegativeQuantity)
{
    constexpr rpm_t val{-50.0f};
    EXPECT_FLOAT_EQ(val.raw(), -50.0f);
    EXPECT_FLOAT_EQ((-val).raw(), 50.0f);
}

} // namespace tmcxx::units::test
