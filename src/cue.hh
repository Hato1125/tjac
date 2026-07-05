#ifndef _TJAC_CUE_HH
#define _TJAC_CUE_HH

#include <cstdint>

namespace tjac {
  struct note {
    enum class kind : std::uint8_t {
      don,
      ka,
      don_big,
      ka_big,
    };

    kind kind;
    float time;
    float bpm;
    float scroll;
    bool hitted;
    bool visible;

    [[nodiscard]] static decltype(note::kind) kind_of(char ch) noexcept;
  };

  struct event {
    enum class kind : std::uint8_t {
      gogo_begin,
      gogo_end,
      bar_on,
      bar_off,
    };

    kind kind;
    float time;
  };

  struct bar {
    float time;
    float bpm;
    float scroll;
  };
}

#endif
