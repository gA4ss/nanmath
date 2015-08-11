#include "nanmath.h"

namespace nanmath {
  
  /* 清0 */
  void nanmath_int::zero(int s) {
    if (s) _sign = NANMATH_ZPOS;
    
    _used = 1;
    
    nanmath_digit *tmp = _dp;
    for (int n = 0; n < _alloc; n++) {
      *tmp++ = 0;
    }
  }
  
  int nanmath_int::iszero() {
    if ((_used == 0) ||
      ((_used == 1) && (_dp[0] == 0))) {
      return NANMATH_YES;
    }
    
    return NANMATH_NO;
  }
  
  int nanmath_int::iseven() {
    return ((_used > 0 && ((_dp[0] & 1) == 0)) ? NANMATH_YES : NANMATH_NO);
  }
  
  int nanmath_int::isodd() {
    return ((_used > 0 && ((_dp[0] & 1) == 1)) ? NANMATH_YES : NANMATH_NO);
  }
 
  int nanmath_int::abs() {
    _sign = NANMATH_ZPOS;
    return NANMATH_OK;
  }
  
  int nanmath_int::neg(nanmath_int &b) {
    if (cmp(b) == NANMATH_EQ) {
      return NANMATH_OK;
    }
    
    b.copy(*this);
    if (b.iszero() != NANMATH_OK) {
      b.set_sign((_sign == NANMATH_ZPOS) ? NANMATH_NEG : NANMATH_ZPOS);
    } else {
      b.set_sign(NANMATH_ZPOS);
    }
    
    return NANMATH_OK;
  }
  
  /* 如果余数可能是平方 - 快速排除非平方的数 */
  static const unsigned char rem_128[128] = {
    0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1
  };
  
  static const unsigned char rem_105[105] = {
    0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1,
    1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1,
    1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1
  };

  /* 如果参数v是一个平方项目则参数r返回非0，否则返回0 */
  int nanmath_int::is_square(nanmath_int &v, int &ret) {
    int res;
    nanmath_digit c;
    nanmath_int t;
    unsigned long r;
    
    /* 默认是非平方项 */
    ret = NANMATH_OK;
    
    if (v.get_sign() == NANMATH_NEG) {
      return NANMATH_VAL;
    }
    
    if (v.get_used() == 0) {
      return NANMATH_OK;
    }
    
    /* 首先检查模128 */
    if (rem_128[127 & v.getv(0)] == 1) {
      return NANMATH_OK;
    }
    
    /* 检查模105 (3*5*7) */
    if ((res = v.mod_d(105, &c)) != NANMATH_OK) {
      return res;
    }
    if (rem_105[c] == 1) {
      return NANMATH_OK;
    }
    
    t.set(11L*13L*17L*19L*23L*29L*31L);
    if ((res = v.mod(t)) != NANMATH_OK) {
      return res;
    }
    
    r = t.get_int();
    /* 检查其余的素数模 */
    if ( (1L<<(r%11)) & 0x5C4L )             return NANMATH_OK;
    if ( (1L<<(r%13)) & 0x9E4L )             return NANMATH_OK;
    if ( (1L<<(r%17)) & 0x5CE8L )            return NANMATH_OK;
    if ( (1L<<(r%19)) & 0x4F50CL )           return NANMATH_OK;
    if ( (1L<<(r%23)) & 0x7ACCA0L )          return NANMATH_OK;
    if ( (1L<<(r%29)) & 0xC2EDD0CL )         return NANMATH_OK;
    if ( (1L<<(r%31)) & 0x6DE2B848L )        return NANMATH_OK;
    
    /* 最终检查开方操作 */
    if ((res = t.copy(v)) != NANMATH_OK) {
      return res;
    }
    if ((res = v.sqrt()) != NANMATH_OK) {
      return res;
    }
    
    if ((res = t.sqr()) != NANMATH_OK) {
      return res;
    }
    
    ret = (t.cmp(v) == NANMATH_EQ) ? NANMATH_YES : NANMATH_NO;
    return res;
  }
  
}
