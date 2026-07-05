#ifndef _TJAC_STMT_HH
#define _TJAC_STMT_HH

#include <expected>
#include <cstdint>
#include <string_view>

#include "tjac.hh"

namespace tjac {
  struct header {
    [[nodiscard]] static std::expected<header, error> parse(const line& line);

    std::string_view name;
    std::string_view value;
  };

  struct command {
    [[nodiscard]] static std::expected<command, error> parse(const line& line);

    std::string_view name;
    std::string_view value;
  };

  struct measure {
    [[nodiscard]] static std::expected<measure, error> parse(
      std::string_view str,
      std::uint32_t line
    );

    float part;
    float beat;

    [[nodiscard]] float rate() const noexcept;
  };
}

#endif
