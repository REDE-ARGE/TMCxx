#include "common/dummy_spi.hpp"

#include <tmcxx/tmc5160.hpp>

#include <iostream>

int main()
{
    std::cout << "--- TMCxx Example 02: Position Control ---\n";

    DummySpi spi_bus{};
    tmcxx::TMC5160<DummySpi>::Settings settings{};
    tmcxx::TMC5160 driver{spi_bus, settings};

    if (!driver.apply_default_configuration())
    {
        std::cerr << "Configuration failed!\n";
        return -1;
    }

    using namespace tmcxx::units::literals;

    std::cout << "Moving to position 50000...\n";
    if (!driver.move_to(50000_steps, 300_rpm))
    {
        std::cerr << "Move 1 failed!\n";
        return -1;
    }

    if (const auto pos{driver.get_actual_motor_position()}; pos.has_value())
    {
        std::cout << "Current Position: " << *pos << "\n";
    }

    std::cout << "Returning to zero...\n";
    if (!driver.move_to(0_steps, 500_rpm))
    {
        std::cerr << "Return to zero failed!\n";
        return -1;
    }

    return 0;
}
