#include "nanmath.h"
#include <string.h>

namespace nanmath {

  /* 定义空值 */
  nanmath_int nnull;
  
  /* 初始化 */
  void nanmath_int::init() {
    _dp = cast(nm_digit, nm_malloc(sizeof(nm_digit) * NM_PREC));
    if (_dp == NULL) {
      set_lasterr(NM_MEM, cast_f(char*, __FUNCTION__));
      return;
    }
    
    for (int i = 0; i < NM_PREC; i++) {
      _dp[i] = 0;
    }
    
    _used = 1;
    _alloc = NM_PREC;
    _sign = NM_ZPOS;
    _lasterr = NM_OK;
    memset(_funcname, 0, MAX_BUFF_SIZE);
    
    /* 设定karatsuba算法的阀值 */
    _karatsuba_mul_threshold = 80;
    _karatsuba_sqr_threshold = 120;
  }
  
  nanmath_int::nanmath_int() {
    init();
    return;
  }
  
  nanmath_int::nanmath_int(nm_digit v) {
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