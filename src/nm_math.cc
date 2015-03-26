#include "nanmath.h"

namespace nanmath {
  
  /* 清0 */
  void nm_int::zero() {
    _sign = NM_ZPOS;
    _used = 0;
    
    nm_digit *tmp = _dp;
    for (int n = 0; n < _alloc; n++) {
      *tmp++ = 0;
    }
  }
  
  int nm_int::iszero() {
    return ((_used == 0) ? NM_YES : NM_NO);
  }
  
  int nm_int::iseven() {
    return ((_used > 0 && ((_dp[0] & 1) == 0)) ? NM_YES : NM_NO);
  }
  
  int nm_int::sodd() {
    return ((_used > 0 && ((_dp[0] & 1) == 1)) ? NM_YES : NM_NO);
  }
 
  int nm_int::abs() {
    _sign = NM_ZPOS;
    return NM_OK;
  }
}
