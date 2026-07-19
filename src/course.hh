#ifndef _TJAC_COURSE_HH
#define _TJAC_COURSE_HH

#include <expected>
#include <vector>
#include <span>
#include <cstdint>

#include "cue.hh"
#include "stmt.hh"
#include "tjac.hh"

namespace tjac {
  enum class course_kind : std::uint8_t {
    easy,
    normal,
    hard,
    oni,
    edit,
  };

  class course {
    struct pending {
      std::uint32_t line;
      std::size_t pos;
      command cmd;
    };

  public:
    [[nodiscard]] std::expected<void, error> parse(
      std::span<const line> lines,
      float bpm = 120.0f
    );

    course_kind difficulty;
    std::uint8_t level = 0;
    std::uint16_t balloon = 0;

    std::int32_t hp_max = 0;
    std::int32_t hp_clear = 0;
    std::int32_t hp_gain_good = 0;
    std::int32_t hp_gain_ok = 0;
    std::int32_t hp_loss_bad = 0;

    std::vector<note> notes;
    std::vector<event> events;
    std::vector<bar> bars;

    [[nodiscard]] std::expected<void, error> header_parse(std::span<const line> lines);
    [[nodiscard]] std::expected<void, error> body_parse(
      std::span<const line> lines,
      float bpm
    );

    [[nodiscard]] static course_kind kind_of(std::string_view str) noexcept;
    [[nodiscard]] static bool is_equal(
      std::string_view a,
      std::string_view b
    ) noexcept;
  };
}

#endif
