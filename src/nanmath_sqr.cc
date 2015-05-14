#include "nanmath.h"

namespace nanmath {
  
  int nanmath_int::sqr() {
    int res;
    nanmath_int b;
    
    if (_used >= _karatsuba_sqr_threshold) {
      res = karatsuba_sqr(*this, b);
    } else if ((_used * 2 + 1) < NANMATH_WARRAY &&
               _used < (1 << (sizeof(nanmath_word) * CHAR_BIT - 2 * DIGIT_BIT - 1))) {
      res = s_sqr_fast(*this, b);
    } else {
      res = s_sqr(*this, b);
    }
    
    res = copy(b);
    _sign = NANMATH_ZPOS;

    return res;
  }
  
  int nanmath_int::sqr(nanmath_int &b) {
    int res;
    
    if ((res = paste(b)) != NANMATH_OK) {
      return res;
    }
    
    return b.sqr();
  }

  int nanmath_int::karatsuba_sqr(nanmath_int &a, nanmath_int &b) {
    nanmath_int x0, x1, t1, t2, x0x0, x1x1;
  
    int err = NANMATH_MEM;
  
    /* 最小的位数 */
    int B = a.get_used();
  
    /* 现在除以2 */
    B = B >> 1;
  
    /* 初始化所有的临时变量 */
    if ((err = x0.resize(B)) != NANMATH_OK)
      return err;
    
    if ((err = x1.resize(a.get_used() - B)) != NANMATH_OK)
      return err;
  
    if ((err = t1.resize(a.get_used() * 2)) != NANMATH_OK)
      return err;
    if ((err = t2.resize(a.get_used() * 2)) != NANMATH_OK)
      return err;
    if ((err = x0x0.resize(B * 2)) != NANMATH_OK)
      return err;
    if ((err = x1x1.resize((a.get_used() - B) * 2)) != NANMATH_OK)
      return err;
    
    nanmath_digit *src = cast(nanmath_digit, a.get_digit());
    nanmath_digit *dst = cast(nanmath_digit, x0.get_digit());
    int x;
    
    /* 一半移动到x0 */
    for (x = 0; x < B; x++)
      *dst++ = *src++;
    
    /* 一半移动到x1 */
    dst = cast(nanmath_digit, x1.get_digit());
    for (x = B; x < a.get_used(); x++)
      *dst++ = *src++;
  
    x0.set_used(B);
    x1.set_used(a.get_used() - B);
    x0.clamp();
  
    /* 现在分别计算x0*x0 与 x1*x1 */
    if ((err = x0.sqr(x0x0)) != NANMATH_OK)
      return err;           /* x0x0 = x0*x0 */
    if (x1.sqr(x1x1) != NANMATH_OK)
      return err;           /* x1x1 = x1*x1 */
  
    /* 现在计算 (x1+x0)**2 */
    if ((err = s_add(x1, x0, t1)) != NANMATH_OK)
      return err;           /* t1 = x1 + x0 */
    if (t1.sqr() != NANMATH_OK)
      return err;           /* t1 = (x1 + x0) * (x1 + x0) */
  
    /* 加上 x0y0 */
    if ((err = s_add(x0x0, x1x1, t2)) != NANMATH_OK)
      return err;           /* t2 = x0x0 + x1x1 */
    if ((err = s_sub(t1, t2, t1)) != NANMATH_OK)
      return err;           /* t1 = (x1 + x0)**2 - (x0x0 + x1x1) */
  
    /* 左移动B精度位 */
    if ((err = t1.lsh_d(B)) != NANMATH_OK)
      return err;           /* t1 = (x0x0 + x1x1 - (x1-x0)*(x1-x0))<<B */
    if ((err = x1x1.lsh_d(B * 2)) != NANMATH_OK)
      return err;           /* x1x1 = x1x1 << 2*B */
  
    if ((err = t1.add(x0x0)) != NANMATH_OK)
      return err;           /* t1 = x0x0 + t1 */
    if ((err = b.add(t1, x1x1)) != NANMATH_OK)
      return err;           /* t1 = x0x0 + t1 + x1x1 */
    
    return NANMATH_OK;
  }
  
