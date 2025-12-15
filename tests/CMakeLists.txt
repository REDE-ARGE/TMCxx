cmake_minimum_required(VERSION 3.20)

message(STATUS "Configuring Tests...")

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tmcxx_tests
        mocks/mock_spi.hpp
        units_test.cpp
        converter_test.cpp
        register_test.cpp
        core_communicator_test.cpp
        motion_test.cpp
        tmc5160_integration_test.cpp
        builder_test.cpp
)

target_link_libraries(tmcxx_tests
        PRIVATE
        TMCxx
        GTest::gtest_main
        GTest::gmock
)

target_compile_features(tmcxx_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tmcxx_tests)