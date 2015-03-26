#include "nanmath.h"

namespace nanmath {

  static const struct {
    int code;
    const char *msg;
  } msgs[] = {
    { NM_OK, "Succesful" },
    { NM_MEM, "Memory failed" },
    { NM_VAL, "Value out of range" }
  };
  
  const char * const nm_int::error_to_string(int code) {
    int x;
    for (x = 0; x < (int)(sizeof(msgs) / sizeof(msgs[0])); x++) {
      if (msgs[x].code == code) {
        return msgs[x].msg;
      }
    }
    return "Invalid error code";
  }
}
