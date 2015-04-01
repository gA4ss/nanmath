#include "nanmath.h"

namespace nanmath {
  /* 快速测试b是否是2的次方 */
  static int is_power_of_two(nm_digit b, int *p) {
    int x;
    
    /* 测试b是否为0,b是否是奇数 */
    if ((b==0) || (b & (b-1))) {
      return 0;
    }
    
    /* 循环匹配 */
    for (x = 0; x < DIGIT_BIT; x++) {
      if (b == (((nm_digit)1)<<x)) {
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
    
    nm_digit *tmpa = cast(nm_digit, _dp + _used - 1);
    nm_digit r = 0, rr = 0;
    
    for (int x = _used - 1; x >= 0; x--) {
      rr = *tmpa & 1;
      *tmpa = (*tmpa >> 1) | (r << (DIGIT_BIT - 1));
      tmpa--;
      r = rr;
    }
    
    clamp();
    return NM_OK;
  }
  
  int nanmath_int::div_d(nm_digit v, nanmath_int &r) {
    /* 除数不能为0 */
    if (v == 0) {
      return NM_VAL;
    }
    
    /* 除数是1，被除数是0 */
    if (v == 1 || iszero()) {
      if (r.testnull() == 0) r.zero();
      return NM_OK;
    }
    
    int ix;
    /* v是2的次方,相当于左移动n位 */
    if (is_power_of_two(v, &ix) == 1) {
      if (r.testnull() == 0) {
        /* 求余数,掩码不为0的部分 */
        nm_digit rem = _dp[0] & ((((nm_digit)1)<<ix) - 1);
        r.set(rem);
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
    nm_word w = 0;
    nm_digit t;
    nm_digit *tmpq = cast(nm_digit, q.get_digit());
    for (ix = _used - 1; ix >= 0; ix--) {
      /* 每次使用一个字的大小来做被除数 */
      w = (w << ((nm_word)DIGIT_BIT)) | ((nm_word)_dp[ix]);
      
      /* 这里其实就是除法基本算法了，列竖式 */
      if (w >= v) {
        t = (nm_digit)(w / v);
        w -= ((nm_word)t) * ((nm_word)v);
      } else {
        t = 0;
      }
      tmpq[ix] = (nm_digit)t;
    }
    
    /* 最后是余数 */
    if (r.testnull() == 0) {
      r.set((nm_digit)w);
    }
    
    q.clamp();
    copy(q);
    
    return NM_OK;
  }
  
  int nanmath_int::div_d(nm_digit v) {
    nanmath_int r;
    return div_d(v, r);
  }
  
  /*
   * 这里用的算法就是正常的辗转相除法
   * a = q * n + r
   */
  int nanmath_int::div(nanmath_int &v, nanmath_int &r) {
    /* 除数为0 */
    if (v.iszero()) {
      return NM_VAL;
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
    
    /* 标准化x,y, 确保 y >= v/2, [v == 2^DIGIT_BIT] */
    int norm = y.count_bits() % DIGIT_BIT;      /* 一个模上DIGIT_BIT循环系统的数值 */
    /* 保证这个值小于DIGIT_BIT的一半 */
    if (norm < (int)(DIGIT_BIT-1)) {
      norm = (DIGIT_BIT-1) - norm;              /* 余数其余的位 */
      
      /* 被除数标准化 */
      if (x.mul_d(norm) != NM_OK) {
        return _lasterr;
      }
      
      /* 除数标准化 */
      if (y.mul_d(norm) != NM_OK) {
        return _lasterr;
      }
    } else {
      norm = 0;
    }
    
    int n = x.get_used() - 1;           /* 被除数最高位 */
    int t = y.get_used() - 1;           /* 除数最高位 */
    
    /* 将 除数的位数 设定到 被除数 相等的位置 */
    if (y.lsh_d(n - t) != NM_OK) {
      return _lasterr;
    }
    
    /* 计算x的n - t所指向的位与y的有多少个
     * 这里就是做正规的除法
     */
    while (x.cmp(y) != NM_LT) {
      ++(*q.getp(n - t));
      if (x.sub(y) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 经过以上的运算，就是计算被除数比除数大于的那个部分
     * y右移动 n - t 位 回到原先的位置
     * 这里 x 就是 竖式 经过 减法的余数
     */
    y.rsh_d(n - t);
    
    /* 计算其余的部分
     * n 是 最初被除数 最高位
     * t 是 最初除数 最高位
     * t + 1
     */
    int i;
    nanmath_int t1, t2;
    for (i = n; i >= (t + 1); i--) {
      
      /* 如果当前位大于被除数位数,直到于x当前的位数相等
       * x会不断的做减法
       */
      if (i > x.get_used()) {
        continue;
      }
      
      if (x.getv(i) == y.getv(i)) {
        /* 如果当前经过减法过后的x与y的某位相等 
         * 商对应的位全满
         */
        *(q.getp(i - t - 1)) = ((((nm_digit)1) << DIGIT_BIT) - 1);
      } else {
        /* 单digit除法，扩展到字除法 */
        nm_word tmp;
        tmp = cast_f(nm_word, x.getv(i)) << cast_f(nm_word, DIGIT_BIT);
        tmp |= cast_f(nm_word, x.getv(i - 1));
        tmp /= cast_f(nm_word, y.getv(t));
        if (tmp > cast_f(nm_word, NM_MASK))
          tmp = NM_MASK;
        (*q.getp(i - t - 1)) = cast_f(nm_digit, (tmp & (nm_word) (NM_MASK)));
      }
      
      /* 当前的digit的值 + 1,为了以下循环-1的一个逻辑规则而已 */
      (*q.getp(i - t - 1)) = (q.getv(i - t - 1) + 1) & NM_MASK;
      do {
        (*q.getp(i - t - 1)) = (q.getv(i - t - 1) - 1) & NM_MASK;
        
        /* 除数左边2位 
         * 如果除数不满2位，则把t1的最低digit设置为0
         * 然后，索引1digit设置为除数的0位
         */
        t1.zero();
        (*t1.getp(0)) = (t - 1 < 0) ? 0 : y.getv(t - 1);
        (*t1.getp(1)) = y.getv(t);
        t1.set_used(2);
        
        /* 这里计算t1 */
        if (t1.mul_d(q.getv(i - t - 1)) != NM_OK) {
          return _lasterr;
        }
        
        /* t2最多取当前被除数的高三位，如果不够则移动 */
        (*t2.getp(0)) = (i - 2 < 0) ? 0 : x.getv(i - 2);
        (*t2.getp(1)) = (i - 1 < 0) ? 0 : x.getv(i - 1);
        (*t2.getp(2)) = x.getv(i);
        t2.set_used(3);
      } while (cmp_mag(t1, t2) == NM_GT); /* while (t1 > t2) 
                                           * 保证被除数大于除数
                                           */
      
      /* 到这里，被除数就小于余数了
       * 重新设置t1为原始被除数 然后 乘上 当前的商
       */
      if (t1.copy(y) != NM_OK) {
        return _lasterr;
      }
      if (t1.mul_d(q.getv(i - t - 1)) != NM_OK) {
        return _lasterr;
      }
      
      /* 还原应该有的位 */
      if (t1.lsh_d(i - t - 1) != NM_OK) {
        return _lasterr;
      }
      
      /* 相减 */
      if (x.sub(t1) != NM_OK) {
        return _lasterr;
      }
      
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
        (*q.getp(i - t - 1)) = (q.getv(i - t - 1) - 1UL) & NM_MASK;
      }
    }/* end for */
    
    /* 现在x是余数，q是商 */
    
    /* 余数不为0，则按照原先被除数的 */
    x.set_sign(x.get_used() == 0 ? NM_ZPOS : _sign);
    
    /* 复制符号 */
    q.clamp();
    copy(q);
    _sign = neg;
    
    /* 求余数 */
    if (r.testnull() == 0) {
      x.div_d(norm);
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