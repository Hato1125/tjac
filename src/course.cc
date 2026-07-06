#include <charconv>

#include "course.hh"
#include "cue.hh"
#include "stmt.hh"

namespace tjac {
  std::expected<void, error> course::parse(
    std::span<const line> lines,
    float bpm
  ) {
    auto spos = 0uz;
    for (; spos < lines.size(); ++spos) {
      if (auto c = command::parse(lines[spos]); c && c->name == "START") {
        break;
      }
    }
    if (spos == lines.size()) {
      return std::unexpected(
        error{
          .code = error::code::missing_statement,
          .message = "course has no #START command",
          .line = lines.empty() ? 0 : lines.front().line,
          .column = 0,
        }
      );
    }

    auto epos = spos + 1;
    for (; epos < lines.size(); ++epos) {
      if (auto c = command::parse(lines[epos]); c && c->name == "END") {
        break;
      }
    }

    if (epos == lines.size()) {
      return std::unexpected(
        error{
          .code = error::code::missing_statement,
          .message = "#START has no matching #END",
          .line = lines[spos].line,
          .column = 0,
        }
      );
    }

    if (auto rs = header_parse(lines.first(spos)); !rs) {
      return std::unexpected(rs.error());
    }

    if (auto rs = body_parse(lines.subspan(spos + 1, epos - spos - 1), bpm); !rs) {
      return std::unexpected(rs.error());
    }

    return {};
  }

  std::expected<void, error> course::header_parse(std::span<const line> lines) {
    for (const auto& line : lines) {
      if (line.str.empty()) {
        continue;
      }

      if (auto rs = header::parse(line); !rs) {
        return std::unexpected(rs.error());
      } else {
        const auto [name, value] = *rs;
        if (name == "COURSE") {
          difficulty = kind_of(value);
        } else if (name == "LEVEL") {
          std::from_chars(value.begin(), value.end(), level);
        } else if (rs->name == "BALLOON") {
          std::from_chars(value.begin(), value.end(), balloon);
        }
      }
    }

    return {};
  }

  std::expected<void, error> course::body_parse(
    std::span<const line> lines,
    float bpm
  ) {
    float time = 0.0f;
    float scroll = 1.0f;
    measure ms{ 4.0f, 4.0f };

    std::vector<char> buffer;
    std::vector<pending> pendings;

    for (const auto& line : lines) {
      if (line.str.empty()) {
        continue;
      }

      if (line.str[0] == '#') {
        if (auto rs = command::parse(line); !rs) {
          return std::unexpected(rs.error());
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
            pendings.emplace_back(
              line.line,
              buffer.size(),
              std::move(*rs)
            );
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
              for (auto& [line, pos, cmd] : pendings) {
                if (i == pos) {
                  const auto [name, value] = cmd;
                  if (name == "MEASURE") {
                    if (auto rs2 = measure::parse(value, line); !rs2) {
                      return std::unexpected(rs2.error());
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
              time += 240.0f / bpm * ms.rate() / static_cast<float>(n);
            }
          }

          buffer.clear();
          pendings.clear();

          continue;
        }

        buffer.push_back(ch);
      }
    }

    return {};
  }

  course::kind course::kind_of(std::string_view str) noexcept {
    if (is_equal(str, "easy") || str == "1") { return kind::easy; }
    if (is_equal(str, "normal") || str == "2") { return kind::normal; }
    if (is_equal(str, "hard") || str == "3") { return kind::hard; }
    if (is_equal(str, "oni") || str == "4") { return kind::oni; }
    if (is_equal(str, "edit") || str == "5") { return kind::edit; }
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
