#ifndef _TJAC_UTILS_HH
#define _TJAC_UTILS_HH

#include <charconv>
#include <expected>
#include <format>
#include <cstdint>
#include <string_view>

#include "tjac.hh"

namespace tjac {
  template <typename T>
  std::expected<T, error> to_num(
    std::string_view value,
    std::string_view what,
    std::uint32_t line
  ) {
    T num{};

    const auto [ptr, ec] = std::from_chars(
      value.begin(),
      value.end(),
      num
    );

    if (ec != std::errc{} || ptr != value.end()) {
      return std::unexpected(
        error{
          .code = error::code::failed_convert_value,
          .message = std::format("invalid number '{}' for {}", value, what),
          .line = line,
          .column = static_cast<std::uint32_t>(ptr - value.data()),
        }
      );
    }

    return num;
  }
}

#define TJAC_TRY_TO_NUM(out, T, value, what, line)          \
  do {                                                      \
    auto ttn_rs = tjac::to_num<T>((value), (what), (line)); \
    if (!ttn_rs) {                                          \
      return std::unexpected(ttn_rs.error());               \
    }                                                       \
    (out) = *ttn_rs;                                        \
  } while (0)

#endif
