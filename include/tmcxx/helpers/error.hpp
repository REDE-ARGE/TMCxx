/************************************************************
 *  Project : TMCxx
 *  File    : error
 *  Author  : Mustafa Berk YILMAZ (mustafa.yilmaz@redearge.com)
 *  Created : 12.12.2025
 *
 *  Note: This file used "tl::expected" cause of used C++20, when version update to C++23 "tl::expected" can changeable
 * with "std::expected"
 ************************************************************/

#ifndef TMCXX_ERROR_HPP
#define TMCXX_ERROR_HPP

#include "tmcxx/vendor/expected.hpp"

#include <cstdint>

namespace tmcxx::helpers {

/**
 * @brief Common error codes for TMCxx library operations.
 */
enum class ErrorCode : uint8_t {
    SUCCESS = 0U,
    SPI_TRANSFER_FAILED,
    REGISTER_ACCESS_FAILED,
    INVALID_PARAMETER,
    TIMEOUT,
    CHIP_BUSY,
    NOT_IMPLEMENTED,
    UNKNOWN_ERROR
};

/**
 * @brief Result type alias using tl::expected.
 *
 * Usage:
 *   result_t<void> apply_settings();
 *   result_t<int32_t> get_position();
 */
template<typename T>
using result_t = tl::expected<T, ErrorCode>;

} // namespace tmcxx::helpers

#endif // TMCXX_ERROR_HPP
