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
  class course {
    struct pending {
      std::uint32_t line;
      std::size_t pos;
      command cmd;
    };

  public:
    enum class kind : std::uint8_t {
      easy,
      normal,
      hard,
      oni,
      edit,
    };

    enum class err : std::uint8_t {
      missing_start,
      missing_end,
      header_parsing_failed,
      command_parsing_failed,
    };

    [[nodiscard]] std::expected<void, error> parse(
      std::span<const line> lines,
      float bpm = 120.0f
    );

    kind difficulty;
    std::uint8_t level;
    std::uint16_t balloon;

    std::vector<note> notes;
    std::vector<event> events;
    std::vector<bar> bars;

    [[nodiscard]] std::expected<void, error> header_parse(std::span<const line> lines);
    [[nodiscard]] std::expected<void, error> body_parse(
      std::span<const line> lines,
      float bpm
    );

    [[nodiscard]] static kind kind_of(std::string_view str) noexcept;
    [[nodiscard]] static bool is_equal(
      std::string_view a,
      std::string_view b
    ) noexcept;
  };
}

#endif
