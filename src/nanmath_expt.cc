#include "nanmath.h"

namespace nanmath {

  int nanmath_int::expt_d(nanmath_digit b) {
    int res;
    nanmath_int c;
    
    /* 初始化结果 */
    c.set(cast_f(nanmath_digit, 1));
    
    while (b > 0) {
      
      /* 如果被置位 */
      if (b & 1) {
        if (c.mul(*this) != NANMATH_OK) {
          return c.get_lasterr();
        }
      }
      
      /* 平方 */
      if (b > 1 && ((res = c.sqr()) != NANMATH_OK)) {
        return res;
      }
      
      /* 右移到下一位 */
      b >>= 1;
    }
    
    /* 复制结果 */
    c.clamp();
    res = copy(c);
    
    return res;
  }

  int nanmath_int::bin_expt(int b) {
    int res;
    
    /* a清0 */
    zero();

    if ((res = grow(b / DIGIT_BIT + 1)) != NANMATH_OK) {
      return res;
    }
    
    /* 设置a使用的位数 */
    _used = b / DIGIT_BIT + 1;
    _dp[b / DIGIT_BIT] = ((nanmath_digit)1) << (b % DIGIT_BIT);
    
    return NANMATH_OK;
  }
  
}