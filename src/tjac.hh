#ifndef _TJAC_TJAC_HH
#define _TJAC_TJAC_HH

#include <string>
#include <cstdint>

namespace tjac {
  struct line {
    std::string_view str;
    std::uint32_t line;
  };

  struct error {
    enum class code : std::uint8_t {
      missing_format,
      missing_separator,
      empty_name,
      failed_convert_value,
      missing_statement,
      parsing_failed,
    };

    code code;

    std::string message;
    std::uint32_t line;
    std::uint32_t column;
  };
}

#endif
