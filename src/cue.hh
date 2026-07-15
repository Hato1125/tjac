#ifndef _TJAC_CUE_HH
#define _TJAC_CUE_HH

#include <cstdint>

namespace tjac {
  enum class note_kind : std::uint8_t {
    none,
    don,
    ka,
    don_big,
    ka_big,
    roll,
    roll_big,
    ballon,
    roll_end,
    kusudama,
  };

  struct note {
    note_kind kind;
    float time;
    float bpm;
    float scroll;
    bool hitted;
    bool visible;

    [[nodiscard]] static note_kind kind_of(char ch) noexcept;
  };

  enum class event_kind : std::uint8_t {
    gogo_begin,
    gogo_end,
    bar_on,
    bar_off,
  };

  struct event {
    event_kind kind;
    float time;
  };

  struct bar {
    float time;
    float bpm;
    float scroll;
  };
}

#endif
