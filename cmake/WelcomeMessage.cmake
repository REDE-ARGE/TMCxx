
# --- ASCII Art Banner ---
message(STATUS "")
message(STATUS "  _______  __  __   _____              ")
message(STATUS " |__   __||  \\/  | / ____|             ")
message(STATUS "    | |   | \\  / || |      __  __      ")
message(STATUS "    | |   | |\\/| || |      \\ \\/ /      ")
message(STATUS "    | |   | |  | || |____   >  <       ")
message(STATUS "    |_|   |_|  |_| \\_____| /_/\\_\\      ")
message(STATUS "                                       ")
message(STATUS "  TMCxx Library - v${PROJECT_VERSION}")
message(STATUS "  Modern C++20 Driver for TMC5160      ")
message(STATUS "-------------------------------------------------------")

# --- Configuration Summary ---
message(STATUS "System Information:")
message(STATUS "  * System          : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} (${CMAKE_SYSTEM_PROCESSOR})")
message(STATUS "  * C++ Compiler    : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  * C++ Standard    : 20 (Enforced)")
message(STATUS "  * Install Prefix  : ${CMAKE_INSTALL_PREFIX}")

message(STATUS "")
message(STATUS "Components & Options:")
message(STATUS "  * Build Tests     : ${TMCXX_BUILD_TESTS}")
message(STATUS "  * Build Examples  : ${TMCXX_BUILD_EXAMPLES}")
message(STATUS "  * Static Analysis : ${ENABLE_CLANG_TIDY}")

if (CMAKE_BUILD_TYPE)
    message(STATUS "  * Build Type      : ${CMAKE_BUILD_TYPE}")
else ()
    message(STATUS "  * Build Type      : Default (None)")
endif ()

message(STATUS "-------------------------------------------------------")
message(STATUS "")