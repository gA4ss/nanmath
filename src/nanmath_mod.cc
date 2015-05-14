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

  int nanmath_int::invmod(nanmath_int &b, nanmath_int &c) {
    if (b.get_sign() == NANMATH_NEG || b.iszero()) {
      return NANMATH_VAL;
    }
    
    /* 如果模数是奇数 */
    if (b.isodd()) {
      return s_invmod_fast(*this, b, c);
    } else {
      
    }
    
    return NANMATH_OK;
  }
  
  int nanmath_int::invmod(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    int res;
    if ((res = copy(a)) != NANMATH_OK)
      return res;
    
    return invmod(b, c);
  }
  
  int nanmath_int::s_invmod_fast(nanmath_int &a, nanmath_int &b, nanmath_int &c) {
    nanmath_int x, y, u, v, B, D;
    int res, neg;
    
    /* b必须是奇数 */
    if (b.iseven()) {
      return NANMATH_VAL;
    }
#if 0
    /* x 等于 模数, y == 余数 */
    if ((res = mp_copy (b, &x)) != MP_OKAY) {
      goto LBL_ERR;
    }
    
    /* we need y = |a| */
    if ((res = mp_mod (a, b, &y)) != MP_OKAY) {
      goto LBL_ERR;
    }
    
    /* 3. u=x, v=y, A=1, B=0, C=0,D=1 */
    if ((res = mp_copy (&x, &u)) != MP_OKAY) {
      goto LBL_ERR;
    }
    if ((res = mp_copy (&y, &v)) != MP_OKAY) {
      goto LBL_ERR;
    }
    mp_set (&D, 1);
    
  top:
    /* 4.  while u is even do */
    while (mp_iseven (&u) == 1) {
      /* 4.1 u = u/2 */
      if ((res = mp_div_2 (&u, &u)) != MP_OKAY) {
        goto LBL_ERR;
      }
      /* 4.2 if B is odd then */
      if (mp_isodd (&B) == 1) {
        if ((res = mp_sub (&B, &x, &B)) != MP_OKAY) {
          goto LBL_ERR;
        }
      }
      /* B = B/2 */
      if ((res = mp_div_2 (&B, &B)) != MP_OKAY) {
        goto LBL_ERR;
      }
    }
    
    /* 5.  while v is even do */
    while (mp_iseven (&v) == 1) {
      /* 5.1 v = v/2 */
      if ((res = mp_div_2 (&v, &v)) != MP_OKAY) {
        goto LBL_ERR;
      }
      /* 5.2 if D is odd then */
      if (mp_isodd (&D) == 1) {
        /* D = (D-x)/2 */
        if ((res = mp_sub (&D, &x, &D)) != MP_OKAY) {
          goto LBL_ERR;
        }
      }
      /* D = D/2 */
      if ((res = mp_div_2 (&D, &D)) != MP_OKAY) {
        goto LBL_ERR;
      }
    }
    
    /* 6.  if u >= v then */
    if (mp_cmp (&u, &v) != MP_LT) {
      /* u = u - v, B = B - D */
      if ((res = mp_sub (&u, &v, &u)) != MP_OKAY) {
        goto LBL_ERR;
      }
      
      if ((res = mp_sub (&B, &D, &B)) != MP_OKAY) {
        goto LBL_ERR;
      }
    } else {
      /* v - v - u, D = D - B */
      if ((res = mp_sub (&v, &u, &v)) != MP_OKAY) {
        goto LBL_ERR;
      }
      
      if ((res = mp_sub (&D, &B, &D)) != MP_OKAY) {
        goto LBL_ERR;
      }
    }
    
    /* if not zero goto step 4 */
    if (mp_iszero (&u) == 0) {
      goto top;
    }
    
    /* now a = C, b = D, gcd == g*v */
    
    /* if v != 1 then there is no inverse */
    if (mp_cmp_d (&v, 1) != MP_EQ) {
      res = MP_VAL;
      goto LBL_ERR;
    }
    
    /* b is now the inverse */
    neg = a->sign;
    while (D.sign == MP_NEG) {
      if ((res = mp_add (&D, b, &D)) != MP_OKAY) {
        goto LBL_ERR;
      }
    }
    mp_exch (&D, c);
    c->sign = neg;
    res = MP_OKAY;
    
  LBL_ERR:mp_clear_multi (&x, &y, &u, &v, &B, &D, NULL);
    return res;
#endif
    return 0;
  }
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
}