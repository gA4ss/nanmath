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
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return addmod(b, c);
  }

  int nanmath_int::submod(nanmath_int &b, nanmath_int &c) {
    int res;
    if ((res = sub(b)) != NANMATH_OK) {
      return res;
    }
    res = mod(c);
    return res;
  }
  
  int nanmath_int::submod(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    int res;
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return submod(b, c);
  }
  
  int nanmath_int::sqrmod(nanmath_int &b) {
    int res;
    
    if ((res = sqr()) != NANMATH_OK) {
      return res;
    }
    res = mod(b);
    return res;
  }
  
  int nanmath_int::sqrmod(nanmath_int &a, nanmath_int &b) {
    int res;
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return sqrmod(b);
  }
  
  /* a = a * b mod c */
  int nanmath_int::mulmod(nanmath_int &b, nanmath_int &c) {
    int res;
    
    if ((res = mul(b)) != NANMATH_OK) {
      return res;
    }
    res = mod(c);
    return res;
  }
  
  int nanmath_int::mulmod(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    int res;
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return mulmod(b, c);
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
    
    /* 太小了 */
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
  
  /* a = a^b mod c */
  int nanmath_int::exptmod(nanmath_int &b, nanmath_int &c) {
    /* 模数P必须是整数 */
    if (c.get_sign() == NANMATH_NEG) {
      return NANMATH_VAL;
    }
    
    /* 如果指数b是负数则我们开始递归 */
    if (b.get_sign() == NANMATH_NEG) {
      int res;
      nanmath_int tb;
      
      /* 第一次计算 1/b mod c */
      if ((res = invmod(c)) != NANMATH_OK) {
        return res;
      }
      
      if ((res = tb.copy(b)) != NANMATH_OK) {
        return res;
      }
      
      /* 现在获取 |b| */
      if ((res = tb.abs()) != NANMATH_OK) {
        return res;
      }
      
      /* 现在计算 (1/a)**|b| 替代 a**b 当b < 0时 */
      res = exptmod(tb, c);
      return res;
    }
    
    /* modified diminished radix reduction */
#if defined(BN_MP_REDUCE_IS_2K_L_C) && defined(BN_MP_REDUCE_2K_L_C) && defined(BN_S_MP_EXPTMOD_C)
    if (mp_reduce_is_2k_l(P) == MP_YES) {
      return s_mp_exptmod(G, X, P, Y, 1);
    }
#endif
    
#ifdef BN_MP_DR_IS_MODULUS_C
    /* is it a DR modulus? */
    dr = mp_dr_is_modulus(P);
#else
    /* default to no */
    dr = 0;
#endif
    
#ifdef BN_MP_REDUCE_IS_2K_C
    /* if not, is it a unrestricted DR modulus? */
    if (dr == 0) {
      dr = mp_reduce_is_2k(P) << 1;
    }
#endif
    
    /* if the modulus is odd or dr != 0 use the montgomery method */
#ifdef BN_MP_EXPTMOD_FAST_C
    if (mp_isodd (P) == 1 || dr !=  0) {
      return mp_exptmod_fast (G, X, P, Y, dr);
    } else {
#endif
#ifdef BN_S_MP_EXPTMOD_C
      /* otherwise use the generic Barrett reduction technique */
      return s_mp_exptmod (G, X, P, Y, 0);
#else
      /* no exptmod for evens */
      return MP_VAL;
#endif
#ifdef BN_MP_EXPTMOD_FAST_C
    }
#endif
  }
  
  int nanmath_int::exptmod(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    int res;
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return exptmod(b, c);
  }
  
  /* Y = G^X mod P */
  int nanmath_int::s_exptmod(nanmath_int &G, nanmath_int &X, nanmath_int &P, nanmath_int &Y, int redmode) {
    const int TAB_SIZE = 256;
    nanmath_int M[TAB_SIZE], res, mu;
    nanmath_digit buf;
    int err, bitbuf, bitcpy, bitcnt, mode, digidx, x, y, winsize;
  
    /* 窗口长度 */
    x = X.count_bits();
    if (x <= 7) {
      winsize = 2;
    } else if (x <= 36) {
      winsize = 3;
    } else if (x <= 140) {
      winsize = 4;
    } else if (x <= 450) {
      winsize = 5;
    } else if (x <= 1303) {
      winsize = 6;
    } else if (x <= 3529) {
      winsize = 7;
    } else {
      winsize = 8;
    }
    
    /* 选择分解模式
     * 如果为0,则选择巴雷特分解算法
     */
    if (redmode == 0) {
      if ((err = s_reduce_setup(mu, P)) != NANMATH_OK) {
        return err;
      }
    } else {
      if ((err = s_reduce_2k_setup_l(P, mu)) != NANMATH_OK) {
        return err;
      }
    }
    
    /* 创建 M 表
     *
     * M表包含基的冥
     * 例如 M[x] = G^x mod P
     *
     * 第一个半表不是被通过M[0]与M[1]计算得到
     */
    if ((err = M[1].mod(G, P)) != NANMATH_OK) {
      return err;
    }
    
    /* 通过 M[1]的(winsize-1)次的平方 计算得到 M[1<<(winsize-1)] */
    if ((err = M[1 << (winsize - 1)].copy(M[1])) != NANMATH_OK) {
      return err;
    }
    
    /* (winsize - 1)次平方 */
    for (x = 0; x < (winsize - 1); x++) {
      /* 平方它 */
      if ((err = M[1 << (winsize - 1)].sqr()) != NANMATH_OK) {
        return err;
      }
      
      /* 分解模P */
      if ((err = redux(M[1 << (winsize - 1)], P, mu)) != NANMATH_OK) {
        return err;
      }
    }
    
    /* 创建表,每项: M[x] = M[x-1] * M[1] (mod P)
     * for x = (2**(winsize - 1) + 1) to (2**winsize - 1)
     */
    for (x = (1 << (winsize - 1)) + 1; x < (1 << winsize); x++) {
      if ((err = M[x].mul(M[x - 1], M[1])) != NANMATH_OK) {
        return err;
      }
      if ((err = redux(M[x], P, mu)) != NANMATH_OK) {
        return err;
      }
    }
    
    /* setup result */
    if ((err = mp_init (&res)) != MP_OKAY) {
      goto LBL_MU;
    }
    mp_set (&res, 1);
    
    /* set initial mode and bit cnt */
    mode   = 0;
    bitcnt = 1;
    buf    = 0;
    digidx = X->used - 1;
    bitcpy = 0;
    bitbuf = 0;
    
    for (;;) {
      /* grab next digit as required */
      if (--bitcnt == 0) {
        /* if digidx == -1 we are out of digits */
        if (digidx == -1) {
          break;
        }
        /* read next digit and reset the bitcnt */
        buf    = X->dp[digidx--];
        bitcnt = (int) DIGIT_BIT;
      }
      
      /* grab the next msb from the exponent */
      y     = (buf >> (mp_digit)(DIGIT_BIT - 1)) & 1;
      buf <<= (mp_digit)1;
      
      /* if the bit is zero and mode == 0 then we ignore it
       * These represent the leading zero bits before the first 1 bit
       * in the exponent.  Technically this opt is not required but it
       * does lower the # of trivial squaring/reductions used
       */
      if (mode == 0 && y == 0) {
        continue;
      }
      
      /* if the bit is zero and mode == 1 then we square */
      if (mode == 1 && y == 0) {
        if ((err = mp_sqr (&res, &res)) != MP_OKAY) {
          goto LBL_RES;
        }
        if ((err = redux (&res, P, &mu)) != MP_OKAY) {
          goto LBL_RES;
        }
        continue;
      }
      
      /* else we add it to the window */
      bitbuf |= (y << (winsize - ++bitcpy));
      mode    = 2;
      
      if (bitcpy == winsize) {
        /* ok window is filled so square as required and multiply  */
        /* square first */
        for (x = 0; x < winsize; x++) {
          if ((err = mp_sqr (&res, &res)) != MP_OKAY) {
            goto LBL_RES;
          }
          if ((err = redux (&res, P, &mu)) != MP_OKAY) {
            goto LBL_RES;
          }
        }
        
        /* then multiply */
        if ((err = mp_mul (&res, &M[bitbuf], &res)) != MP_OKAY) {
          goto LBL_RES;
        }
        if ((err = redux (&res, P, &mu)) != MP_OKAY) {
          goto LBL_RES;
        }
        
        /* empty window and reset */
        bitcpy = 0;
        bitbuf = 0;
        mode   = 1;
      }
    }
    
    /* if bits remain then square/multiply */
    if (mode == 2 && bitcpy > 0) {
      /* square then multiply if the bit is set */
      for (x = 0; x < bitcpy; x++) {
        if ((err = mp_sqr (&res, &res)) != MP_OKAY) {
          goto LBL_RES;
        }
        if ((err = redux (&res, P, &mu)) != MP_OKAY) {
          goto LBL_RES;
        }
        
        bitbuf <<= 1;
        if ((bitbuf & (1 << winsize)) != 0) {
          /* then multiply */
          if ((err = mp_mul (&res, &M[1], &res)) != MP_OKAY) {
            goto LBL_RES;
          }
          if ((err = redux (&res, P, &mu)) != MP_OKAY) {
            goto LBL_RES;
          }
        }
      }
    }
    
    /* 复制结果 */
    err = Y.copy(res);
    return err;
  }
}