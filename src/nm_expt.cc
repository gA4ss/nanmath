#include "nanmath.h"

namespace nanmath {

#if 0
  int nm_int::expt_d(nm_digit b) {
    int res;
    nm_int g;
    
    if (g.copy(b) != NM_OK) {
      return _lasterr;
    }
    
    set(cast_f(nm_digit, 1));
    
    while (b > 0) {
      if (b & 1) {
        if (mul(g)) != NM_OK) {
          mp_clear (&g);
          return res;
        }
      }
      
      /* square */
      if (b > 1 && (res = mp_sqr (&g, &g)) != MP_OKAY) {
        mp_clear (&g);
        return res;
      }
      
      /* shift to next bit */
      b >>= 1;
    }
    
    mp_clear (&g);
    return MP_OKAY;
  }
#endif

#if 0
  int
  mp_2expt (mp_int * a, int b)
  {
    int     res;
    
    /* zero a as per default */
    mp_zero (a);
    
    /* grow a to accomodate the single bit */
    if ((res = mp_grow (a, b / DIGIT_BIT + 1)) != MP_OKAY) {
      return res;
    }
    
    /* set the used count of where the bit will go */
    a->used = b / DIGIT_BIT + 1;
    
    /* put the single bit in its place */
    a->dp[b / DIGIT_BIT] = ((mp_digit)1) << (b % DIGIT_BIT);
    
    return MP_OKAY;
  }
#endif
  
}