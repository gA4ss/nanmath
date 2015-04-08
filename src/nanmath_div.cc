#include "nanmath.h"

namespace nanmath {
  /* 快速测试b是否是2的次方 */
  static int is_power_of_two(nanmath_digit b, int *p) {
    int x;
    
    /* 测试b是否为0,b是否是奇数 */
    if ((b==0) || (b & (b-1))) {
      return 0;
    }
    
    /* 循环匹配 */
    for (x = 0; x < DIGIT_BIT; x++) {
      if (b == (((nanmath_digit)1)<<x)) {
        *p = x;
        return 1;
      }
    }
    return 0;
  }
  
  int nanmath_int::div_2() {
    if ((_used == 0) || (_alloc == 0) || (_dp == NULL)) {
      zero();
      return NM_OK;
    }
    
    nanmath_digit *tmpa = cast(nanmath_digit, _dp + _used - 1);
    nanmath_digit r = 0, rr = 0;
    
    for (int x = _used - 1; x >= 0; x--) {
      rr = *tmpa & 1;
      *tmpa = (*tmpa >> 1) | (r << (DIGIT_BIT - 1));
      tmpa--;
      r = rr;
    }
    
    clamp();
    return NM_OK;
  }
  
  int nanmath_int::div_d(nanmath_digit v, nanmath_digit *r) {
    /* 除数不能为0 */
    if (v == 0) {
      return set_lasterr(NM_VAL, cast_f(char*, __FUNCTION__));
    }
    
    /* 除数是1，被除数是0 */
    if (v == 1 || iszero()) {
      if (r) *r = 0;
      return NM_OK;
    }
    
    int ix;
    /* v是2的次方,相当于左移动n位 */
    if (is_power_of_two(v, &ix) == 1) {
      if (r) {
        /* 求余数,掩码不为0的部分 */
        nanmath_digit rem = _dp[0] & ((((nanmath_digit)1)<<ix) - 1);
        *r = rem;
      }
      
      return rsh(ix);
    }
    
    /* 分配商的内存 */
    nanmath_int q;
    if (q.allocs(_used) != NM_OK) {
      return _lasterr;
    }
    
    q.set_used(_used);
    q.set_sign(_sign);
    nanmath_word w = 0;
    nanmath_digit t;
    nanmath_digit *tmpq = cast(nanmath_digit, q.get_digit());
    for (ix = _used - 1; ix >= 0; ix--) {
      /* 每次使用一个字的大小来做被除数 */
      w = (w << ((nanmath_word)DIGIT_BIT)) | ((nanmath_word)_dp[ix]);
      
      /* 这里其实就是除法基本算法了，列竖式 */
      if (w >= v) {
        t = (nanmath_digit)(w / v);
        w -= ((nanmath_word)t) * ((nanmath_word)v);
      } else {
        t = 0;
      }
      tmpq[ix] = (nanmath_digit)t;
    }
    
    /* 最后是余数 */
    if (r) {
      *r = cast_f(nanmath_digit, w);
    }
    
    q.clamp();
    copy(q);
    return NM_OK;
  }
  
