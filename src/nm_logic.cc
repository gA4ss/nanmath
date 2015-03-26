#include "nanmath.h"

namespace nanmath {
  
  /* 向左移动2进制b位 */
  int nm_int::lsh(nm_size b) {
    if (b <= 0) {
      return NM_OK;
    }
    
    nm_size d = b / DIGIT_BIT;/* 有多少个位精度 */
    nm_size g = (nm_size)(_used + d + 1);
    if (_alloc < g) {
      if (grow(g) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 首先移动整数倍 */
    if (d > 0) {
      if (lsh_d(d) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 移动剩下的余数 */
    d = b % DIGIT_BIT;    /* 保证d < DIGIT_BIT */
    /* 如果d为0，说明上面已经移动完毕了 */
    if (d != 0) {
      nm_digit *tmp, shift, mask, r, rr;
      mask = (((nm_digit)1) << d) - 1;      /* 进位掩码 */
      shift = DIGIT_BIT - (int)d;           /* 一切位了取进位 */
      tmp = _dp;
      
      r = 0;
      /* 每位都与剩余部分参与运算 */
      for (int x = 0; x < _used; x++) {
        rr = (*tmp >> shift) & mask;        /* 取进位 */
        *tmp = ((*tmp << d) | r) & NM_MASK; /* 进行位移 */
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
    return NM_OK;
  }
  
  int nm_int::rsh(nm_size b) {
    /* 不用移动 */
    if (b <= 0)
      return NM_OK;
    
    /* 如果要移动的位数大于当的已经使用的位数，好吧直接清0 */
    if ((_used * DIGIT_BIT) <= b) {
      zero();
      return NM_OK;
    }
    
    nm_size d = b / DIGIT_BIT;/* 有多少个位精度 */
    
    /* 先移动整的 */
    if (d > 0) {
      if (rsh_d(d) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 移动剩下的余数 */
    d = b % DIGIT_BIT;    /* 保证d < DIGIT_BIT */
    
    /* 如果d为0，说明上面已经移动完毕了 */
    if (d != 0) {
      nm_digit *tmp, shift, mask, r, rr;
      mask = (((nm_digit)1) << d) - 1;      /* 进位掩码 */
      shift = DIGIT_BIT - (int)d;           /* 一切位了取进位 */
      
      tmp = _dp + _used - 1;
      
      /* 每位都与剩余部分参与运算 */
      r = 0;
      for (int x = _used - 1; x >= 0; x--) {
        rr = *tmp & mask;                   /* 取进位 */
        *tmp = ((*tmp >> d) | r) & NM_MASK; /* 进行位移 */
        tmp--;
      
        /* 保存进位 */
        r = rr;
      }
    }
    
    clamp();
    return NM_OK;
  }
  
  /* 左移精度b位 */
  int nm_int::lsh_d(nm_size b) {
    if (b <= 0) {
      return NM_OK;
    }
    
    if (_alloc < _used + b) {
      if (grow(_used + b) != NM_OK) {
        return _lasterr;
      }
    }
    
    nm_digit *top, *bottom;
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
    return NM_OK;
  }
  
  /* 右移b位 */
  int nm_int::rsh_d(nm_size b) {
    if (b <= 0)
      return NM_OK;
    
    /* 如果要移动的位数大于当的已经使用的位数，好吧直接清0 */
    if (_used <= b) {
      zero();
      return NM_OK;
    }
    
    /* 移动到底 */
    int x;
    nm_digit *bottom = _dp;
    nm_digit *top = _dp + b;
    
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
    
    return NM_OK;
  }
  
}