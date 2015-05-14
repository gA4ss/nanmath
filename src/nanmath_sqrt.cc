#include "nanmath.h"

namespace nanmath {

  int nanmath_int::sqrt() {
    int res;
    nanmath_int t1,t2;
  
    /* 必须是正数 */
    if (_sign == NANMATH_NEG) {
      return NANMATH_VAL;
    }
  
    /* 非零 */
    if (iszero() == NANMATH_YES) {
      return NANMATH_OK;
    }
  
    /* t1是要分解的值 */
    if ((res = t1.copy(*this)) != NANMATH_OK) {
      goto _end;
    }
    
    /* 右移1半 */
    if ((res = t1.rsh_d(t1.get_used() / 2) != NANMATH_OK)) {
      goto _end;
    }
  
    /* t1 > 0  */
    if ((res = t2.div(*this, t1, nnull)) != NANMATH_OK) {
      goto _end;
    }
    
    if ((res = t1.add(t2)) != NANMATH_OK) {
      goto _end;
    }
    
    if ((res = t1.div_2()) != NANMATH_OK) {
      goto _end;
    }
    
    do {
      if ((res = t2.div(*this, t1, nnull)) != NANMATH_OK) {
        goto _end;
      }
      
      if ((res = t1.add(t2)) != NANMATH_OK) {
        goto _end;
      }
      
      if ((res = t1.div_2()) != NANMATH_OK) {
        goto _end;
      }
    } while (s_cmp_mag(t1, t2) == NANMATH_GT);
  
    res = copy(t1);
    
  _end:
    return res;
  }
}