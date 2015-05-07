#include "nanmath.h"

namespace nanmath {
  
  /* 清0 */
  void nanmath_int::zero() {
    _sign = NANMATH_ZPOS;
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
  
  int nanmath_int::sodd() {
    return ((_used > 0 && ((_dp[0] & 1) == 1)) ? NANMATH_YES : NANMATH_NO);
  }
 
  int nanmath_int::abs() {
    _sign = NANMATH_ZPOS;
    return NANMATH_OK;
  }
}
