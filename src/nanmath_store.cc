#include "nanmath.h"

namespace nanmath {
  int nanmath_int::to_big_byte_order(unsigned char *b) {
    int x, res;
    nanmath_int t;
    
    nanmath_assert(b);
    
    if ((res = t.copy(*this)) != NANMATH_OK) {
      return res;
    }
    
    x = 0;
    while (t.iszero() == NANMATH_NO) {
      b[x++] = (unsigned char) (t.getv(0) & 0xFF);
      if ((res = t.div_2x(8, nnull)) != NANMATH_OK) {
        return res;
      }
    }
    reverse_mem(b, x);
    return NANMATH_OK;
  }
  
  int nanmath_int::to_big_byte_order_n(unsigned char *b, int *outlen) {
    
    int len = 0;
    
    nanmath_assert(b);
    nanmath_assert(outlen);
    
    len = get_big_unsigned_size();
    if (*outlen < len) {
      return NANMATH_VAL;
    }
    
    *outlen = len;
    return to_big_byte_order(b);
  }
  
  int nanmath_int::to_big_signed_byte_order(unsigned char *b) {
    int res;
    
    if ((res = to_big_byte_order(b + 1)) != NANMATH_OK) {
      return res;
    }
    b[0] = cast_f(unsigned char, (_sign == NANMATH_ZPOS) ? 0 : 1);
    return NANMATH_OK;
  }
  
  int nanmath_int::to_big_signed_byte_order_n(unsigned char *b, int *outlen) {
    nanmath_assert(b);
    nanmath_assert(outlen);
    
    if (*outlen < get_big_signed_size()) {
      return NANMATH_VAL;
    }
    *outlen = get_big_signed_size();
    return to_big_signed_byte_order(b);
  }
  
  int nanmath_int::get_big_unsigned_size() {
    int size = count_bits();
    return (size / 8 + ((size & 7) != 0 ? 1 : 0));
  }
  
  int nanmath_int::get_big_signed_size() {
    return 1 + get_big_unsigned_size();
  }
}
