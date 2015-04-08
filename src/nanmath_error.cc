#include "nanmath.h"
#include <string.h>

namespace nanmath {

  static const struct {
    int code;
    const char *msg;
  } msgs[] = {
    { NM_OK, "Succesful" },
    { NM_MEM, "Memory failed" },
    { NM_VAL, "Value out of range" }
  };
  
  const char * const nanmath_int::error_to_string(int code) {
    int x;
    for (x = 0; x < (int)(sizeof(msgs) / sizeof(msgs[0])); x++) {
      if (msgs[x].code == code) {
        return msgs[x].msg;
      }
    }
    return "Invalid error code";
  }
  
  int nanmath_int::get_lasterr() {
    return _lasterr;
  }
  
  char *nanmath_int::get_lasterr_func() {
    return _funcname;
  }
  
  void nanmath_int::rsle() {
    _lasterr = NM_OK;
    memset(_funcname, 0, MAX_BUFF_SIZE);
  }
  
  /* 设置最后一次错误 */
  int nanmath_int::set_lasterr(int err, char *fn) {
    _lasterr = err;
    strcpy(_funcname, fn);
    return err;
  }
}
