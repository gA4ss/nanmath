#include "nanmath.h"

namespace nanmath {
  
  int nanmath_int::add_d(nanmath_digit b) {
    int ix, oldused;
    nanmath_digit *tmp, mu;
    nanmath_word x;

    if (_alloc < _used + 1) {
      if (grow(_used + 1) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 如果当前是个负数并且|a| >= b,调用|a| - b */
    if (_sign == NM_NEG && (_used > 1 || _dp[0] >= b)) {
      /* |a| - b */
      _sign = NM_ZPOS;        /* 求a的绝对值 */
      sub_d(b);
      _sign = NM_NEG;
      clamp();
      return _lasterr;
    }
    
    oldused = _used;
    tmp = _dp;
    
    /* 如果是正数 */
    if (_sign == NM_ZPOS) {
      x = *tmp;
      x += b;
      *tmp = x & NM_MASK;
      mu = *tmp >> DIGIT_BIT;       /* 取出溢出的位 */
      /* 没溢出则不需要进入以下流程 */
      if (mu == 0) {
        goto _end;
      }
      
      /* 循环处理其他的位 */
      for (ix = 1; ix < _used; ix++) {
        *tmp += mu;                 /* 加进位 */
        mu = *tmp >> DIGIT_BIT;     /* 取进位 */
        *tmp++ &= NM_MASK;
        
        /* 没溢出则不需要进入以下流程 */
        if (mu == 0) {
          goto _end;
        }
      }
      
      /* 设置进位 */
      if (mu != 0) {
        *tmp++ = mu;
        _used = _used + 1;
      }
    } else {
      /* 是一个负数并且 |a| < b */
      if (_used == 1) {
        *tmp++ = b - _dp[0];
      } else {
        /* !!!这里不可能会发生 */
        *tmp++ = b;
      }
    }
    
  _end:
    clamp();
    return NM_OK;
  }

  int nanmath_int::add(nanmath_int &b) {
    int res;
    nanmath_int c;

    /* 获取标志 */
    int sa = _sign;
    int sb = b.get_sign();
    
    if (sa == sb) {   /* 符号相同 */
      res = s_add(*this, b, c);
    } else {
      if (cmp_mag(*this, b) == NM_LT) { /* a < b */
        _sign = sb;
        res = s_sub(b, *this, c);       /* b - a */
      } else {                          /* a >= b */
        res = s_sub(*this, b, c);       /* a - b */
      }
    }/* end if */

    /* 复制结果 */
    res = copy(c);

    return res;
  }
  
  int nanmath_int::add(nanmath_int &a, nanmath_int &b) {
    if (copy(a) != NM_OK)
      return _lasterr;
    return add(b);
  }
  
  int nanmath_int::s_add(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    nanmath_int &x = a;
    int min, max;
    
    /* 求出位数大的那一个，并且让x指向它 */
    if (a.get_used() > b.get_used()) {
      min = b.get_used();
      max = a.get_used();
      x = a;
    } else {
      min = a.get_used();
      max = b.get_used();
      x = b;
    }
    
    /* 初始化结果 */
    if (c.get_alloc() < max + 1) {
      if (c.grow(max + 1) != NM_OK) {
        return c.get_lasterr();
      }
    }
    c.zero();
    c.set_used(max + 1);
    
    nanmath_digit *tmpa = cast(nanmath_digit, a.get_digit());
    nanmath_digit *tmpb = cast(nanmath_digit, b.get_digit());
    nanmath_digit *tmpc = cast(nanmath_digit, c.get_digit());
    nanmath_digit *tmpx = cast(nanmath_digit, x.get_digit());
    
    int i;
    nanmath_digit u = 0;
    for (i = 0; i < min; i++) {
      *tmpc = *tmpa++ + *tmpb++ + u;      /* 运算 */
      u = *tmpc >> ((nanmath_digit)DIGIT_BIT); /* 取进位 */
      *tmpc++ &= NM_MASK;                 /* 清0进位 */
    }
    
    /* 两个数的位数不一样，则将位数多的高出的位加上进位 */
    if (min != max) {
      for (; i < max; i++) {
        *tmpc = tmpx[i] + u;
        u = *tmpc >> ((nanmath_digit)DIGIT_BIT);
        *tmpc++ &= NM_MASK;
      }
    }
    
    /* 加最终的进位 */
    *tmpc++ = u;
    
    c.clamp();
    return NM_OK;
  }
  
}





































