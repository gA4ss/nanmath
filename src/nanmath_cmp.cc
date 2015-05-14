#include "nanmath.h"

namespace nanmath {
  int nanmath_int::cmp(nanmath_int &b) {
    if (_sign != b.get_sign()) {
      if (_sign == NANMATH_NEG) {
        return NANMATH_LT;
      } else {
        return NANMATH_GT;
      }
    }
    
    if (_sign == NANMATH_NEG) {
      return s_cmp_mag(b, *this);
    } else {
      return s_cmp_mag(*this, b);
    }
  }
  
  int nanmath_int::cmp_d(nanmath_digit b) {
    if (_sign == NANMATH_NEG) {
      return NANMATH_LT;
    }
    
    if (_used > 1) {
      return NANMATH_GT;
    }
    
    if (_dp[0] > b) {
      return NANMATH_GT;
    } else if (_dp[0] < b) {
      return NANMATH_LT;
    } else {
      return NANMATH_EQ;
    }
  }
  
  int nanmath_int::s_cmp_mag(nanmath_int &a, nanmath_int &b) {
    if (a.get_used() > b.get_used()) {
      return NANMATH_GT;
    }
    
    if (a.get_used() < b.get_used()) {
      return NANMATH_LT;
    }
    
    /* 相同的位数 */
    int used = a.get_used();
    nanmath_digit *tmpa = cast(nanmath_digit, a.get_digit()) + (used - 1);
    nanmath_digit *tmpb = cast(nanmath_digit, b.get_digit()) + (used - 1);
    
    for (int n = 0; n < used; ++n, --tmpa, --tmpb) {
      if (*tmpa > *tmpb) {
        return NANMATH_GT;
      }
      
      if (*tmpa < *tmpb) {
        return NANMATH_LT;
      }
    }
    return NANMATH_EQ;
  }
  
}