  int nanmath_int::s_sqr_fast(nanmath_int &a, nanmath_int &b) {
    int res, iz;
    nanmath_digit W[NANMATH_WARRAY], *tmpx;
    
    /* 增长到目标所需要的 */
    int pa = a.get_used() + a.get_used();
    if (b.get_alloc() < pa) {
      if ((res = b.grow(pa)) != NANMATH_OK)
        return res;
    }
    
    /* 输出结果的位数 */
    nanmath_word W1 = 0;
    int ix = 0;
    for (ix = 0; ix < pa; ix++) {
      int tx, ty, iy;
      nanmath_word _W;
      nanmath_digit *tmpy;
      
      /* 清除计数 */
      _W = 0;
      
      /* 获取两个大数的偏移 */
      ty = MIN(a.get_used() - 1, ix);
      tx = ix - ty;
      
      /* 设置临时变量别名 */
      tmpx = a.getp(tx);
      tmpy = a.getp(ty);
      
      /* this is the number of times the loop will iterrate, essentially
       while (tx++ < a->used && ty-- >= 0) { ... }
       */
      iy = MIN(a.get_used()-tx, ty+1);
      
      /* now for squaring tx can never equal ty
       * we halve the distance since they approach at a rate of 2x
       * and we have to round because odd cases need to be executed
       */
      iy = MIN(iy, (ty-tx+1)>>1);
      
      /* 执行循环 */
      for (iz = 0; iz < iy; iz++) {
        _W += ((nanmath_word)*tmpx++)*((nanmath_word)*tmpy--);
      }
      
      /* 两倍自身 并且 加上进位 */
      _W = _W + _W + W1;
      
      /* even columns have the square term in them */
      if ((ix & 1) == 0) {
        _W += ((nanmath_word)a.getv(ix>>1)) * ((nanmath_word)a.getv(ix>>1));
      }
      
      /* 保存它 */
      W[ix] = (nanmath_digit)(_W & NANMATH_MASK);
      
      /* 生成下一个进位 */
      W1 = _W >> ((nanmath_word)DIGIT_BIT);
    }/* end for */
    
    /* 设置结构 */
    int olduse = b.get_used();
    b.set_used(a.get_used() * 2);
    
    nanmath_digit *tmpb = cast(nanmath_digit, b.get_digit());
    for (ix = 0; ix < pa; ix++)
      *tmpb++ = W[ix] & NANMATH_MASK;
    
    /* 清除没有用到的位 */
    for (; ix < olduse; ix++)
      *tmpb++ = 0;
    
    b.clamp();
    return NANMATH_OK;
  }
  
  int nanmath_int::s_sqr(nanmath_int &a, nanmath_int &b) {
    nanmath_int t;
    int res, iy;
    nanmath_word r;
    nanmath_digit u, tmpx, *tmpt;
    
    /* 要计算的值的位数 */
    int pa = a.get_used();
    
    /* 分配目标的内存 */
    if ((res = t.resize(2*pa + 1)) != NANMATH_OK)
      return res;
    
    /* 默认使用最大可能的位数 */
    t.set_used(2*pa + 1);
    
    /* 遍历源操作数的所有位 */
    for (int ix = 0; ix < pa; ix++) {
      /* 结果 */
      r = ((nanmath_word)t.getv(2 * ix)) +
          ((nanmath_word)a.getv(ix)) * ((nanmath_word)a.getv(ix));
      
      /* 保存结果的低位的部分 */
      t.setv(ix + ix, (nanmath_digit)(r & ((nanmath_word)NANMATH_MASK)));
      
      /* 获取进位 */
      u = (nanmath_digit)(r >> ((nanmath_word)DIGIT_BIT));
      
      /* 左值 = A[ix] * A[iy] */
      tmpx = a.getv(ix);
      
      /* 保存结果的临时值 */
      tmpt = t.getp(2 * ix + 1);
      
      /* 遍历 */
      for (iy = ix + 1; iy < pa; iy++) {
        /* 第一次计算结果 */
        r = ((nanmath_word)tmpx) * ((nanmath_word)a.getv(iy));
        
        /* 现在计算双精度的结果 */
        r = ((nanmath_word)*tmpt) + r + r + ((nanmath_word)u);
        
        /* 保存低位的值 */
        *tmpt++ = (nanmath_digit)(r & ((nanmath_word)NANMATH_MASK));
        
        /* 获取进位 */
        u = (nanmath_digit)(r >> ((nanmath_word) DIGIT_BIT));
      }
      
      while (u != ((nanmath_digit)0)) {
        r = ((nanmath_word)*tmpt) + ((nanmath_word)u);
        *tmpt++ = (nanmath_digit)(r & ((nanmath_word)NANMATH_MASK));
        u = (nanmath_digit)(r >> ((nanmath_word)DIGIT_BIT));
      }
    }/* end for */
    
    t.clamp();
    b.copy(t);

    return NANMATH_OK;
  }
  
}
