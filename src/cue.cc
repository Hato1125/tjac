#include "cue.hh"

namespace tjac {
  decltype(note::kind) note::kind_of(char ch) noexcept {
    switch (ch) {
      case '1': return kind::don;
      case '2': return kind::ka;
      case '3': return kind::don_big;
      case '4': return kind::ka_big;
    }
    return kind::don;
  }
}
