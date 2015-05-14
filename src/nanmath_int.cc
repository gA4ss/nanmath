#include "nanmath.h"
#include <string.h>

namespace nanmath {

  /* 定义空值 */
  nanmath_int nnull;
  
  /* 初始化 */
  void nanmath_int::init() {
    _dp = cast(nanmath_digit, nm_malloc(sizeof(nanmath_digit) * NANMATH_PREC));
    if (_dp == NULL) {
      /* 这里抛出异常 */
      return;
    }
    
    for (int i = 0; i < NANMATH_PREC; i++) {
      _dp[i] = 0;
    }
    
    _used = 1;
    _alloc = NANMATH_PREC;
    _sign = NANMATH_ZPOS;
    _result = NULL;
    
    /* 设定karatsuba算法的阀值 */
    _karatsuba_mul_threshold = 80;
    _karatsuba_sqr_threshold = 120;
  }
  
  nanmath_int::nanmath_int() {
    init();
    return;
  }
  
  nanmath_int::nanmath_int(nanmath_digit v) {
    init();
    set(v);
  }
  
  nanmath_int::nanmath_int(nanmath_int &v) {
    init();
    v.paste(*this);
  }
  
  nanmath_int::~nanmath_int() {
    clear();
  }
}