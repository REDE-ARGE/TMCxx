#include <gtest/gtest.h>

#include "mocks/mock_spi.hpp"

#include "tmcxx/builder/tmc_register_builder.hpp"

namespace tmcxx::helpers::builder::test {

using namespace units::literals;

class BuilderTest : public ::testing::Test {
  protected:
    ::tmcxx::test::MockSpi spi;
};

TEST_F(BuilderTest, BuilderReturnsReference)
{
    TMC5160Builder builder{spi};

    [[maybe_unused]] auto& ref{builder.clock_frequency(12.0_MHz)};

    EXPECT_EQ(&ref, &builder);
}

TEST_F(BuilderTest, FullChainCompiles)
{
    TMC5160Builder builder{spi};

    [[maybe_unused]] auto motor{builder.clock_frequency(12.0_MHz)
            .sense_resistor(75.0_mOhm)
            .run_current(1.5_A)
            .hold_current(0.5_A)
            .hold_delay(6)
            .power_down_delay(10)
            .v_start(10.0_rpm)
            .v_stop(5.0_rpm)
            .v_transition(100.0_rpm)
            .a_start(500.0_pps2)
            .a_max(2000.0_pps2)
            .d_max(2000.0_pps2)
            .d_stop(500.0_pps2)
            .stealth_chop_enabled(true)
            .toff(3)
            .hysteresis(4, 1)
            .blank_time(2)
            .build()};

    SUCCEED();
}

TEST_F(BuilderTest, MinimalBuild)
{
    TMC5160Builder builder{spi};

    [[maybe_unused]] auto motor{builder.build()};

    SUCCEED();
}

TEST_F(BuilderTest, ClockFrequencyCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.clock_frequency(16.0_MHz);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, ClockFrequencyChaining)
{
    TMC5160Builder builder{spi};

    [[maybe_unused]] auto motor{builder.clock_frequency(8.0_MHz).clock_frequency(12.0_MHz).build()};

    SUCCEED();
}

TEST_F(BuilderTest, SenseResistorCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.sense_resistor(100.0_mOhm);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, RunCurrentCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.run_current(2.0_A);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, HoldCurrentCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.hold_current(0.5_A);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, HoldDelayCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.hold_delay(10);

    auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, PowerDownDelayCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.power_down_delay(20);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, VStartCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.v_start(5.0_rpm);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, VStopCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.v_stop(2.0_rpm);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, VTransitionCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.v_transition(50.0_rpm);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, AStartCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.a_start(100.0_pps2);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, AMaxCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.a_max(1000.0_pps2);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, DMaxCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.d_max(1000.0_pps2);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, DStopCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.d_stop(100.0_pps2);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, StealthChopCanBeEnabled)
{
    TMC5160Builder builder{spi};

    builder.stealth_chop_enabled(true);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, StealthChopCanBeDisabled)
{
    TMC5160Builder builder{spi};

    builder.stealth_chop_enabled(false);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, ToffCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.toff(5);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, HysteresisCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.hysteresis(5, 2);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, HysteresisNegativeEnd)
{
    TMC5160Builder builder{spi};

    builder.hysteresis(4, -2);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, BlankTimeCanBeSet)
{
    TMC5160Builder builder{spi};

    builder.blank_time(3);

    [[maybe_unused]] auto motor{builder.build()};
    SUCCEED();
}

TEST_F(BuilderTest, BuilderIsConstexpr)
{
    TMC5160Builder builder{spi};

    auto& b1 = builder.clock_frequency(12.0_MHz);
    auto& b2 = b1.sense_resistor(75.0_mOhm);
    auto& b3 = b2.run_current(1.0_A);

    EXPECT_EQ(&b1, &builder);
    EXPECT_EQ(&b2, &builder);
    EXPECT_EQ(&b3, &builder);
}

TEST_F(BuilderTest, BuildReturnsValidMotor)
{
    TMC5160Builder builder{spi};

    [[maybe_unused]] auto motor{builder.build()};

    EXPECT_TRUE(motor.stop().has_value());

    SUCCEED();
}

TEST_F(BuilderTest, MultipleBuildsFromSameBuilder)
{
    TMC5160Builder builder{spi};
    builder.clock_frequency(12.0_MHz).sense_resistor(75.0_mOhm);

    auto motor1{builder.build()};
    auto motor2{builder.build()};

    EXPECT_TRUE(motor1.stop().has_value());
    EXPECT_TRUE(motor2.stop().has_value());

    SUCCEED();
}

TEST_F(BuilderTest, ModifyBuilderAfterBuild)
{
    TMC5160Builder builder{spi};
    builder.clock_frequency(12.0_MHz);

    auto motor1{builder.build()};

    builder.clock_frequency(16.0_MHz);

    auto motor2{builder.build()};

    EXPECT_TRUE(motor1.stop().has_value());
    EXPECT_TRUE(motor2.stop().has_value());

    SUCCEED();
}

TEST_F(BuilderTest, ZeroCurrentValues)
{
    TMC5160Builder builder{spi};

    auto motor{builder.run_current(0.0_A).hold_current(0.0_A).build()};

    SUCCEED();
}

TEST_F(BuilderTest, ZeroVelocityValues)
{
    TMC5160Builder builder{spi};

    [[maybe_unused]] auto motor{builder.v_start(0.0_rpm).v_stop(0.0_rpm).v_transition(0.0_rpm).build()};

    SUCCEED();
}

TEST_F(BuilderTest, ZeroAccelerationValues)
{
    TMC5160Builder builder{spi};

    [[maybe_unused]] auto motor{builder.a_start(0.0_pps2).a_max(0.0_pps2).d_max(0.0_pps2).d_stop(0.0_pps2).build()};

    SUCCEED();
}

} // namespace tmcxx::helpers::builder::test
