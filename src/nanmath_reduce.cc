#include "nanmath.h"

namespace nanmath {

  /* 预计算Barrett reduction算法的值 
   * res = a / 2^b
   */
  int nanmath_int::s_reduce_setup(nanmath_int &a, nanmath_int &b) {
    int res;
    
    if ((res = a.bin_expt(b.get_used() * 2 * DIGIT_BIT)) != NANMATH_OK) {
      return res;
    }
    return a.div(b);
  }
  
  int nanmath_int::s_reduce_2k_setup(nanmath_int &a, nanmath_digit *d) {
    int res;
    nanmath_int tmp;
    
    int p = a.count_bits();
    if ((res =  tmp.bin_expt(p)) != NANMATH_OK) {
      return res;
    }
    
    if ((res = s_sub(tmp, a, tmp)) != NANMATH_OK) {
      return res;
    }
    
    *d = tmp.getv(0);
    return NANMATH_OK;
  }
  
  int nanmath_int::s_reduce_2k_setup_l(nanmath_int &a, nanmath_int &d) {
    int res;
    nanmath_int tmp;
    
    if ((res = tmp.bin_expt(a.count_bits())) != NANMATH_OK) {
      return res;
    }
    
    if ((res = s_sub(tmp, a, d)) != NANMATH_OK) {
      return res;
    }
    
    return res;
  }
  
  int nanmath_int::s_reduce(nanmath_int &a, nanmath_int &b, nanmath_int &c, int redmode) {
    if (redmode == 0) return reduce(a, b, c);
    //else return ;
    return NANMATH_OK;
  }
  
  /* Barrett reduction算法
   * 分解x mod m, 指定 0 < x < m^2, mu是通过 reduce_setup预计算得到 */
  int nanmath_int::reduce(nanmath_int &x, nanmath_int &m, nanmath_int &mu) {
    nanmath_int q;
    int res, um = m.get_used();
  
    /* q = x */
    if ((res = q.copy(x)) != NANMATH_OK) {
      return res;
    }
  
    /* q1 = x / b^(k-1)  */
    q.rsh_d(um - 1);
  
    if (((unsigned long)um) > (((nanmath_digit)1) << (DIGIT_BIT - 1))) {
      if ((res = q.mul(mu)) != NANMATH_OK) {
        return res;
      }
    } else {
      if ((res = s_mul_high_digs_(q, mu, q, um)) != NANMATH_OK) {
        return res;
      }
    }
  
    /* q3 = q2 / b^(k+1) */
    q.rsh_d(um + 1);
  
    /* x = x mod b^(k+1), quick (no division) */
    if ((res = x.mod_2x(DIGIT_BIT * (um + 1))) != NANMATH_OK) {
      return res;
    }
  
    /* q = q * m mod b**(k+1), quick (no division) */
    if ((res = s_mul_high_digs_(q, m, q, um + 1)) != NANMATH_OK) {
      return res;
    }
  
    /* x = x - q */
    if ((res = x.sub(q)) != NANMATH_OK) {
      return res;
    }
  
    /* 如果 x < 0, 加上 b^(k+1) 到 x */
    if (x.cmp_d(0) == NANMATH_LT) {
      q.set(1);
      
      if ((res = q.lsh_d(um + 1)) != NANMATH_OK)
        return res;
      if ((res = x.add(q)) != NANMATH_OK)
        return res;
    }
  
    /* 如果太大则后退 */
    while (x.cmp(m) != NANMATH_LT) {
      if ((res = s_sub(x, m, x)) != NANMATH_OK) {
        return res;
      }
    }/* end while */
  
    return res;
  }
  
  int nanmath_int::s_reduce_is_2k(nanmath_int &a) {
    int ix, iy, iw;
    nanmath_digit iz;
    
    if (a.get_used() == 0) {
      return NANMATH_NO;
    } else if (a.get_used() == 1) {
      return NANMATH_YES;
    } else if (a.get_used() > 1) {
      iy = a.count_bits();
      iz = 1;
      iw = 1;
      
      /* 从第二精度位开始测试每位,必须是1 */
      for (ix = DIGIT_BIT; ix < iy; ix++) {
        if ((a.getv(iw) & iz) == 0) {
          return NANMATH_NO;
        }
        iz <<= 1;
        if (iz > cast_f(nanmath_digit, NANMATH_MASK)) {
          ++iw;
          iz = 1;
        }
      }/* end for */
    }/* end else if */
    return NANMATH_YES;
  }
  
  int nanmath_int::s_reduce_is_2k_l(nanmath_int &a) {
    int ix, iy;
    
    if (a.get_used() == 0) {
      return NANMATH_NO;
    } else if (a.get_used() == 1) {
      return NANMATH_YES;
    } else if (a.get_used() > 1) {
      for (iy = ix = 0; ix < a.get_used(); ix++) {
        if (a.getv(ix) == NANMATH_MASK) {
          ++iy;
        }
      }
      return (iy >= (a.get_used() / 2)) ? NANMATH_YES : NANMATH_NO;
    }
    return NANMATH_NO;
  }
  
}
