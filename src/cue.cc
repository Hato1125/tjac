#include "cue.hh"

namespace tjac {
  note_kind note::kind_of(char ch) noexcept {
    switch (ch) {
      case '1': return note_kind::don;
      case '2': return note_kind::ka;
      case '3': return note_kind::don_big;
      case '4': return note_kind::ka_big;
    }
    return note_kind::don;
  }
}
