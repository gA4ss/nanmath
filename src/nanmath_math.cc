#include "nanmath.h"

namespace nanmath {
  
  /* 清0 */
  void nanmath_int::zero() {
    _sign = NM_ZPOS;
    _used = 1;
    
    nanmath_digit *tmp = _dp;
    for (int n = 0; n < _alloc; n++) {
      *tmp++ = 0;
    }
  }
  
  int nanmath_int::iszero() {
    if ((_used == 0) ||
      ((_used == 1) && (_dp[0] == 0))) {
      return NM_YES;
    }
    
    return NM_NO;
  }
  
  int nanmath_int::iseven() {
    return ((_used > 0 && ((_dp[0] & 1) == 0)) ? NM_YES : NM_NO);
  }
  
  int nanmath_int::sodd() {
    return ((_used > 0 && ((_dp[0] & 1) == 1)) ? NM_YES : NM_NO);
  }
 
  int nanmath_int::abs() {
    _sign = NM_ZPOS;
    return NM_OK;
  }
}
