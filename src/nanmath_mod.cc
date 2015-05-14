#include "nanmath.h"

namespace nanmath {
  
  int nanmath_int::mod_2x(nanmath_digit v) {
    /* 如果 v <= 0 然后清空结果 */
    if (v <= 0) {
      zero();
      return NANMATH_OK;
    }
    
    /* 如果模数v大于被模数则直接返回 */
    if (v >= cast_f(nanmath_digit, _used * DIGIT_BIT)) {
      return NANMATH_OK;
    }
    
    /* 清0模数的高位 */
    for (nanmath_digit x = (v / DIGIT_BIT) + ((v % DIGIT_BIT) == 0 ? 0 : 1);
         x < _used; x++) {
      _dp[x] = 0;
    }

    _dp[v / DIGIT_BIT] &=
    cast_f(nanmath_digit,
           ((((nanmath_digit)1) <<
             (((nanmath_digit)v) % DIGIT_BIT)) - ((nanmath_digit) 1)));
    clamp();
    return NANMATH_OK;
  }
  
  int nanmath_int::mod_d(nanmath_digit v, nanmath_digit *r) {
    return div_d(v, r);
  }
  
  int nanmath_int::mod(nanmath_int &v) {
    nanmath_int r;
    int res;
  
    if ((res = div(v, r)) != NANMATH_OK) {
      return res;
    }
  
    if (r.iszero() ||
        r.get_sign() == v.get_sign()) {
      res = copy(r);
    } else {
      if ((res = r.add(v)) != NANMATH_OK)
        return res;
      
      res = copy(r);
    }
    
    return res;
  }
  
  int nanmath_int::mod(nanmath_int &a, nanmath_int &b) {
    int res;
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return mod(b);
  }
  
  /* a + b (mod c) */
  int nanmath_int::addmod(nanmath_int &b, nanmath_int &c) {
    int res;
    if ((res = add(b)) != NANMATH_OK) {
      return res;
    }
    
    if ((res = mod(c)) != NANMATH_OK) {
      return res;
    }
    
    return NANMATH_OK;
  }
  
  int nanmath_int::addmod(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    int res;
    if ((res = add(a, b)) != NANMATH_OK) {
      return res;
    }
    
    if ((res = mod(c)) != NANMATH_OK) {
      return res;
    }
    
    return NANMATH_OK;
  }

  int nanmath_int::invmod(nanmath_int &b) {
    if (b.get_sign() == NANMATH_NEG || b.iszero()) {
      return NANMATH_VAL;
    }
    
    /* 如果模数是奇数 */
    if (b.isodd()) {
      return s_invmod_fast(*this, b, *this);
    }
    
    return s_invmod_slow(*this, b, *this);
  }
  
  int nanmath_int::invmod(nanmath_int &a, nanmath_int &b) {
    int res;
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return invmod(b);
  }
  
