#include "nanmath.h"

namespace nanmath {
  int nm_int::cmp(nm_int &b) {
    if (_sign != b.get_sign()) {
      if (_sign == NM_NEG) {
        return NM_LT;
      } else {
        return NM_GT;
      }
    }
    
    if (_sign == NM_NEG) {
      return cmp_mag(b, *this);
    } else {
      return cmp_mag(*this, b);
    }
  }
  
  int nm_int::cmp_d(nm_digit b) {
    if (_sign == NM_NEG) {
      return NM_LT;
    }
    
    if (_used > 1) {
      return NM_GT;
    }
    
    if (_dp[0] > b) {
      return NM_GT;
    } else if (_dp[0] < b) {
      return NM_LT;
    } else {
      return NM_EQ;
    }
  }
  
  int nm_int::cmp_mag (nm_int &a, nm_int &b) {
    if (a.get_used() > b.get_used()) {
      return NM_GT;
    }
    
    if (a.get_used() < b.get_used()) {
      return NM_LT;
    }
    
    /* 相同的位数 */
    int used = a.get_used();
    nm_digit *tmpa = cast(nm_digit, a.get_digit()) + (used - 1);
    nm_digit *tmpb = cast(nm_digit, b.get_digit()) + (used - 1);
    
    for (int n = 0; n < used; ++n, --tmpa, --tmpb) {
      if (*tmpa > *tmpb) {
        return NM_GT;
      }
      
      if (*tmpa < *tmpb) {
        return NM_LT;
      }
    }
    return NM_EQ;
  }
  
}