#include "common/dummy_spi.hpp"

#include <tmcxx/builder/tmc_register_builder.hpp>

#include <iostream>

int main()
{
    std::cout << "--- TMCxx Example 03: Builder Usage ---\n";

    DummySpi spi_bus{};

    using namespace tmcxx::units::literals;

    auto motor{tmcxx::helpers::builder::TMC5160Builder{spi_bus}
            .clock_frequency(16.0_MHz)
            .sense_resistor(50.0_mOhm)
            .run_current(2.0_A)
            .hold_current(0.5_A)
            .stealth_chop_enabled(true)
            .full_steps(200_steps)
            .v_start(10_rpm)
            .v_max(600_rpm)
            .a_max(5000_pps2)
            .build()};

    if (motor.apply_settings())
    {
        std::cout << "Custom settings applied!\n";
    }
    else
    {
        std::cerr << "Failed to apply custom settings.\n";
    }

    (void)motor.rotate(500_rpm);

    return 0;
}
