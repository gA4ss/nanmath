#include "nanmath.h"

namespace nanmath {
  int nm_int::div_2() {
    return NM_OK;
  }
  
  int nm_int::div_d(nm_digit v) {
    return NM_OK;
  }
  
  int nm_int::div(nm_int &v) {
    return NM_OK;
  }
  
  int nm_int::div(nm_int &a, nm_int &b) {
    if (copy(a) != NM_OK)
      return _lasterr;
    return div(b);
  }
}