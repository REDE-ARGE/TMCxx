#include "common/dummy_spi.hpp"

#include <tmcxx/tmc5160.hpp>

#include <iostream>

int main()
{
    std::cout << "--- TMCxx Example 01: Simple Rotation ---\n";

    DummySpi spi_bus{};

    tmcxx::TMC5160<DummySpi>::Settings settings{};

    tmcxx::TMC5160 driver{spi_bus, settings};

    if (driver.apply_default_configuration())
    {
        std::cout << "Configuration applied successfully.\n";
    }
    else
    {
        std::cerr << "Failed to apply configuration!\n";
        return -1;
    }

    std::cout << "Rotating at 120 RPM...\n";
    using namespace tmcxx::units::literals;
    if (!driver.rotate(120_rpm))
    {
        std::cerr << "Rotation failed!\n";
        return -1;
    }

    return 0;
}
