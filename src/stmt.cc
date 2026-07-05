#include <charconv>
#include <format>

#include "stmt.hh"

namespace tjac {
  std::expected<header, error> header::parse(const line& line) {
    auto spos = line.str.find(':');
    if (spos == std::string_view::npos) {
      return std::unexpected(
        error{
          .code = error::code::missing_separator,
          .message = "header line has no ':' separator",
          .line = line.line,
          .column = 0,
        }
      );
    }
    if (spos == 0) {
      return std::unexpected(
        error{
          .code = error::code::empty_name,
          .message = "header name before ':' is empty",
          .line = line.line,
          .column = 0,
        }
      );
    }

    std::string_view name = line.str.substr(0, spos);

    auto i = spos + 1;
    while (i < line.str.size() && line.str[i] == ' ') {
      ++i;
    }

    return header{ name, line.str.substr(i) };
  }

  std::expected<command, error> command::parse(const line& line) {
    if (!line.str.starts_with('#')) {
      return std::unexpected(
        error{
          .code = error::code::missing_format,
          .message = "command line must start with '#'",
          .line = line.line,
          .column = 0,
        }
      );
    }

    auto spos = line.str.find(' ');
    std::string_view name = line.str.substr(1, spos - 1);
    if (name.empty()) {
      return std::unexpected(
        error{
          .code = error::code::empty_name,
          .message = "command name after '#' is empty",
          .line = line.line,
          .column = 1,
        }
      );
    }
    if (spos == std::string_view::npos) {
      return command{ name, {} };
    }

    auto i = spos + 1;
    while (i < line.str.size() && line.str[i] == ' ') {
      ++i;
    }

    return command{ name, line.str.substr(i) };
  }

  std::expected<measure, error> measure::parse(
    std::string_view str,
    std::uint32_t line
  ) {
    auto spos = str.find('/');
    if (spos == std::string_view::npos) {
      return std::unexpected(
        error{
          .code = error::code::missing_separator,
          .message = std::format("expected '/' in measure value '{}'", str),
          .line = line,
          .column = 0,
        }
      );
    }

    auto pstr = str.substr(0, spos);
    float pv = 0.0f;
    auto [pptr, pec] = std::from_chars(
      pstr.begin(),
      pstr.end(),
      pv
    );
    if (pec != std::errc{} || pptr != pstr.end()) {
      return std::unexpected(
        error{
          .code = error::code::failed_convert_value,
          .message = std::format("invalid number '{}' before '/'", pstr),
          .line = line,
          .column = static_cast<std::uint32_t>(pptr - str.data()),
        }
      );
    }

    auto bstr = str.substr(spos + 1);
    float bv = 0.0f;
    auto [bptr, bec] = std::from_chars(
      bstr.begin(),
      bstr.end(),
      bv
    );
    if (bec != std::errc{} || bptr != bstr.end()) {
      return std::unexpected(
        error{
          .code = error::code::failed_convert_value,
          .message = std::format("invalid number '{}' after '/'", bstr),
          .line = line,
          .column = static_cast<std::uint32_t>(bptr - str.data()),
        }
      );
    }

    return measure{ pv, bv };
  }

  float measure::rate() const noexcept {
    return part / beat;
  }
}