  /*
   * 这里用的算法就是正常的辗转相除法
   * a = q * n + r
   */
  int nanmath_int::div(nanmath_int &v, nanmath_int &r) {
    
    /* 除数为0 */
    if (v.iszero()) {
      return set_lasterr(NM_VAL, cast_f(char*, __FUNCTION__));
    }
    
    /* 如果被除数小于除数,则商为0,余数等于被除数 */
    if (cmp_mag(*this, v) == NM_LT) {
      
      if (r.testnull() == 0) {
        if (r.copy(*this) != NM_OK)
          return _lasterr;
      }
      
      /* 商为0 */
      zero();
      return NM_OK;
    }
    
    /* 这里就保证了 被除数 大于 除数 */
    
    /* 商 */
    nanmath_int q;
    if (q.allocs(_used + 2) != NM_OK) {
      return _lasterr;
    }
    q.set_used(_used + 2);
    
    /* x = 被除数, y = 除数 */
    nanmath_int x, y;
    if (x.copy(*this) != NM_OK) {
      return _lasterr;
    }
    
    if (y.copy(v) != NM_OK) {
      return _lasterr;
    }
    
    /* 修订符号 */
    int neg = (_sign == v.get_sign()) ? NM_ZPOS : NM_NEG;
    x.set_sign(NM_ZPOS);
    y.set_sign(NM_ZPOS);
    
    NANMATH_DBG("before normalize x y\n");
    NANMATH_DBG("x's used = %d, y's used = %d\n", x.get_used(), y.get_used());
    NANMATH_DBG("x = %s\n", x.result());
    NANMATH_DBG("y = %s\n", y.result());
    
    /* 标准化x,y, 确保 y >= v/2, [v == 2^DIGIT_BIT] */
    int norm = y.count_bits() % DIGIT_BIT;      /* 一个模上DIGIT_BIT循环系统的数值 */
    
    NANMATH_DBG("norm1 = %d, y's count_bits = %d\n", norm, y.count_bits());
    
    /* 保证这个值小于DIGIT_BIT的一半 
     * 如果除数的最高digit所占用的实际位数小于精度-1，换句话说就是
     * 最高digit的值小于精度全值的一半
     *
     * 让除数最高为补满一个digit
     */
    if (norm < (int)(DIGIT_BIT-1)) {
      norm = (DIGIT_BIT-1) - norm;              /* 余数其余的位 */
      NANMATH_DBG("norm2 = %d\n", norm);
      
      /* 被除数标准化 */
      if (x.lsh(norm) != NM_OK) {
        return _lasterr;
      }
      
      /* 除数标准化 */
      if (y.lsh(norm) != NM_OK) {
        return _lasterr;
      }
    } else {
      norm = 0;
    }
    
    NANMATH_DBG("after normalize x y\n");
    NANMATH_DBG("x's used = %d, y's used = %d\n", x.get_used(), y.get_used());
    NANMATH_DBG("x = %s\n", x.result());
    NANMATH_DBG("y = %s\n", y.result());
    
    int n = x.get_used() - 1;           /* 被除数最高位 */
    int t = y.get_used() - 1;           /* 除数最高位 */
    
    NANMATH_DBG("n = %d, t = %d, n - t = %d\n", n, t, n - t);
    
    /* 将 除数的位数 设定到 被除数 相等的位置 
     * 高位先做除法
     * / 56789
     *   345
     */
    if (y.lsh_d(n - t) != NM_OK) {
      return _lasterr;
    }

    NANMATH_DBG("after y.lsh_d (n - t) = %s\n", y.result());
    
    /* 计算x的n - t所指向的位与y的有多少个
     * 这里就是做正规的除法
     */
    while (x.cmp(y) != NM_LT) {
      /* 商的位置是在被除数与除数的位数相差的地方开始
       * 想想列竖式
       */
      ++(*q.getp(n - t));
      if (x.sub(y) != NM_OK) {
        return _lasterr;
      }
    }
    
    NANMATH_DBG("after 1th div x = %s, q = %s\n", x.result(), q.result());
    
    /* 经过以上的运算，就是计算被除数比除数大于的那个部分
     * y右移动 n - t 位 回到原先的位置
     * 这里 x 就是 竖式 经过 减法的余数
     */
    y.rsh_d(n - t);
    
    NANMATH_DBG("after y.rsh_d(n - t) = %s\n", y.result());
    
    /* 计算其余的部分
     * n 是 最初被除数 最高位
     * t 是 最初除数 最高位
     * t + 1
     */
    int i;
    nanmath_int t1, t2;
    for (i = n; i >= (t + 1); i--) {
      NANMATH_DBG("i = %d\n", i);
      /* 如果当前位大于被除数位数,直到于x当前的位数相等
       * x会不断的做减法
       */
      if (i > x.get_used()) {
        NANMATH_DBG("i > x's used => continue\n");
        continue;
      }
      
      if (x.getv(i) == y.getv(t)) {
        /* 如果当前经过减法过后的x与y的某位相等 
         * 商对应的位全满
         */
        *(q.getp(i - t - 1)) = ((((nanmath_digit)1) << DIGIT_BIT) - 1);
      } else {
        /* 单digit除法，扩展到字除法 */
        nanmath_word tmp;
        
//      NANMATH_DBG("x in tmp = %s, y in tmp = %s\n", x.result(), y.result());

        
        tmp = cast_f(nanmath_word, x.getv(i)) << cast_f(nanmath_word, DIGIT_BIT);
        tmp |= cast_f(nanmath_word, x.getv(i - 1));
        tmp /= cast_f(nanmath_word, y.getv(t));
        if (tmp > cast_f(nanmath_word, NM_MASK))
          tmp = NM_MASK;
        *(q.getp(i - t - 1)) = cast_f(nanmath_digit, (tmp & (nanmath_word) (NM_MASK)));
      }
      
//    NANMATH_DBG("now q1 = %s\n", q.result());
      
      /* 当前的digit的值 + 1,为了以下循环-1的一个逻辑规则而已 */
      *(q.getp(i - t - 1)) = (q.getv(i - t - 1) + 1) & NM_MASK;
      do {
        *(q.getp(i - t - 1)) = (q.getv(i - t - 1) - 1) & NM_MASK;
        
        /* 除数左边2位 
         * 如果除数不满2位，则把t1的最低digit设置为0
         * 然后，索引1digit设置为除数的0位
         */
        t1.zero();
        t1.set_used(2);
        *(t1.getp(0)) = (t - 1 < 0) ? 0 : y.getv(t - 1);
        *(t1.getp(1)) = y.getv(t);
        
        /* 这里计算t1 */
        if (t1.mul_d(q.getv(i - t - 1)) != NM_OK) {
          return _lasterr;
        }
        
        /* t2最多取当前被除数的高三位，如果不够则移动 */
        t2.set_used(3);
        *(t2.getp(0)) = (i - 2 < 0) ? 0 : x.getv(i - 2);
        *(t2.getp(1)) = (i - 1 < 0) ? 0 : x.getv(i - 1);
        *(t2.getp(2)) = x.getv(i);
      } while (cmp_mag(t1, t2) == NM_GT); /* while (t1 > t2) 
                                           * 保证被除数大于除数
                                           */
      
//    NANMATH_DBG("now q2 = %s\n", q.result());
      
      /* 到这里，被除数就小于余数了
       * 重新设置t1为原始被除数 然后 乘上 当前的商
       */
      if (t1.copy(y) != NM_OK) {
        return _lasterr;
      }
      
      NANMATH_DBG("q's dp[%d] = %d\n", i - t - 1, q.getv(i - t - 1));
      
      NANMATH_DBG("1) t1 = %s\n", t1.result());
      
      if (t1.mul_d(q.getv(i - t - 1)) != NM_OK) {
        return _lasterr;
      }
      
      NANMATH_DBG("2) t1 = %s\n", t1.result());

      
      /* 还原应该有的位 */
      if (t1.lsh_d(i - t - 1) != NM_OK) {
        return _lasterr;
      }
      
      NANMATH_DBG("3) t1 = %s\n", t1.result());

      NANMATH_DBG("x before sub = %s\n", x.result());
      
#ifdef DEBUG
      if (x.get_sign() == NM_ZPOS) {
        NANMATH_DBG("x = +\n");
      } else {
        NANMATH_DBG("x = -\n");
      }
      
      if (t1.get_sign() == NM_ZPOS) {
        NANMATH_DBG("t1 = +\n");
      } else {
        NANMATH_DBG("t1 = -\n");
      }
#endif
      
      NANMATH_DBG("x's used = %d\n", x.get_used());
      NANMATH_DBG("t1's used = %d\n", t1.get_used());
      
      /* 相减 */
      if (x.sub(t1) != NM_OK) {
        return _lasterr;
      }
      
      NANMATH_DBG("x after sub = %s\n", x.result());
      
      /* 如果当前被除数小于当前的商
       * 则还原以上减法操作
       */
      if (x.get_sign() == NM_NEG) {
        if (t1.copy(y) != NM_OK) {
          return _lasterr;
        }
        if (t1.lsh_d(i - t - 1) != NM_OK) {
          return _lasterr;
        }
        if (x.add(t1) != NM_OK) {
          return _lasterr;
        }
        
        /* 减去前面另外加上的1 */
        *(q.getp(i - t - 1)) = (q.getv(i - t - 1) - 1UL) & NM_MASK;
      }
    }/* end for */
    
    /* 现在x是余数，q是商 */
    
    /* 余数不为0，则按照原先被除数的 */
    if (x.iszero()) {
      x.set_sign(NM_ZPOS);
    } else {
      x.set_sign(_sign);
    }
    
    /* 复制符号 */
    q.clamp();
    copy(q);
    _sign = neg;
    
    /* 求余数 */
    if (r.testnull() == 0) {
      x.rsh(norm);
      r.copy(x);
    }
    return NM_OK;
  }
  
  int nanmath_int::div(nanmath_int &a, nanmath_int &b, nanmath_int& r) {
    if (copy(a) != NM_OK)
      return _lasterr;
    return div(b, r);
  }
}