#include "course.hh"
#include "cue.hh"
#include "stmt.hh"
#include "utils.hh"

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
          TJAC_TRY_TO_NUM(level, std::uint8_t, value, "LEVEL", line.line);
        } else if (name == "BALLOON") {
          TJAC_TRY_TO_NUM(balloon, std::uint16_t, value, "BALLOON", line.line);
        } else if (name == "HPMAX") {
          TJAC_TRY_TO_NUM(hp_max, std::int32_t, value, "HPMAX", line.line);
        } else if (name == "HPCLEAR") {
          TJAC_TRY_TO_NUM(hp_clear, std::int32_t, value, "HPCLEAR", line.line);
        } else if (name == "HPGAINGOOD") {
          TJAC_TRY_TO_NUM(
            hp_gain_good,
            std::int32_t,
            value,
            "HPGAINGOOD",
            line.line
          );
        } else if (name == "HPGAINOK") {
          TJAC_TRY_TO_NUM(hp_gain_ok, std::int32_t, value, "HPGAINOK", line.line);
        } else if (name == "HPLOSSBAD") {
          TJAC_TRY_TO_NUM(hp_loss_bad, std::int32_t, value, "HPLOSSBAD", line.line);
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
            events.emplace_back(event_kind::gogo_begin, time);
          } else if (name == "GOGOEND") {
            events.emplace_back(event_kind::gogo_end, time);
          } else if (name == "BARLINEON") {
            events.emplace_back(event_kind::bar_on, time);
          } else if (name == "BARLINEOFF") {
            events.emplace_back(event_kind::bar_off, time);
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
          const auto apply_pendings_at = [&](std::size_t pos)
            -> std::expected<void, error> {
            for (auto& [line, pending_pos, cmd] : pendings) {
              if (pos != pending_pos) {
                continue;
              }

              const auto [name, value] = cmd;

              if (name == "MEASURE") {
                if (auto rs2 = measure::parse(value, line); !rs2) {
                  return std::unexpected(rs2.error());
                } else {
                  ms = *rs2;
                }
              } else if (name == "BPMCHANGE") {
                TJAC_TRY_TO_NUM(bpm, float, value, "#BPMCHANGE", line);
              } else if (name == "SCROLL") {
                TJAC_TRY_TO_NUM(scroll, float, value, "#SCROLL", line);
              } else if (name == "DELAY") {
                float delay = 0.0f;
                TJAC_TRY_TO_NUM(delay, float, value, "#DELAY", line);
                time += delay;
              }
            }

            return {};
          };

          // Apply commands pending at the start of the measure before
          // creating the bar line so that they affect it as well.
          if (auto rs = apply_pendings_at(0); !rs) {
            return std::unexpected(rs.error());
          }

          bars.emplace_back(time, bpm, scroll);

          if (buffer.empty()) {
            time += 240.0f / bpm * ms.rate();
          } else {
            auto n = buffer.size();
            for (auto i = 0uz; i < n; ++i) {
              if (i != 0) {
                if (auto rs = apply_pendings_at(i); !rs) {
                  return std::unexpected(rs.error());
                }
              }

              if (buffer[i] != '0') {
                notes.emplace_back(
                  note::kind_of(buffer[i]),
                  time,
                  bpm,
                  scroll
                );
              }
              time += 240.0f / bpm * ms.rate() / n;
            }

            if (auto rs = apply_pendings_at(n); !rs) {
              return std::unexpected(rs.error());
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

  course_kind course::kind_of(std::string_view str) noexcept {
    if (is_equal(str, "easy") || str == "1") { return course_kind::easy; }
    if (is_equal(str, "normal") || str == "2") { return course_kind::normal; }
    if (is_equal(str, "hard") || str == "3") { return course_kind::hard; }
    if (is_equal(str, "oni") || str == "4") { return course_kind::oni; }
    if (is_equal(str, "edit") || str == "5") { return course_kind::edit; }
    return course_kind::easy;
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
