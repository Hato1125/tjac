#include <charconv>

#include "course.hh"
#include "cue.hh"
#include "stmt.hh"

namespace tjac {
  std::expected<void, course::err> course::parse(std::span<const line> lines) {
    auto spos = 0uz;
    for (; spos < lines.size(); ++spos) {
      if (auto c = command::parse(lines[spos]); c && c->name == "START") {
        break;
      }
    }
    if (spos == lines.size()) {
      return std::unexpected{ err::missing_start };
    }

    auto epos = spos + 1;
    for (; epos < lines.size(); ++epos) {
      if (auto c = command::parse(lines[epos]); c && c->name == "END") {
        break;
      }
    }

    if (epos == lines.size()) {
      return std::unexpected{ err::missing_end };
    }

    if (auto rs = header_parse(lines.first(spos)); !rs) {
      return std::unexpected{ rs.error() };
    }

    if (auto rs = body_parse(lines.subspan(spos + 1, epos - spos - 1)); !rs) {
      return std::unexpected{ rs.error() };
    }

    return {};
  }

  std::expected<void, course::err> course::header_parse(std::span<const line> lines) {
    for (const auto& line : lines) {
      if (line.str.empty()) {
        continue;
      }

      if (auto rs = header::parse(line); !rs) {
        return std::unexpected{ err::header_parsing_failed };
      } else {
        const auto [name, value] = *rs;
        if (name == "COURSE") {
          _kind = parse_kind(value);
        } else if (name == "LEVEL") {
          std::from_chars(value.begin(), value.end(), _level);
        } else if (rs->name == "BALLOON") {
          std::from_chars(value.begin(), value.end(), _balloon);
        }
      }
    }

    return {};
  }

  std::expected<void, course::err> course::body_parse(std::span<const line> lines) {
    float time = 0.0f;
    float bpm = 120.0f;
    float scroll = 1.0f;
    measure ms{ 4.0f, 4.0f };

    std::vector<char> buffer;
    std::vector<std::pair<decltype(buffer)::size_type, command>> pending;

    for (const auto& line : lines) {
      if (line.str.empty()) {
        continue;
      }

      if (line.str[0] == '#') {
        if (auto rs = command::parse(line); !rs) {
          return std::unexpected{ err::command_parsing_failed };
        } else {
          const auto [name, value] = *rs;
          if (name == "GOGOSTART") {
            _events.emplace_back(event::kind::gogo_begin, time);
          } else if (name == "GOGOEND") {
            _events.emplace_back(event::kind::gogo_end, time);
          } else if (name == "BARLINEON") {
            _events.emplace_back(event::kind::bar_on, time);
          } else if (name == "BARLINEOFF") {
            _events.emplace_back(event::kind::bar_off, time);
          } else {
            pending.emplace_back(buffer.size(), std::move(*rs));
          }
        }
        continue;
      }

      for (char ch : line.str) {
        // ',' closes a measure. note chars just pile up in `buffer`
        // (possibly across several lines) until then, because the
        // measure is divided evenly by its char count and that count
        // is only known here. on close: emit a bar line at the head,
        // then walk the chars, giving each 1/n of the measure
        // (240/bpm * a/b seconds in total).
        //
        //   input:  "12"  #bpmchange 240  "12,"
        //
        //   buffer:  1    2    1    2       (n = 4)
        //   pending:         ^ { pos = 2, bpmchange 240 }
        //   time:    0.0  0.5  1.0  1.25
        //            [ bpm 120 ][ bpm 240 ]
        //
        // mid-measure commands are applied when the walk reaches the
        // buffer position they appeared at (pos), so only the chars
        // after them see the new bpm/scroll. an empty buffer means a
        // ","-only measure: no notes, time still advances by one full
        // measure.
        if (ch == ',') {
          _bars.emplace_back(time, bpm, scroll);

          if (buffer.empty()) {
            time += 240.0f / bpm * ms.rate();
          } else {
            auto n = buffer.size();
            for (auto i = 0uz; i < n; ++i) {
              for (auto& [pos, cmd] : pending) {
                if (i == pos) {
                  const auto [name, value] = cmd;
                  if (name == "MEASURE") {
                    if (auto rs2 = measure::parse(value); !rs2) {
                      return std::unexpected{ err::command_parsing_failed };
                    } else {
                      ms = *rs2;
                    }
                  } else if (name == "BPMCHANGE") {
                    std::from_chars(value.begin(), value.end(), bpm);
                  } else if (name == "SCROLL") {
                    std::from_chars(value.begin(), value.end(), scroll);
                  }
                }
              }

              if (buffer[i] != '0') {
                _notes.emplace_back(
                  note::kind_of(buffer[i]),
                  time,
                  bpm,
                  scroll
                );
              }
              time += 240.0f / bpm * ms.rate() / n;
            }
          }

          buffer.clear();
          pending.clear();

          continue;
        }

        buffer.push_back(ch);
      }
    }

    return {};
  }

  course::kind course::parse_kind(std::string_view str) noexcept {
    if (is_equal(str, "easy")
      || is_equal(str, "1")
      || str == "かんたん"
      || str == "簡単"
    ) {
      return kind::easy;
    }

    if (is_equal(str, "normal")
      || is_equal(str, "2")
      || str == "ふつう"
      || str == "普通"
    ) {
      return kind::normal;
    }

    if (is_equal(str, "hard")
      || is_equal(str, "3")
      || str == "むずかしい"
      || str == "難しい"
    ) {
      return kind::hard;
    }

    if (is_equal(str, "oni")
      || is_equal(str, "4")
      || str == "おに"
      || str == "鬼"
    ) {
      return kind::oni;
    }

    if (is_equal(str, "edit")
      || is_equal(str, "5")
      || str == "うらおに"
      || str == "裏鬼"
    ) {
      return kind::edit;
    }

    return kind::easy;
  }

  bool course::is_equal(
    std::string_view a,
    std::string_view b
  ) noexcept {
    if (a.size() != b.size()) {
      return false;
    }
    for (auto i = 0uz; i < a.size(); ++i) {
      auto la = (a[i] >= 'A' && a[i] <= 'Z') ? a[i] + ('a' - 'A') : a[i];
      auto lb = (b[i] >= 'A' && b[i] <= 'Z') ? b[i] + ('a' - 'A') : b[i];
      if (la != lb) {
        return false;
      }
    }
    return true;
  }
}
