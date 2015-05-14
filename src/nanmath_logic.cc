#include "nanmath.h"

namespace nanmath {
  
  /* 向左移动2进制b位 */
  int nanmath_int::lsh(nanmath_size b) {
    int res;
    if (b <= 0) {
      return NANMATH_OK;
    }
    
    nanmath_size d = b / DIGIT_BIT;/* 有多少个位精度 */
    nanmath_size g = (nanmath_size)(_used + d + 1);
    if (_alloc < g) {
      if ((res = grow(g)) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 首先移动整数倍 */
    if (d > 0) {
      if ((res = lsh_d(d)) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 移动剩下的余数 */
    d = b % DIGIT_BIT;    /* 保证d < DIGIT_BIT */
    /* 如果d为0，说明上面已经移动完毕了 */
    if (d != 0) {
      nanmath_digit *tmp, shift, mask, r, rr;
      mask = (((nanmath_digit)1) << d) - 1;      /* 进位掩码 */
      shift = DIGIT_BIT - (int)d;           /* 一切位了取进位 */
      tmp = _dp;
      
      r = 0;
      /* 每位都与剩余部分参与运算 */
      for (int x = 0; x < _used; x++) {
        rr = (*tmp >> shift) & mask;        /* 取进位 */
        *tmp = ((*tmp << d) | r) & NANMATH_MASK; /* 进行位移 */
        tmp++;
        
        /* 保存进位 */
        r = rr;
      }
      
      /* 设置最后的进位 */
      if (r != 0) {
        _dp[_used++] = r;
      }
    }
    
    clamp();
    return NANMATH_OK;
  }
  
  int nanmath_int::rsh(nanmath_size b) {
    int res;
    /* 不用移动 */
    if (b <= 0) {
      return NANMATH_OK;
    }
    
    nanmath_size d = b / DIGIT_BIT;/* 有多少个位精度 */
    
    /* 先移动整的 */
    if (d > 0) {
      if ((res = rsh_d(d)) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 移动剩下的余数 */
    d = b % DIGIT_BIT;    /* 保证d < DIGIT_BIT */
    
    /* 如果d为0，说明上面已经移动完毕了 */
    if (d != 0) {
      nanmath_digit *tmp, shift, mask, r, rr;
      mask = (((nanmath_digit)1) << d) - 1;      /* 进位掩码 */
      shift = DIGIT_BIT - (int)d;           /* 一切位了取进位 */
      
      tmp = _dp + _used - 1;
      
      /* 每位都与剩余部分参与运算 */
      r = 0;
      for (int x = _used - 1; x >= 0; x--) {
        rr = *tmp & mask;                   /* 取进位 */
        *tmp = (*tmp >> d) | (r << shift);  /* 进行位移 */
        tmp--;
      
        /* 保存进位 */
        r = rr;
      }
    }
    
    clamp();
    return NANMATH_OK;
  }
  
  /* 左移精度b位 */
  int nanmath_int::lsh_d(nanmath_size b) {
    int res;
    if (b <= 0) {
      return NANMATH_OK;
    }
    
    if (_alloc < _used + b) {
      if ((res = grow(_used + b)) != NANMATH_OK) {
        return res;
      }
    }
    
    nanmath_digit *top, *bottom;
    _used += b;
    top = _dp + _used - 1;          /* 最高位 */
    bottom = _dp + _used - 1 - b;   /* 当前的有效位 */
    
    /* 填充要设置的b位 */
    for (int x = _used - 1; x >= b; x--) {
      *top-- = *bottom--;
    }
    
    /* 清空原来的位置 */
    top = _dp;
    for (int x = 0; x < b; x++) {
      *top++ = 0;
    }
    return NANMATH_OK;
  }
  
  /* 右移b位 */
  int nanmath_int::rsh_d(nanmath_size b) {
    if (b <= 0) {
      return NANMATH_OK;
    }
    
    /* 如果要移动的位数大于当的已经使用的位数，好吧直接清0 */
    if (_used <= b) {
      zero();
      return NANMATH_OK;
    }
    
    /* 移动到底 */
    int x;
    nanmath_digit *bottom = _dp;
    nanmath_digit *top = _dp + b;
    
    /* 直接将要移动的位值放到它的位置上去 */
    for (x = 0; x < (_used - b); x++) {
      *bottom++ = *top++;
    }
    
    /* 清0顶部的位 */
    for (; x < _used; x++) {
      *bottom++ = 0;
    }
      
    /* 移除额外的位 */
    _used -= b;
    return NANMATH_OK;
  }
  
  int nanmath_int::count_bits() {
    if (_used == 0) {
      return 0;
    }

    /* 计算除最高digit的位总和 */
    int r = (_used - 1) * DIGIT_BIT;
    /* 最高digit的位 + fffff ... */
    /* 加上最高digit的位 */
    nanmath_digit q = _dp[_used - 1];
    while (q > ((nanmath_digit)0)) {
      ++r;
      q >>= ((nanmath_digit)1);
    }
    return r;
  }
  
  static const int lnz[16] = {
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
  };
  int nanmath_int::count_lsb() {
    int x;
    nanmath_digit qq;
    
    if (iszero())
      return 0;
    
    /* 遍历低位，直到一个位不为0 */
    for (x = 0; x < _used && _dp[x] == 0; x++);
    nanmath_digit q = _dp[x];
    x *= DIGIT_BIT;
    
    /* 扫描精度位中的二进制位情况
     * 4位递进
     */
    if ((q & 1) == 0) {
      do {
        qq = q & 0x0F;
        x  += lnz[qq];
        q >>= 4;
      } while (qq == 0);
    }
    return x;
  }
  
  int nanmath_int::and_d(nanmath_digit d) {
    if (iszero()) {
      return NANMATH_OK;
    }
    
    _dp[0] &= d;
    
    return NANMATH_OK;
  }
  
  int nanmath_int::and_v(nanmath_int &b) {
    int res, ix, px;
    nanmath_int t, *x;
    
    if (_used > b.get_used()) {
      if ((res = t.copy(*this)) != NANMATH_OK) {
        return res;
      }
      px = b.get_used();
      x = &b;
    } else {
      if ((res = t.copy(b)) != NANMATH_OK) {
        return res;
      }
      px = _used;
      x = this;
    }
    
    nanmath_digit *tmp = cast(nanmath_digit, t.get_digit());
    for (ix = 0; ix < px; ix++) {
      tmp[ix] &= x->getv(ix);
    }
    
    for (; ix < t.get_used(); ix++) {
      tmp[ix] = 0;
    }
    
    t.clamp();
    return copy(t);
  }
  
  int nanmath_int::or_d(nanmath_digit d) {
    if (iszero()) {
      return NANMATH_OK;
    }
    
    _dp[0] |= d;
    
    return NANMATH_OK;
  }
  
  int nanmath_int::or_v(nanmath_int &b) {
    int res, ix, px;
    nanmath_int t, *x;
    
    if (_used > b.get_used()) {
      if ((res = t.copy(*this)) != NANMATH_OK) {
        return res;
      }
      px = b.get_used();
      x = &b;
    } else {
      if ((res = t.copy(b)) != NANMATH_OK) {
        return res;
      }
      px = _used;
      x = this;
    }
    
    nanmath_digit *tmp = cast(nanmath_digit, t.get_digit());
    for (ix = 0; ix < px; ix++) {
      tmp[ix] |= x->getv(ix);
    }
    
    for (; ix < t.get_used(); ix++) {
      tmp[ix] = 0;
    }
    
    t.clamp();
    return copy(t);
  }
}
















































