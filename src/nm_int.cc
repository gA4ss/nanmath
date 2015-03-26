#include "nanmath.h"

namespace nanmath {

  /* 初始化 */
  void nm_int::init() {
    _dp = cast(nm_digit, nm_malloc(sizeof(nm_digit) * NM_PREC));
    if (_dp == NULL) {
      _lasterr = NM_MEM;
      return;
    }
    
    for (int i = 0; i < NM_PREC; i++) {
      _dp[i] = 0;
    }
    
    _used = 0;
    _alloc = NM_PREC;
    _sign = NM_ZPOS;
    _lasterr = NM_OK;
    
    /* 设定karatsuba算法的阀值 */
    _karatsuba_mul_threshold = 80;
    _karatsuba_sqr_threshold = 120;
  }
  
  nm_int::nm_int() {
    init();
    return;
  }
  
  nm_int::nm_int(nm_digit v) {
    init();
    set(v);
  }
  
  nm_int::nm_int(nm_int &v) {
    init();
    v.paste(*this);
  }
  
  nm_int::~nm_int() {
    clear();
  }
}