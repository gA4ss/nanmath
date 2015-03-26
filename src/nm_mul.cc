#include "nanmath.h"

namespace nanmath {
  /* 乘以2 */
  int nm_int::mul_2() {
    /* 为了防止溢出增加一位 */
    if (grow(_used + 1) != NM_OK) {
      return _lasterr;
    }
    
    nm_digit r, rr, *tmp;
    
    tmp = _dp;
    
    /* 进位 */
    r = 0;
    for (int x = 0; x < _used; x++) {
      /* 保存当前位的MSB，取进位值 */
      rr = (*tmp >> ((nm_digit)(DIGIT_BIT - 1))) & 0x01;
      
      /* 加上进位左移1位，由于上一条指令已经把最高位取出，所以这里
       * 的左移1位，如果产生进位也不影响上面。
       * 左移后在这位最低位设置上进位
       */
      *tmp = ((*tmp << ((nm_digit)1)) | r) & NM_MASK;
      tmp++;
      
      r = rr; /* 保存进位 */
    }
    
    /* 最后还存在进位 */
    if (r != 0) {
      /* 最后还存在进位，则直接设置目标当前最高位直接为1 */
      *tmp = 1;
      ++_used;
    }
    return NM_OK;
  }
  
  /* 乘以一个单精度位 */
  int nm_int::mul_d(nm_digit b) {
    /* 确定有足够空间存放 a * b */
    if (_alloc < _used + 1) {
      if (grow(_used + 1) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 当_used为0时做保证，尤其是与set_s配合
     * 如果多了，则最后末尾的clamp做缩减
     */
    _used++;
    
    nm_digit *tmp = _dp;
    nm_digit u = 0;    /* 进位 */
    nm_word r;  /* 运算结果 */
    int i = 0;
    for (i = 0; i < _used; i++) {
      /* 将单精度扩展到双精度相乘并加上进位值 */
      r = ((nm_word)u) + ((nm_word)*tmp) * ((nm_word)b);
      /* 获取进位 */
      u = (nm_digit)(r >> ((nm_word) DIGIT_BIT));
      /* 设置结果的低位部分 */
      *tmp++ = (nm_digit)(r & ((nm_word) NM_MASK));
    }
    
    /* 设置最后的进位  */
    *tmp++ = u;
    ++i;
    
    clamp();
    
    return NM_OK;
  }
    
  int nm_int::mul(nm_int &b) {
    int res = NM_OK;
    int neg = (_sign == b.get_sign()) ? NM_ZPOS : NM_NEG;
    
    /* 使用 Karatsuba算法 */
    if (MIN(_used, b.get_used()) >= _karatsuba_mul_threshold) {
      res = karatsuba_mul(b);
    } else {
      /* 使用快速倍乘算法
       * 
       * 当结果小于NM_WARRAY位并且digits的数量不影响进位时
       */
      int digs = _used + b.get_used() + 1;
      
      if ((digs < NM_WARRAY) &&
          MIN(_used, b.get_used()) <=
          (1 << ((CHAR_BIT * sizeof(nm_word)) - (2 * DIGIT_BIT)))) {
        res = s_mul_digs(*this, b, *this, digs);
      } else {}
        //res = s_mp_mul (a, b, c);
    }
    
    /* 符号 */
    _sign = (_used > 0) ? neg : NM_ZPOS;
    return res;
  }
  
  int nm_int::mul(nm_int &a, nm_int &b) {
    if (copy(a) != NM_OK)
      return _lasterr;
    return mul(b);
  }
  
  /******************************
   * 底层算法支持
   ******************************/
  /*
   * Karatsuba乘法是一种快速乘法。此算法在1960年由Anatolii Alexeevitch Karatsuba
   * 提出，并于1962年得以发表。
   * 此算法主要用于两个大数相乘。普通乘法的复杂度是n2，而Karatsuba算法的复杂度仅为
   * 3nlog3=3n1.585（log3是以2为底的）
   *
   * Karatsuba算法主要应用于两个大数的相乘，原理是将大数分成两段后变成较小的数位，然后做3次乘法，
   * 并附带少量的加法操作和移位操作。
   *
   * 现有两个大数，x，y。
   * 首先将x，y分别拆开成为两部分，可得x1，x0，y1，y0。他们的关系如下：
   * x = x1 * 10m + x0；
   * y = y1 * 10m + y0。其中m为正整数，m < n，且x0，y0 小于 10m。
   * 那么 xy = (x1 * 10m + x0)(y1 * 10m + y0) =z2 * 102m + z1 * 10m + z0，
   * 其中：
   * z2 = x1 * y1；
   * z1 = x1 * y0 + x0 * y1；
   * z0 = x0 * y0。
   * 此步骤共需4次乘法，但是由Karatsuba改进以后仅需要3次乘法。因为：
   * z1 = x1 * y0+ x0 * y1
   * z1 = (x1 + x0) * (y1 + y0) - x1 * y1 - x0 * y0，
   * 故x0 * y0 便可以由加减法得到。
   */
  int nm_int::karatsuba_mul(nm_int &b) {
    nm_int x0, x1, y0, y1, t1, x0y0, x1y1, c;
    int B, b_used;
    
    b_used = b.get_used();
    
    /* 最小的位数,保证a,b都能够 */
    B = MIN(_used, b_used);
    
    /* 除以2 */
    B = B >> 1;
    
    /* x0,y0保存B位，x1,y1保存其余位 */
    if (x0.allocs(B) != NM_OK)
      goto _err;
    if (x1.allocs(_used - B) != NM_OK)
      goto _err;
    if (y0.allocs(B) != NM_OK)
      goto _err;
    if (y1.allocs(b_used - B) != NM_OK)
      goto _err;
    
    /* 临时数 */
    if (t1.allocs(B * 2) != NM_OK)
      goto _err;
    if (x0y0.allocs(B * 2) != NM_OK)
      goto _err;
    if (x1y1.allocs(B * 2) != NM_OK)
      goto _err;
    
    /* 现在设置使用位，分配多大位，使用了多大位 */
    x0.set_used(B);
    y0.set_used(B);
    x1.set_used(_used - B);
    y1.set_used(b_used - B);
    
    int x;
    nm_digit *tmpa, *tmpb, *tmpx, *tmpy;
    
    tmpa = _dp;
    tmpb = cast(nm_digit,b.get_digit());
      
    tmpx = cast(nm_digit,x0.get_digit());
    tmpy = cast(nm_digit,y0.get_digit());
    
    /* 设定B位 */
    for (x = 0; x < B; x++) {
      *tmpx++ = *tmpa++;
      *tmpy++ = *tmpb++;
    }
    
    /* 设定a的剩余位 */
    tmpx = cast(nm_digit,x1.get_digit());
    for (x = B; x < _used; x++) {
      *tmpx++ = *tmpa++;
    }
    
    /* 设定b的剩余位 */
    tmpy = cast(nm_digit,y1.get_digit());
    for (x = B; x < b_used; x++) {
      *tmpy++ = *tmpb++;
    }
    
    /* 清除无效位 */
    x0.clamp();
    y0.clamp();
    
    /* 现在计算结果 x0y0 与 x1y1 */
    if (x0y0.mul(x0, y0) != NM_OK)
      goto _err;          /* x0y0 = x0*y0 */
    if (x1y1.mul(x1, y1) != NM_OK)
      goto _err;          /* x1y1 = x1*y1 */
    
    /* 计算 x1+x0 and y1+y0 */
    if (s_add(x1, x0, t1) != NM_OK)
      goto _err;          /* t1 = x1 + x0 */
    if (s_add(y1, y0, x0) != NM_OK)
      goto _err;          /* t2 = y1 + y0 */
    if (t1.mul(x0) != NM_OK)
      goto _err;          /* t1 = (x1 + x0) * (y1 + y0) */
    
    /* 加上 x0y0 */
    if (x0.add(x0y0, x1y1) != NM_OK)
      goto _err;          /* t2 = x0y0 + x1y1 */
    if (s_sub(t1, x0) != NM_OK)
      goto _err;          /* t1 = (x1+x0)*(y1+y0) - (x1y1 + x0y0) */
    
    /* 左移B位 */
    if (t1.lsh_d(B) != NM_OK)
      goto _err;          /* t1 = (x0y0 + x1y1 - (x1-x0)*(y1-y0))<<B */
    if (x1y1.lsh_d(B * 2) != NM_OK)
      goto _err;          /* x1y1 = x1y1 << 2*B */
    
    if (t1.add(x0y0) != NM_OK)
      goto _err;          /* t1 = x0y0 + t1 */
    if (c.add(t1, x1y1) != NM_OK)
      goto _err;          /* t1 = x0y0 + t1 + x1y1 */
    
    /* 复制结果 */
    copy(c);
    
  _err:
    return _lasterr;
  }
  
  /* 计算|a| * |b|并且只计算到digits位 */
  int nm_int::s_mul_digs(nm_int &a, nm_int &b, nm_int &c, int digs) {
    nm_int t;
    int pa, pb, ix, iy;
    nm_digit u;
    nm_word r;
    nm_digit tmpx, *tmpt, *tmpy, *tmpa;
    
    /* 选择更快速的算法 */
    if (((digs) < NM_WARRAY) &&
        MIN (a.get_used(), b.get_used()) <
        (1 << ((CHAR_BIT * sizeof(nm_word)) - (2 * DIGIT_BIT)))) {
      return s_mul_digs_(a, b, c, digs);
    }
    
    /* 临时结果位 */
    if (t.allocs(digs) != NM_OK) {
      return _lasterr;
    }
    t.set_used(digs);
    
    tmpa = cast(nm_digit, a.get_digit());
    pa = a.get_used();
    for (ix = 0; ix < pa; ix++) {
      u = 0;
      pb = MIN(b.get_used(), digs - ix);
      
      tmpx = cast_f(nm_digit, tmpa[ix]);
      tmpt = cast(nm_digit, t.get_digit()) + ix;
      tmpy = cast(nm_digit, b.get_digit());
      
      /* 循环相乘 */
      for (iy = 0; iy < pb; iy++) {
        r = ((nm_word)*tmpt) + ((nm_word)tmpx) * ((nm_word)*tmpy++) + ((nm_word)u);
        
        /* 取结果 */
        *tmpt++ = (nm_digit)(r & ((nm_word)NM_MASK));
        
        /* 取进位 */
        u = (nm_digit)(r >> ((nm_word)DIGIT_BIT));
      }
      
      /* 设置最后的进位 */
      if (ix + iy < digs) {
        *tmpt = u;
      }
    }
    
    t.clamp();
    t.exch(c);
    return NM_OK;
  }

  int nm_int::s_mul_high_digs(nm_int &a, nm_int &b, nm_int &c, int digs) {
    nm_int t;
    int pa, pb, ix, iy;
    nm_digit u;
    nm_word r;
    nm_digit tmpx, *tmpt, *tmpy, *tmpa;
    
    /* 如果位数小使用更快的算法 */
    if (((a.get_used() + b.get_used() + 1) < NM_WARRAY)
        && MIN (a.get_used(), b.get_used()) <
        (1 << ((CHAR_BIT * sizeof (nm_word)) - (2 * DIGIT_BIT)))) {
      return s_mul_high_digs_(a, b, c, digs);
    }
    
    if (t.allocs(a.get_used() + b.get_used() + 1) != NM_OK) {
      return _lasterr;
    }
    t.set_used(a.get_used() + b.get_used() + 1);
    
    tmpa = cast(nm_digit, a.get_digit());
    pa = a.get_used();
    pb = b.get_used();
    for (ix = 0; ix < pa; ix++) {
      u = 0;
      tmpx = tmpa[ix];
      tmpt = cast(nm_digit, t.get_digit()) + digs;
      tmpy = cast(nm_digit, b.get_digit()) + (digs - ix);
      
      for (iy = digs - ix; iy < pb; iy++) {
        r = ((nm_word)*tmpt) + ((nm_word)tmpx) *
        ((nm_word)*tmpy++) + ((nm_word) u);
        *tmpt++ = (nm_digit)(r & ((nm_word) NM_MASK));
        u = (nm_digit)(r >> ((nm_word) DIGIT_BIT));
      }
      *tmpt = u;
    }
    t.clamp();
    t.exch(c);
    return NM_OK;
  }
  
  /* Based on Algorithm 14.12 on pp.595 of HAC. */
  int nm_int::s_mul_digs_(nm_int &a, nm_int &b, nm_int &c, int digs) {
    int ix, iz;
    nm_digit W[NM_WARRAY];        /* 存放最终结果 */
    
    if (c.get_alloc() < digs) {
      if (c.grow(digs) != NM_OK) {
        return _lasterr;
      }
    }
    
    /* 结果的位数最大超不出a+b位数范围 
     * 这里的算法，就是最基本的错位相乘法
     * 不断的使用被乘数与乘数的某位相乘并进位相加
     * 算法速度为O(n^2)
     */
    int pa = MIN(digs, a.get_used() + b.get_used());
    
    /* 进行乘法运算 */
    nm_word _W = 0;   /* 临时项相乘结果 */
    for (ix = 0; ix < pa; ix++) {
      int tx, ty;
      int iy;
      nm_digit *tmpx, *tmpy;

      ty = MIN(b.get_used()-1, ix);
      tx = ix - ty;
      
      tmpx = cast(nm_digit, a.get_digit()) + tx;
      tmpy = cast(nm_digit, b.get_digit()) + ty;
      
      iy = MIN(a.get_used()-tx, ty+1);
      
      /* 循环相乘 */
      for (iz = 0; iz < iy; ++iz) {
        _W += cast_f(nm_word, *tmpx++) * cast_f(nm_word, *tmpy--);
      }
      
      /* 设定结果 */
      W[ix] = cast_f(nm_digit, _W) & NM_MASK;
      
     /* 产生进位 */
      _W = _W >> cast_f(nm_word, DIGIT_BIT);
    }
    
    /* 
     * 设定结果
     */
    
    int olduse = c.get_used();
    c.set_used(pa);
    
    nm_digit *tmpc = cast(nm_digit, c.get_digit());
    for (ix = 0; ix < pa+1; ix++) {
      *tmpc++ = W[ix];
    }
      
    /* 清除没有使用的位 */
    for (; ix < olduse; ix++) {
      *tmpc++ = 0;
    }
    
    c.clamp();
    return NM_OK;
  }

  /* 直接从digs位相乘 */
  int nm_int::s_mul_high_digs_(nm_int &a, nm_int &b, nm_int &c, int digs) {
    int ix, iz;
    nm_digit W[NM_WARRAY];
    
    int pa = a.get_used() + b.get_used();
    if (c.get_alloc() < pa) {
      if (c.grow(pa) != NM_OK) {
        return _lasterr;
      }
    }
    
    nm_word _W = 0;
    for (ix = digs; ix < pa; ix++) {
      int tx, ty, iy;
      nm_digit *tmpx, *tmpy;
      
      ty = MIN(b.get_used()-1, ix);
      tx = ix - ty;

      tmpx = cast(nm_digit, a.get_digit()) + tx;
      tmpy = cast(nm_digit, b.get_digit()) + ty;
      
      iy = MIN(a.get_used()-tx, ty+1);
      
      for (iz = 0; iz < iy; iz++) {
        _W += cast_f(nm_word, *tmpx++) * cast_f(nm_word, *tmpy--);
      }
      
      /* 设置结果 */
      W[ix] = cast_f(nm_digit, _W) & NM_MASK;
      
      /* 产生进位 */
      _W = _W >> cast_f(nm_word, DIGIT_BIT);
    }
    
    int olduse = c.get_used();
    c.set_used(pa);

    nm_digit *tmpc = cast(nm_digit, c.get_digit()) + digs;
    for (ix = digs; ix < pa; ix++) {
      *tmpc++ = W[ix];
    }
    
    for (; ix < olduse; ix++) {
      *tmpc++ = 0;
    }
    c.clamp();
    return NM_OK;
  }
}


























































