#ifndef _TJAC_COURSE_HH
#define _TJAC_COURSE_HH

#include <expected>
#include <vector>
#include <span>
#include <cstdint>

#include "cue.hh"
#include "tjac.hh"

namespace tjac {
  class course {
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

    [[nodiscard]] std::expected<void, err> parse(std::span<const line> lines);

  // debug: private:
    kind _kind;
    std::uint8_t _level;
    std::uint16_t _balloon;

    std::vector<note> _notes;
    std::vector<event> _events;
    std::vector<bar> _bars;

    [[nodiscard]] std::expected<void, err> header_parse(std::span<const line> lines);
    [[nodiscard]] std::expected<void, err> body_parse(std::span<const line> lines);

    [[nodiscard]] static kind parse_kind(std::string_view str) noexcept;
    [[nodiscard]] static bool is_equal(
      std::string_view a,
      std::string_view b
    ) noexcept;
  };
}

#endif