  /* 计算模逆使用二分扩展欧几里德算法, c = 1/a mod b */
  int nanmath_int::s_invmod_fast(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    nanmath_int x, y, u, v, B, D;
    int res, neg;
    
    /* b必须是奇数 */
    if (b.iseven()) {
      return NANMATH_VAL;
    }

    /* x 等于 模数, y == 余数 */
    if ((res = x.copy(b)) != NANMATH_OK) {
      return res;
    }
    
    /* y = |a| */
    if ((res = y.mod(a, b)) != NANMATH_OK) {
      return res;
    }
    
    /* u=x, v=y, A=1, B=0, C=0, D=1 */
    if ((res = u.copy(x)) != NANMATH_OK) {
      return res;
    }
    if ((res = v.copy(y)) != NANMATH_OK) {
      return res;
    }
    D.set(1);
    
  top:
    /* 当u是偶数 */
    while (u.iseven() == 1) {
      /* u = u/2 */
      if ((res = u.div_2()) != NANMATH_OK) {
        return res;
      }
      /* 如果 B 是奇数 */
      if (B.isodd()) {
        if ((res = B.sub(x)) != NANMATH_OK) {
          return res;
        }
      }
      /* B = B/2 */
      if ((res = B.div_2()) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 当 v 是偶数 */
    while (v.iseven()) {
      /* v = v/2 */
      if ((res = v.div_2()) != NANMATH_OK) {
        return res;
      }
      /* 如果 D 是奇数 */
      if (D.isodd()) {
        /* D = (D-x)/2 */
        if ((res = D.sub(x)) != NANMATH_OK) {
          return res;
        }
      }
      /* D = D/2 */
      if ((res = D.div_2()) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 如果 u >= v */
    if (u.cmp(v) != NANMATH_LT) {
      /* u = u - v, B = B - D */
      if ((res = u.sub(v)) != NANMATH_OK) {
        return res;
      }
      
      if ((res = B.sub(D)) != NANMATH_OK) {
        return res;
      }
    } else {
      /* v - v - u, D = D - B */
      if ((res = v.sub(u)) != NANMATH_OK) {
        return res;
      }
      
      if ((res = D.sub(B)) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 如果不是0则返回top */
    if (u.iszero() == 0) {
      goto top;
    }
    
    /* 现在 a = C, b = D, gcd == g*v */
    
    /* 如果 v != 1 则没有逆 */
    if (v.cmp_d(1) != NANMATH_EQ) {
      return NANMATH_VAL;
    }
    
    /* b 现在是逆 */
    neg = a.get_sign();
    while (D.get_sign() == NANMATH_NEG) {
      if ((res = D.add(b)) != NANMATH_OK) {
        return res;
      }
    }
    
    if ((res = c.copy(D)) != NANMATH_OK) {
      return res;
    }
    c.set_sign(neg);
    
    return res;
  }
  
  /* hac 14.61, pp608 */
  int nanmath_int::s_invmod_slow(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    nanmath_int x, y, u, v, A, B, C, D;
    int res;
    
    /* b 不能为 负 */
    if (b.get_sign() == NANMATH_NEG || b.iszero()) {
      return NANMATH_VAL;
    }
    
    /* x = a, y = b */
    if ((res = x.mod(a, b)) != NANMATH_OK) {
      return res;
    }
    if ((res = y.copy(b)) != NANMATH_OK) {
      return res;
    }
    
    /* 如果x,y都是偶数,那么返回错误 */
    if (x.iseven() && y.iseven()) {
      return NANMATH_VAL;
    }
    
    /* u=x, v=y, A=1, B=0, C=0, D=1 */
    if ((res = u.copy(x)) != NANMATH_OK) {
      return res;
    }
    if ((res = v.copy(y)) != NANMATH_OK) {
      return res;
    }
    A.set(1);
    D.set(1);
    
  top:
    /* 当 u 是偶数 */
    while (u.iseven()) {
      /* u = u/2 */
      if ((res = u.div_2()) != NANMATH_OK) {
        return res;
      }
      /* 如果A或者B是奇数 */
      if (A.isodd() || B.isodd()) {
        /* A = (A+y)/2, B = (B-x)/2 */
        if ((res = A.add(y)) != NANMATH_OK) {
          return res;
        }
        if ((res = B.sub(x)) != NANMATH_OK) {
          return res;
        }
      }
      /* A = A/2, B = B/2 */
      if ((res = A.div_2()) != NANMATH_OK) {
        return res;
      }
      if ((res = B.div_2()) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 当v是偶数 */
    while (v.iseven()) {
      /* v = v/2 */
      if ((res = v.div_2()) != NANMATH_OK) {
        return res;
      }
      /* 如果 C 或者 D 是奇数 */
      if (C.isodd() || D.isodd()) {
        /* C = (C+y)/2, D = (D-x)/2 */
        if ((res = C.add(y)) != NANMATH_OK) {
          return res;
        }
        if ((res = D.sub(x)) != NANMATH_OK) {
          return res;
        }
      }
      /* C = C/2, D = D/2 */
      if ((res = C.div_2()) != NANMATH_OK) {
        return res;
      }
      if ((res = D.div_2()) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 如果 u >= v */
    if (u.cmp(v) != NANMATH_LT) {
      /* u = u - v, A = A - C, B = B - D */
      if ((res = u.sub(v)) != NANMATH_OK) {
        return res;
      }
      
      if ((res = A.sub(C)) != NANMATH_OK) {
        return res;
      }
      
      if ((res = B.sub(D)) != NANMATH_OK) {
        return res;
      }
    } else {
      /* v - v - u, C = C - A, D = D - B */
      if ((res = v.sub(u)) != NANMATH_OK) {
        return res;
      }
      
      if ((res = C.sub(A)) != NANMATH_OK) {
        return res;
      }
      
      if ((res = D.sub(B)) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 如果不是0则跳到top */
    if (u.iszero() == 0)
      goto top;
    
    /* 现在 a = C, b = D, gcd == g*v */
    
    /* 如果 v != 1  无逆 */
    if (v.cmp_d(1) != NANMATH_EQ) {
      return NANMATH_VAL;
    }
    
    while (C.cmp_d(0) == NANMATH_LT) {
      if ((res = C.add(b)) != NANMATH_OK) {
        return res;
      }
    }
    
    /* 太大了 */
    while (s_cmp_mag(C, b) != NANMATH_LT) {
      if ((res = C.sub(b)) != NANMATH_OK) {
        return res;
      }
    }
    
    /* C现在是逆 */
    return c.copy(C);
  }

  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
}