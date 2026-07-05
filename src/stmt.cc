#include <charconv>

#include "stmt.hh"

namespace tjac {
  std::expected<header, header::err> header::parse(const line& line) {
    auto spos = line.str.find(':');
    if (spos == std::string_view::npos) {
      return std::unexpected{ err::missing_separator };
    }
    if (spos == 0) {
      return std::unexpected{ err::empty_name };
    }

    std::string_view name = line.str.substr(0, spos);

    auto i = spos + 1;
    while (i < line.str.size() && line.str[i] == ' ') {
      ++i;
    }

    return header{ name, line.str.substr(i) };
  }

  std::expected<command, command::err> command::parse(const line& line) {
    if (!line.str.starts_with('#')) {
      return std::unexpected{ err::not_command };
    }

    auto spos = line.str.find(' ');
    std::string_view name = line.str.substr(1, spos - 1);
    if (name.empty()) {
      return std::unexpected{ err::empty_name };
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

  std::expected<measure, measure::err> measure::parse(std::string_view str) {
    auto spos = str.find('/');
    if (spos == std::string_view::npos) {
      return std::unexpected{ err::missing_separator };
    }

    auto pstr = str.substr(0, spos);
    float pv = 0.0f;
    auto [pptr, pec] = std::from_chars(
      pstr.begin(),
      pstr.end(),
      pv
    );
    if (pec != std::errc{} || pptr != pstr.end()) {
      return std::unexpected{ err::failed_convert_part_value };
    }

    auto bstr = str.substr(spos + 1);
    float bv = 0.0f;
    auto [bptr, bec] = std::from_chars(
      bstr.begin(),
      bstr.end(),
      bv
    );
    if (bec != std::errc{} || bptr != bstr.end()) {
      return std::unexpected{ err::failed_convert_beat_value };
    }

    return measure{ pv, bv };
  }

  float measure::rate() const noexcept {
    return part / beat;
  }
}
