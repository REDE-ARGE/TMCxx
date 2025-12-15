cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(TMCxx
        VERSION 0.1.0
        DESCRIPTION "Modern C++20 Library for TMC5160 Stepper Drivers"
        LANGUAGES CXX
)

option(TMCXX_BUILD_TESTS "Build unit tests" OFF)
option(TMCXX_BUILD_EXAMPLES "Build example projects" OFF)
option(TMCXX_INSTALL "Generate install target" ON)
option(TMCXX_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(TMCXX_BUILD_TESTS ON CACHE BOOL "" FORCE)
    message(STATUS "TMCxx: Building as top-level project, tests enabled by default")
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GNUInstallDirs)

add_library(TMCxx INTERFACE)
add_library(tmc::xx ALIAS TMCxx)

target_compile_features(TMCxx INTERFACE cxx_std_20)

configure_file(
        "include/tmcxx/version.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/tmcxx/version.hpp"
        @ONLY
)

target_include_directories(TMCxx INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CompilerWarnings)
    include(StaticAnalyzers)
    include(cmake/WelcomeMessage.cmake)

    file(GLOB_RECURSE PUBLIC_HEADERS "include/*.hpp")
    add_executable(tmc_sandbox EXCLUDE_FROM_ALL
            ${PUBLIC_HEADERS}
            "demo/dummy_main.cpp"
    )
    target_link_libraries(tmc_sandbox PRIVATE TMCxx)
    set_project_warnings(tmc_sandbox)
endif ()

if (TMCXX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (TMCXX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (TMCXX_INSTALL)
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TMCxxConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/TMCxxConfig.cmake"
            INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TMCxx"
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/TMCxxConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/tmcxx.pc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/tmcxx.pc"
            @ONLY
    )

    install(DIRECTORY include/tmcxx
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(TARGETS TMCxx
            EXPORT TMCxxTargets
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT TMCxxTargets
            FILE TMCxxTargets.cmake
            NAMESPACE tmc::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TMCxx"
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/TMCxxConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/TMCxxConfigVersion.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TMCxx"
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/tmcxx.pc"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    )
endif ()

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(STATUS "")
    message(STATUS "TMCxx ${PROJECT_VERSION} Configuration:")
    message(STATUS "  Build tests:    ${TMCXX_BUILD_TESTS}")
    message(STATUS "  Build examples: ${TMCXX_BUILD_EXAMPLES}")
    message(STATUS "  Install:        ${TMCXX_INSTALL}")
    message(STATUS "  C++ Standard:   20")
    message(STATUS "")
endif ()