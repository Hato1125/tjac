#ifndef _TJAC_FUMEN_HH
#define _TJAC_FUMEN_HH

#include <vector>
#include <string>
#include <expected>

#include "tjac.hh"
#include "course.hh"

namespace tjac {
  struct localized_text {
    std::string ja;
    std::string en;
    std::string kr;
    std::string tw;
  };

  class fumen {
  public:
    [[nodiscard]] static std::expected<fumen, error> parse(std::string src);

    fumen(
      localized_text&& title,
      localized_text&& subtitle,
      std::string& wave,
      std::string& genre,
      std::string& maker,
      float bpm,
      float offset,
      float demostart,
      std::vector<course>&& courses
    ) noexcept;

    fumen(const fumen&) = delete;
    fumen& operator=(const fumen&) = delete;

    fumen(fumen&&) = default;
    fumen& operator=(fumen&&) = default;

    [[nodiscard]] localized_text& title() noexcept;
    [[nodiscard]] localized_text& subtitle() noexcept;
    [[nodiscard]] std::string& wave() noexcept;
    [[nodiscard]] std::string& genre() noexcept;
    [[nodiscard]] std::string& maker() noexcept;
    [[nodiscard]] float bpm() const noexcept;
    [[nodiscard]] float offset() const noexcept;
    [[nodiscard]] float demostart() const noexcept;
    [[nodiscard]] std::vector<course>& courses() noexcept;

  private:
    localized_text _title;
    localized_text _subtitle;
    std::string _wave;
    std::string _genre;
    std::string _maker;
    float _bpm;
    float _offset;
    float _demostart;
    std::vector<course> _courses;
  };
}

#endif
