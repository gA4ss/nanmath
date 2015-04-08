#include "nanmath.h"

namespace nanmath {
  int nanmath_int::cmp(nanmath_int &b) {
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
  
  int nanmath_int::cmp_d(nanmath_digit b) {
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
  
  int nanmath_int::cmp_mag (nanmath_int &a, nanmath_int &b) {
    if (a.get_used() > b.get_used()) {
      return NM_GT;
    }
    
    if (a.get_used() < b.get_used()) {
      return NM_LT;
    }
    
    /* 相同的位数 */
    int used = a.get_used();
    nanmath_digit *tmpa = cast(nanmath_digit, a.get_digit()) + (used - 1);
    nanmath_digit *tmpb = cast(nanmath_digit, b.get_digit()) + (used - 1);
    
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