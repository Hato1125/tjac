#ifndef _TJAC_STMT_HH
#define _TJAC_STMT_HH

#include <expected>
#include <cstdint>

#include "tjac.hh"

namespace tjac {
  struct header {
    enum class err : std::uint8_t {
      missing_separator,
      empty_name,
    };

    [[nodiscard]] static std::expected<header, err> parse(const line& line);

    std::string_view name;
    std::string_view value;
  };

  struct command {
    enum class err : std::uint8_t {
      not_command,
      empty_name,
    };

    [[nodiscard]] static std::expected<command, err> parse(const line& line);

    std::string_view name;
    std::string_view value;
  };

  struct measure {
    enum class err : std::uint8_t {
      missing_separator,
      failed_convert_part_value,
      failed_convert_beat_value,
    };

    [[nodiscard]] static std::expected<measure, err> parse(std::string_view str);

    float part;
    float beat;

    [[nodiscard]] float rate() const noexcept;
  };
}

#endif
