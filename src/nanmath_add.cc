#include "nanmath.h"

namespace nanmath {
  
  int nanmath_int::add_d(nm_digit b) {
    int ix, oldused;
    nm_digit *tmp, mu;
    nm_word x;

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

    /* 获取标志 */
    int sa = _sign;
    int sb = b.get_sign();
    
    if (sa == sb) {   /* 符号相同 */
      res = s_add(b);
    } else {
      if (cmp_mag(*this, b) == NM_LT) { /* a < b */
        _sign = sb;
        res = s_sub(b, *this);    /* b - a */
      } else {                    /* a >= b */
        res = s_sub(*this, b);    /* a - b */
      }
    }/* end if */

    return res;
  }
  
  int nanmath_int::add(nanmath_int &a, nanmath_int &b) {
    if (copy(a) != NM_OK)
      return _lasterr;
    return add(b);
  }
  
  /* 辅助算法, 依赖 HAC pp.594, Algorithm 14.7 */
  int nanmath_int::s_add(nanmath_int &b) {
    nanmath_int *x;
    int olduse, min, max;
    
    /* 求出位数大的那一个，并且让x指向它 */
    if (_used > b.get_used()) {
      min = b.get_used();
      max = _used;
      x = this;
    } else {
      min = _used;
      max = b.get_used();
      x = &b;
    }
    
    /* 初始化结果 */
    if (_alloc < max + 1) {
      if (grow(max + 1) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 保存旧的位数，设置新的 */
    olduse = _used;
    _used = max + 1;
    
    nm_digit *tmpa = _dp;
    nm_digit *tmpb = cast(nm_digit, b.get_digit());
    nm_digit *tmpx = cast(nm_digit, x->get_digit());
    
    int i;
    nm_digit u = 0;
    for (i = 0; i < min; i++) {
      *tmpa += (*tmpb++ + u);             /* 运算 */
      u = *tmpa >> ((nm_digit)DIGIT_BIT); /* 取进位 */
      *tmpa++ &= NM_MASK;                 /* 清0进位 */
    }
      
    /* 两个数的位数不一样，则将位数多的高出的位加上进位 */
    if (min != max) {
      for (; i < max; i++) {
        *tmpa = tmpx[i] + u;
        u = *tmpa >> ((nm_digit)DIGIT_BIT);
        *tmpa++ &= NM_MASK;
      }
    }
      
    /* 加最终的进位 */
    *tmpa++ = u;
    
    clamp();
    return NM_OK;
  }
  
  int nanmath_int::s_add(nanmath_int &a, nanmath_int &b) {
    if (copy(a) != NM_OK)
      return _lasterr;
    return s_add(b);
  }
 
  int nanmath_int::s_add(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    nanmath_int *x;
    int olduse, min, max;
    
    /* 求出位数大的那一个，并且让x指向它 */
    if (a.get_used() > b.get_used()) {
      min = b.get_used();
      max = a.get_used();
      x = this;
    } else {
      min = a.get_used();
      max = b.get_used();
      x = &b;
    }
    
    /* 初始化结果 */
    if (c.get_alloc() < max + 1) {
      if (c.grow(max + 1) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 保存旧的位数，设置新的 */
    olduse = _used;
    _used = max + 1;
    
    nm_digit *tmpa = cast(nm_digit, a.get_digit());
    nm_digit *tmpb = cast(nm_digit, b.get_digit());
    nm_digit *tmpc = cast(nm_digit, c.get_digit());
    nm_digit *tmpx = cast(nm_digit, x->get_digit());
    
    int i;
    nm_digit u = 0;
    for (i = 0; i < min; i++) {
      *tmpc = *tmpa++ + *tmpb++ + u;      /* 运算 */
      u = *tmpc >> ((nm_digit)DIGIT_BIT); /* 取进位 */
      *tmpc++ &= NM_MASK;                 /* 清0进位 */
    }
    
    /* 两个数的位数不一样，则将位数多的高出的位加上进位 */
    if (min != max) {
      for (; i < max; i++) {
        *tmpc = tmpx[i] + u;
        u = *tmpc >> ((nm_digit)DIGIT_BIT);
        *tmpc++ &= NM_MASK;
      }
    }
    
    /* 加最终的进位 */
    *tmpc++ = u;
    
    c.clamp();
    return NM_OK;
  }
  
}





































