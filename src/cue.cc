#include "cue.hh"

namespace tjac {
  note_kind note::kind_of(char ch) noexcept {
    switch (ch) {
      case '1': return note_kind::don;
      case '2': return note_kind::ka;
      case '3': return note_kind::don_big;
      case '4': return note_kind::ka_big;
      case '5': return note_kind::roll;
      case '6': return note_kind::roll_big;
      case '7': return note_kind::ballon;
      case '8': return note_kind::roll_end;
      case '9': return note_kind::kusudama;
    }
    return note_kind::none;
  }
}
