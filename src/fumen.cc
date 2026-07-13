#include <vector>
#include <ranges>

#include "fumen.hh"
#include "course.hh"
#include "stmt.hh"

namespace {
  std::string_view trim(std::string_view sv) {
    constexpr std::string_view ws = " \t\r\n";
    const auto first = sv.find_first_not_of(ws);
    if (first == std::string_view::npos) {
      return {};
    }
    const auto last = sv.find_last_not_of(ws);
    return sv.substr(first, last - first + 1);
  }
}

namespace tjac {
  std::expected<fumen, error> fumen::parse(std::string src) {
    auto lines = src
      | std::views::split('\n')
      | std::views::transform([](auto&& r) {
          std::string_view str{r.begin(), r.end()};
          if (auto pos = str.find("//"); pos != std::string_view::npos) {
            str = std::string_view{str.substr(0, pos)};
          }
          return trim(str);
        })
      | std::views::enumerate
      | std::views::filter([](auto&& t) {
          return !std::get<1>(t).empty();
        })
      | std::views::transform([](auto&&t) {
          auto [i, sv] = t;
          return line{ sv, static_cast<std::uint32_t>(i + 1) };
        })
      | std::ranges::to<std::vector<line>>();

    localized_text title;
    localized_text subtitle;
    std::string wave;
    std::string genre;
    std::string maker;
    float bpm = 120.0f;
    float offset = 0.0f;
    float demostart = 0.0f;

    auto hpos = 0uz;
    for (; hpos < lines.size(); ++hpos) {
      if (auto rs = header::parse(lines[hpos]); !rs) {
        return std::unexpected(rs.error());
      } else {
        auto [name, value] = *rs;
        if (name == "TITLE") {
          title.ja = value;
        } else if (name == "TITLEEN") {
          title.en = value;
        } else if (name == "TITLEKR") {
          title.kr = value;
        } else if (name == "TITLETW") {
          title.tw = value;
        } else if (name == "SUBTITLE") {
          subtitle.ja = value;
        } else if (name == "SUBTITLEEN") {
          subtitle.en = value;
        } else if (name == "SUBTITLEKR") {
          subtitle.kr = value;
        } else if (name == "SUBTITLETW") {
          subtitle.tw = value;
        } else if (name == "WAVE") {
          wave = value;
        } else if (name == "GENRE") {
          genre = value;
        } else if (name == "MAKER") {
          maker = value;
        } else if (name == "BPM") {
          std::from_chars(value.begin(), value.end(), bpm);
        } else if (name == "OFFSET") {
          std::from_chars(value.begin(), value.end(), offset);
        } else if (name == "DEMOSTART") {
          std::from_chars(value.begin(), value.end(), demostart);
        } else if (name == "COURSE") {
          break;
        }
      }
    }

    std::vector<course> courses;

    if (hpos < lines.size()) {
      std::size_t begin = hpos;

      for (++hpos; hpos < lines.size(); ++hpos) {
        if (lines[hpos].str.starts_with("COURSE")) {
          course cs;
          auto rs = cs.parse({
            lines.begin() + begin,
            lines.begin() + hpos
          }, bpm);
          if (!rs) {
            return std::unexpected(rs.error());
          }
          courses.push_back(cs);

          begin = hpos;
        }
      }

      course cs;
      auto rs = cs.parse({
        lines.begin() + begin,
        lines.begin() + hpos
      }, bpm);
      if (!rs) {
        return std::unexpected(rs.error());
      }
      courses.push_back(cs);
    }

    return fumen{
      std::move(title),
      std::move(subtitle),
      wave,
      genre,
      maker,
      bpm,
      offset,
      demostart,
      std::move(courses)
    };
  }

  fumen::fumen(
    localized_text&& title,
    localized_text&& subtitle,
    std::string& wave,
    std::string& genre,
    std::string& maker,
    float bpm,
    float offset,
    float demostart,
    std::vector<course>&& courses
  ) noexcept : _title(std::move(title)),
    _subtitle(std::move(subtitle)),
    _wave(wave),
    _genre(genre),
    _maker(maker),
    _bpm(bpm),
    _offset(offset),
    _demostart(demostart),
    _courses(std::move(courses)) {}

  localized_text& fumen::title() noexcept { return _title; }
  localized_text& fumen::subtitle() noexcept { return _subtitle; }
  std::string& fumen::wave() noexcept { return _wave; }
  std::string& fumen::genre() noexcept { return _genre; }
  std::string& fumen::maker() noexcept { return _maker; }
  float fumen::bpm() const noexcept { return _bpm; }
  float fumen::offset() const noexcept { return _offset; }
  float fumen::demostart() const noexcept { return _demostart; }
  std::vector<course>& fumen::courses() noexcept { return _courses; }
}
