#include "nanmath.h"

namespace nanmath {
  
  /* 清0 */
  void nanmath_int::zero(int s) {
    if (s) _sign = NANMATH_ZPOS;
    
    _used = 1;
    
    nanmath_digit *tmp = _dp;
    for (int n = 0; n < _alloc; n++) {
      *tmp++ = 0;
    }
  }
  
  int nanmath_int::iszero() {
    if ((_used == 0) ||
      ((_used == 1) && (_dp[0] == 0))) {
      return NANMATH_YES;
    }
    
    return NANMATH_NO;
  }
  
  int nanmath_int::iseven() {
    return ((_used > 0 && ((_dp[0] & 1) == 0)) ? NANMATH_YES : NANMATH_NO);
  }
  
  int nanmath_int::isodd() {
    return ((_used > 0 && ((_dp[0] & 1) == 1)) ? NANMATH_YES : NANMATH_NO);
  }
 
  int nanmath_int::abs() {
    _sign = NANMATH_ZPOS;
    return NANMATH_OK;
  }
  
  int nanmath_int::neg(nanmath_int &b) {
    if (cmp(b) == NANMATH_EQ) {
      return NANMATH_OK;
    }
    
    b.copy(*this);
    if (b.iszero() != NANMATH_OK) {
      b.set_sign((_sign == NANMATH_ZPOS) ? NANMATH_NEG : NANMATH_ZPOS);
    } else {
      b.set_sign(NANMATH_ZPOS);
    }
    
    return NANMATH_OK;
  }
}
