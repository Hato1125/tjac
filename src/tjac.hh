#ifndef _TJAC_TJAC_HH
#define _TJAC_TJAC_HH

#include <string_view>
#include <cstdint>

namespace tjac {
  struct line {
    std::string_view str;
    std::uint32_t line;
    std::uint32_t column;
  };
}

#endif
