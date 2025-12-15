#include <gtest/gtest.h>

#include "mocks/mock_spi.hpp"

#include "tmcxx/chips/tmc5160_registers.hpp"
#include "tmcxx/detail/tmc5160_bus.hpp"
#include "tmcxx/detail/tmc5160_motion.hpp"
#include "tmcxx/features/converter.hpp"
#include "tmcxx/helpers/error.hpp"

namespace tmcxx::detail::test {

using namespace chip::tmc5160;
using namespace units::literals;

class MotionTest : public ::testing::Test {
  protected:
    void SetUp() override
    {
        spi.reset();
    }

    ::tmcxx::test::MockSpi spi{};
    TMC5160Bus<::tmcxx::test::MockSpi> bus{spi};
    features::Converter converter{12.0_MHz, units::microsteps_t{200}, 75.0_mOhm};
    TMC5160Motion<TMC5160Bus<::tmcxx::test::MockSpi>> motion{bus, converter};
};

TEST_F(MotionTest, StopWritesZeroToVmax)
{
    EXPECT_TRUE(motion.stop());

    const auto written{spi.get_last_written_value(VMAX::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 0U);
}

TEST_F(MotionTest, RotatePositiveVelocitySetsPositiveMode)
{
    EXPECT_TRUE(motion.rotate(100.0_rpm));

    const auto mode{spi.get_last_written_value(RAMPMODE::address)};
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(*mode, static_cast<uint32_t>(RampModeType::VELOCITY_POS));
}

TEST_F(MotionTest, RotateNegativeVelocitySetsNegativeMode)
{
    EXPECT_TRUE(motion.rotate(-100.0_rpm));

    const auto writes{spi.find_writes_to(RAMPMODE::address)};
    ASSERT_FALSE(writes.empty());

    const auto mode_value{writes.back().get_write_value()};
    EXPECT_EQ(mode_value, static_cast<uint32_t>(RampModeType::VELOCITY_NEG));
}

TEST_F(MotionTest, RotateSetsVmax)
{
    EXPECT_TRUE(motion.rotate(60.0_rpm));

    const auto vmax{spi.get_last_written_value(VMAX::address)};
    ASSERT_TRUE(vmax.has_value());
    EXPECT_GT(*vmax, 0U);
}

TEST_F(MotionTest, RotateWithNegativeVelocityUsesAbsoluteVmax)
{
    EXPECT_TRUE(motion.rotate(-60.0_rpm));
    const auto vmax_neg{spi.get_last_written_value(VMAX::address)};

    spi.clear_transactions();
    EXPECT_TRUE(motion.rotate(60.0_rpm));
    const auto vmax_pos{spi.get_last_written_value(VMAX::address)};

    EXPECT_EQ(*vmax_neg, *vmax_pos);
}

TEST_F(MotionTest, RotateZeroVelocitySetsPositiveMode)
{
    EXPECT_TRUE(motion.rotate(0.0_rpm));

    const auto mode{spi.get_last_written_value(RAMPMODE::address)};
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(*mode, static_cast<uint32_t>(RampModeType::VELOCITY_POS));
}

TEST_F(MotionTest, SetGlobalScalingZero)
{
    EXPECT_TRUE(motion.set_global_scaling(0.0_factor));

    const auto written{spi.get_last_written_value(GLOBAL_SCALER::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 0U);
}

TEST_F(MotionTest, SetGlobalScalingFull)
{
    EXPECT_TRUE(motion.set_global_scaling(1.0_factor));

    const auto written{spi.get_last_written_value(GLOBAL_SCALER::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 256U);
}

TEST_F(MotionTest, SetGlobalScalingHalf)
{
    EXPECT_TRUE(motion.set_global_scaling(0.5_factor));

    const auto written{spi.get_last_written_value(GLOBAL_SCALER::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 128U);
}

TEST_F(MotionTest, SetGlobalScalingClampedAboveOne)
{
    EXPECT_TRUE(motion.set_global_scaling(units::factor_t{2.0f}));

    const auto written{spi.get_last_written_value(GLOBAL_SCALER::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 256U);
}

TEST_F(MotionTest, SetGlobalScalingClampedBelowZero)
{
    EXPECT_TRUE(motion.set_global_scaling(units::factor_t{-0.5f}));

    const auto written{spi.get_last_written_value(GLOBAL_SCALER::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 0U);
}

TEST_F(MotionTest, MoveToSetsPositioningMode)
{
    EXPECT_TRUE(motion.move_to(units::microsteps_t{1000}, 100.0_rpm));

    const auto mode{spi.get_last_written_value(RAMPMODE::address)};
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(*mode, static_cast<uint32_t>(RampModeType::POSITIONING));
}

TEST_F(MotionTest, MoveToWritesXtarget)
{
    EXPECT_TRUE(motion.move_to(units::microsteps_t{5000}, 100.0_rpm));

    const auto target{spi.get_last_written_value(XTARGET::address)};
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(*target, 5000U);
}

TEST_F(MotionTest, SetRampModePositioning)
{
    EXPECT_TRUE(motion.set_ramp_mode(RampModeType::POSITIONING));

    const auto written{spi.get_last_written_value(RAMPMODE::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 0U);
}

TEST_F(MotionTest, SetRampModeVelocityPos)
{
    EXPECT_TRUE(motion.set_ramp_mode(RampModeType::VELOCITY_POS));

    const auto written{spi.get_last_written_value(RAMPMODE::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 1U);
}

TEST_F(MotionTest, SetRampModeVelocityNeg)
{
    EXPECT_TRUE(motion.set_ramp_mode(RampModeType::VELOCITY_NEG));

    const auto written{spi.get_last_written_value(RAMPMODE::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 2U);
}

TEST_F(MotionTest, SetRampModeHold)
{
    EXPECT_TRUE(motion.set_ramp_mode(RampModeType::HOLD));

    const auto written{spi.get_last_written_value(RAMPMODE::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 3U);
}

TEST_F(MotionTest, SetMaxVelocityWritesToVmax)
{
    EXPECT_TRUE(motion.set_max_velocity(120.0_rpm));

    const auto written{spi.get_last_written_value(VMAX::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_GT(*written, 0U);
}

TEST_F(MotionTest, SetStartSpeedWritesToVstart)
{
    EXPECT_TRUE(motion.set_start_speed(10.0_rpm));

    const auto written{spi.get_last_written_value(VSTART::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_GT(*written, 0U);
}

TEST_F(MotionTest, SetRampTransitionVelocityWritesToV1)
{
    EXPECT_TRUE(motion.set_ramp_transition_velocity(50.0_rpm));

    const auto written{spi.get_last_written_value(V1::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_GT(*written, 0U);
}

TEST_F(MotionTest, SetStopVelocityWritesToVstop)
{
    EXPECT_TRUE(motion.set_stop_velocity(5.0_rpm));

    const auto written{spi.get_last_written_value(VSTOP::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_GE(*written, 1U);
}

TEST_F(MotionTest, SetStopVelocityMinimumIsOne)
{
    EXPECT_TRUE(motion.set_stop_velocity(0.0_rpm));

    const auto written{spi.get_last_written_value(VSTOP::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_GE(*written, 1U);
}

TEST_F(MotionTest, SetLinearAccelerationWritesToAmax)
{
    EXPECT_TRUE(motion.set_linear_acceleration(1000.0_pps2, 1000.0_pps2));

    const auto amax{spi.get_last_written_value(AMAX::address)};
    ASSERT_TRUE(amax.has_value());
    EXPECT_GT(*amax, 0U);
}

TEST_F(MotionTest, SetLinearAccelerationWritesToDmax)
{
    EXPECT_TRUE(motion.set_linear_acceleration(1000.0_pps2, 2000.0_pps2));

    const auto dmax{spi.get_last_written_value(DMAX::address)};
    ASSERT_TRUE(dmax.has_value());
    EXPECT_GT(*dmax, 0U);
}

TEST_F(MotionTest, SetLinearAccelerationWritesToA1AndD1)
{
    EXPECT_TRUE(motion.set_linear_acceleration(500.0_pps2, 500.0_pps2));

    EXPECT_TRUE(spi.get_last_written_value(A1::address).has_value());
    EXPECT_TRUE(spi.get_last_written_value(D1::address).has_value());
}

TEST_F(MotionTest, SetAdvancedAccelerationWritesAllRegisters)
{
    EXPECT_TRUE(motion.set_advanced_acceleration(100.0_pps2, 500.0_pps2, 400.0_pps2, 100.0_pps2));

    EXPECT_TRUE(spi.get_last_written_value(A1::address).has_value());
    EXPECT_TRUE(spi.get_last_written_value(AMAX::address).has_value());
    EXPECT_TRUE(spi.get_last_written_value(DMAX::address).has_value());
    EXPECT_TRUE(spi.get_last_written_value(D1::address).has_value());
}

TEST_F(MotionTest, SetStandstillWaitWritesToTzerowait)
{
    EXPECT_TRUE(motion.set_standstill_wait(100.0_ms));

    const auto written{spi.get_last_written_value(TZEROWAIT::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_GT(*written, 0U);
}

TEST_F(MotionTest, SetActualMotorPositionWritesToXactual)
{
    EXPECT_TRUE(motion.set_actual_motor_position(units::microsteps_t{12345}));

    const auto written{spi.get_last_written_value(XACTUAL::address)};
    ASSERT_TRUE(written.has_value());
    EXPECT_EQ(*written, 12345U);
}

TEST_F(MotionTest, GetActualMotorPositionReadsXactual)
{
    spi.set_register_value(XACTUAL::address, 54321U);

    const auto position{motion.get_actual_motor_position()};

    ASSERT_TRUE(position.has_value());
    EXPECT_EQ(*position, 54321);
}

TEST_F(MotionTest, GetActualMotorPositionHandlesSignedValue)
{
    spi.set_register_value(XACTUAL::address, 0xFFFFFFFFU);

    const auto position{motion.get_actual_motor_position()};

    ASSERT_TRUE(position.has_value());
    EXPECT_EQ(*position, -1);
}

TEST_F(MotionTest, SetIrunWritesToField)
{
    EXPECT_TRUE(motion.set_irun(1.5_A));

    const auto written{spi.get_last_written_value(IHOLD_IRUN::address)};
    ASSERT_TRUE(written.has_value());

    const auto irun{IHOLD_IRUN::i_run_t::extract(*written)};
    EXPECT_GT(irun, 0U);
    EXPECT_LE(irun, 31U);
}

TEST_F(MotionTest, SetIholdWritesToFields)
{
    EXPECT_TRUE(motion.set_ihold(1.0_A));

    const auto written{spi.get_last_written_value(IHOLD_IRUN::address)};
    ASSERT_TRUE(written.has_value());

    const auto ihold{IHOLD_IRUN::i_hold_t::extract(*written)};
    EXPECT_GT(ihold, 0U);
    EXPECT_LE(ihold, 31U);

    const auto delay{IHOLD_IRUN::i_hold_delay_t::extract(*written)};
    EXPECT_EQ(delay, 6U);
}

TEST_F(MotionTest, GetActualVelocityReadsVactual)
{
    spi.set_register_value(VACTUAL::address, 0x10000U);

    const auto velocity{motion.get_actual_velocity()};

    ASSERT_TRUE(velocity.has_value());
    EXPECT_GT(velocity->raw(), 0.0f);
}

TEST_F(MotionTest, GetActualVelocityHandles24BitSigned)
{
    spi.set_register_value(VACTUAL::address, 0x800000U);

    const auto velocity{motion.get_actual_velocity()};

    ASSERT_TRUE(velocity.has_value());
    EXPECT_GE(velocity->raw(), 0.0f);
}

TEST_F(MotionTest, SetStealthChopEnable)
{
    EXPECT_TRUE(motion.set_stealth_chop(true));

    const auto written{spi.get_last_written_value(CHOPCONF::address)};
    ASSERT_TRUE(written.has_value());

    const auto chm{CHOPCONF::chm_t::extract(*written)};
    EXPECT_EQ(chm, 1U);
}

TEST_F(MotionTest, SetStealthChopDisable)
{
    EXPECT_TRUE(motion.set_stealth_chop(false));

    const auto written{spi.get_last_written_value(CHOPCONF::address)};
    ASSERT_TRUE(written.has_value());

    const auto chm{CHOPCONF::chm_t::extract(*written)};
    EXPECT_EQ(chm, 0U);
}

} // namespace tmcxx::detail::test
