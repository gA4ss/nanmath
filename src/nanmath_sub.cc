#include "nanmath.h"

namespace nanmath {
  int nanmath_int::sub_d(nanmath_digit b) {
    nanmath_digit *tmpa, mu;
    int res, ix;
    
    if (_alloc < _used + 1) {
      if (grow(_used + 1) != NM_OK)
        return _lasterr;
    }
    
    /* 如果a是负的，减去b则使用加法，最后改变符号 */
    if (_sign == NM_NEG) {
      _sign = NM_ZPOS;
      res = add_d(b);
      _sign = NM_NEG;     /* 改变回来符号 */
      clamp();
      return res;
    }
    
    int oldused = _used;
    tmpa = _dp;
    
    /* 如果 a <= b 这个就比较简单了 */
    if ((_used == 1 && _dp[0] <= b) || _used == 0) {
      if (_used == 1) {
        *tmpa = b - *tmpa;
        tmpa++;
      } else {
        *tmpa++ = b;
      }
      ix = 1;
      
      _sign = NM_NEG;
      _used = 1;
    } else {     /* a > b */
      _sign = NM_ZPOS;
      
      /* subtract first digit */
      *tmpa -= b;
      mu = *tmpa >> (sizeof(nanmath_digit) * CHAR_BIT - 1);  /* 取借位 */
      *tmpa++ &= NM_MASK;
      
      for (ix = 1; ix < _used; ix++) {
        *tmpa -= mu;
        mu = *tmpa >> (sizeof(nanmath_digit) * CHAR_BIT - 1);
        *tmpa++ &= NM_MASK;
      }
    }
    
    /* 清除位 */
    while (ix++ < oldused) {
      *tmpa++ = 0;
    }
    
    clamp();
    return NM_OK;
  }
  
  int nanmath_int::sub(nanmath_int &b) {
    int res, sa = _sign, sb = b.get_sign(), sc = 0;
    nanmath_int c;
    
    if (sa != sb) {
      res = s_add(*this, b, c);
    } else {
      if (cmp_mag(*this, b) != NM_LT) {   /* a >= b */
        res = s_sub(*this, b, c);
      } else {        /* a < b */
        /* 设定结果符号 */
        sc = (sa == NM_ZPOS) ? NM_NEG : NM_ZPOS;
        c.set_sign(sc);
        
        res = s_sub(b, *this, c);
      }
    }
    
    res = copy(c);
    
    return res;
  }
  
  int nanmath_int::sub(nanmath_int &a, nanmath_int &b) {
    if (copy(a) != NM_OK)
      return _lasterr;
    return sub(b);
  }
  
  int nanmath_int::s_sub(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    int min = b.get_used();
    int max = a.get_used();
    
    if (c.get_alloc() < max) {
      if (c.grow(max) != NM_OK) {
        return c.get_lasterr();
      }
    }
    c.zero();
    c.set_used(max);
    
    nanmath_digit *tmpa = cast(nanmath_digit, a.get_digit());
    nanmath_digit *tmpb = cast(nanmath_digit, b.get_digit());
    nanmath_digit *tmpc = cast(nanmath_digit, c.get_digit());
    
    int i;
    nanmath_digit u = 0;     /* 借位 */
    for (i = 0; i < min; i++) {
      *tmpc = *tmpa++ - *tmpb++ - u;
      u = *tmpc >> ((nanmath_digit)(CHAR_BIT * sizeof(nanmath_digit) - 1)); /* u其实是最高位MSB */
      *tmpc++ &= NM_MASK;
    }
    
    /* 处理高位 */
    for (; i < max; i++) {
      *tmpc -= u;
      u = *tmpc >> ((nanmath_digit)(CHAR_BIT * sizeof(nanmath_digit) - 1));
      *tmpc++ &= NM_MASK;
    }
    
    c.clamp();
    return NM_OK;
  }
}