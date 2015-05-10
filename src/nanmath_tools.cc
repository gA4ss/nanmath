#include "nanmath.h"
#include <string.h>

namespace nanmath {
  
//  /* 辅助函数 */
//  static nanmath_digit f_pow_d(nanmath_int &a, nanmath_digit d, int n) {
//    if ((n < 0) || (n > DIGIT_BIT)) {
//      a.set_lasterr(NANMATH_VAL);
//      return 0;
//    }
//    
//    if (n == 0) {
//      return 1;
//    }
//    
//    nanmath_digit res = d;
//    while (n--) {
//      res *= res;
//    }
//    
//    return res;
//  }
  
  /* 进制基表 */
  static const char *s_rmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";
  
  int nanmath_int::setv(int index, nanmath_digit v) {
    nanmath_digit *p = getp(index);
    if (p == NULL) {
      return set_lasterr(NANMATH_VAL);
    } else {
      *p = (v & NANMATH_MASK);
    }
    
    return NANMATH_OK;
  }
  
  nanmath_digit nanmath_int::getv(int index) {
    nanmath_digit *p = getp(index);
    if (p == NULL) {
      return (nanmath_digit)-1;
    }
    return cast_f(nanmath_digit, ((*p) & NANMATH_MASK));
  }
  
  nanmath_digit *nanmath_int::getp(int index) {
    if ((_used <= 0) || (_alloc <= 0) || (_dp == NULL)) {
      set_lasterr(NANMATH_MEM);
      return NULL;
    }
    
    if ((index < 0) || (index >= _used)) {
      set_lasterr(NANMATH_VAL);
      return NULL;
    }
    
    return _dp + index;
  }
  
  /* 输出结果,pstr指向的值，如果为空则分配内存，不为空直接释放空间
   */
  char *nanmath_int::result(int radix) {
    if (radix < 2 || radix > 64) {
      set_lasterr(NANMATH_VAL, cast_f(char*, __FUNCTION__));
      return NULL;
    }
    
    if ((_used == 0) || (_alloc == 0) || (_dp == NULL))  {
      return NULL;
    }
    
    /* 参数为不为空则分配内存 */
    if (_result != NULL) {
      nm_free(_result);
      _result = NULL;
    }
    
    char *r_str = cast(char, nm_malloc(_used * DIGIT_BIT + 1));
    if (r_str == NULL) {
      set_lasterr(NANMATH_MEM, cast_f(char*, __FUNCTION__));
      return NULL;
    }
    _result = r_str;
    
    if (iszero() == 1) {
      *r_str++ = '0';
      *r_str = '\0';
      return _result;
    }
    
    /* 生成临时变量 */
    nanmath_int tmp;
    paste(tmp);
    
    if (_sign == NANMATH_NEG) {
      *r_str++ = '-';
      tmp.set_sign(NANMATH_ZPOS);
    }
    
    int digs = 0;
    nanmath_digit r = 0;
    while (tmp.iszero() == 0) {
      if (tmp.div_d(cast_f(nanmath_digit, radix), &r) != NANMATH_OK) {
        return NULL;
      }
      *r_str++ = s_rmap[r];
      ++digs;
    }
    
    /* 翻转字符串 */
    char *rs = _result;
    if (_sign == NANMATH_NEG) {
      rs = _result + 1;  /* 跳过负号 */
    }
    reverse_mem(cast_f(unsigned char *, rs), digs);
    *r_str = '\0';
    return _result;
  }
  
  /* 清除无效位 */
  void nanmath_int::clamp() {
    while (_used > 0 && _dp[_used - 1] == 0) {
      --(_used);
    }

    if (_used == 0) {
      zero();
    }
  }
  
  void nanmath_int::spread() {
    if ((_dp == NULL) || (_alloc == 0)) {
      return;
    }
    
    _used = 0;
    for (int i = 0; i < _alloc; i++) {
      if (_dp[i] != 0)
        _used++;
    }
  }
  
  void nanmath_int::set(nanmath_digit v) {
    zero();
    _dp[0] = v & NANMATH_MASK;
    clamp();
  }
  
  int nanmath_int::set_s(const char *str, int radix) {
    int y, neg;
    char ch;
    
    if (radix < 2 || radix > 64) {
      return set_lasterr(NANMATH_VAL, cast_f(char*, __FUNCTION__));
    }
    
    /* 清0 */
    zero();
    
    /* 读取符号 */
    if (*str == '-') {
      ++str;
      neg = NANMATH_NEG;
    } else {
      neg = NANMATH_ZPOS;
    }
    
    /* 循环读取字符串 */
    while (*str) {
      ch = (char)((radix < 36) ? toupper((int) *str) : *str);   /* 基数小于36，数字字符是大写为主
                                                                 * 所以这里进行转换
                                                                 */
      /* 找寻进展系统的映射 */
      for (y = 0; y < 64; y++) {
        if (ch == s_rmap[y]) {
          break;
        }
      }
      
      /* y必须小于基数 */
      if (y < radix) {
        if (mul_d(cast_f(nanmath_digit, radix)) != NANMATH_OK) {
          return _lasterr;
        }
        
        if (add_d(cast_f(nanmath_digit, y)) != NANMATH_OK) {
          return _lasterr;
        }
      } else {
        /* 非法字符 */
        break;
      }
      ++str;
    }
    
    /* 设置标志位 */
    if (iszero() == 0) {
      _sign = neg;
    }

    return NANMATH_OK;
  }
  
  /* 增长数值内存到为size */
  int nanmath_int::grow (nanmath_size size) {
    if (_alloc < size) {
      /* 至少是精度的2倍 */
      size += (NANMATH_PREC * 2) - (size % NANMATH_PREC);
      nanmath_digit *tmp = cast(nanmath_digit, nm_realloc(_dp, sizeof(nanmath_digit) * size));
      if (tmp == NULL) {
        return set_lasterr(NANMATH_MEM, cast_f(char*, __FUNCTION__));
      }
      
      _dp = tmp;
      int i = _alloc;
      
      _alloc = (int)size;
      for (; i < _alloc; i++) {
        _dp[i] = 0;
      }
    }
    return NANMATH_OK;
  }

  /* 将d的信息设置到自身中 */
  int nanmath_int::copy(nanmath_int &d) {
    /* 目标缓存不够 */
    int d_used = d.get_used();
    if (_alloc < d_used) {
      if (grow(d_used) != NANMATH_OK)
        return _lasterr;
    }
    
    nanmath_digit *tmpa = _dp;
    nanmath_digit *tmpd = cast(nanmath_digit, d.get_digit());
    
    /* 复制 */
    int n;
    for (n = 0; n < d_used; n++) {
      *tmpa++ = *tmpd++;
    }
    
    /* 清除高位 */
    for (; n < _used; n++) {
      *tmpa++ = 0;
    }
    
    _used = d_used;
    _sign = d.get_sign();
    
    return NANMATH_OK;
  }
  
  /* 将自身信息设置到d中 */
  int nanmath_int::paste(nanmath_int &d) {
    /* 目标缓存不够 */
    if (d.get_alloc() < _used) {
     if (d.grow(_used) != NANMATH_OK)
        return _lasterr;
    }
    
    nanmath_digit *tmpa, *tmpb;
    int n;
    tmpa = _dp;
    tmpb = cast(nanmath_digit,d.get_digit());

    /* 复制 */
    for (n = 0; n < _used; n++) {
      *tmpb++ = *tmpa++;
    }
    
    /* 清除高位 */
    for (; n < d.get_used(); n++) {
      *tmpb++ = 0;
    }
    
    d.set_used(_used);
    d.set_sign(_sign);
    
    return NANMATH_OK;
  }
  
  int nanmath_int::allocs(int size) {
    size += (NANMATH_PREC * 2) - (size % NANMATH_PREC);
    _dp = cast(nanmath_digit, nm_malloc(sizeof(nanmath_digit) * size));
    if (_dp == NULL) {
      return set_lasterr(NANMATH_MEM, cast_f(char*, __FUNCTION__));
    }
    
    zero();
    
    return NANMATH_OK;
  }
  
  int nanmath_int::resize(int size) {
    size += (NANMATH_PREC * 2) - (size % NANMATH_PREC);
    _dp = cast(nanmath_digit, nm_realloc(_dp, sizeof(nanmath_digit) * size));
    if (_dp == NULL) {
      set_lasterr(NANMATH_MEM, cast_f(char*, __FUNCTION__));
      return _lasterr;
    }

    _alloc = size;
    _sign = NANMATH_ZPOS;
    
    for (int x = _used; x < size; x++) {
      _dp[x] = 0;
    }
    
    return NANMATH_OK;
  }
  
  int nanmath_int::shrink() {
    nanmath_digit *tmp;
    
    if (_alloc != _used) {
      if ((tmp = cast(nanmath_digit, nm_realloc(_dp, sizeof(nanmath_digit) * _used))) == NULL) {
        return NANMATH_MEM;
      }
      _dp = tmp;
      _alloc = _used;
    }

    return NANMATH_OK;
  }
  
  int nanmath_int::exch(nanmath_int &b) {
    nanmath_int t;
    if (t.copy(*this) != NANMATH_OK) return _lasterr;
    if (copy(b) != NANMATH_OK) return _lasterr;
    if (b.copy(t) != NANMATH_OK) return _lasterr;

    return NANMATH_OK;
  }
  
  int nanmath_int::exch(nanmath_int &a, nanmath_int &b) {
    nanmath_int t;
    if (t.copy(a) != NANMATH_OK) return _lasterr;
    if (a.copy(b) != NANMATH_OK) return _lasterr;
    if (b.copy(t) != NANMATH_OK) return _lasterr;

    return NANMATH_OK;
  }
  
  void nanmath_int::clear() {
    if (_dp != NULL) {
      for (int i = 0; i < _used; i++) {
        _dp[i] = 0;
      }
      
      nm_free(_dp);
      
      _dp = NULL;
      _alloc = _used = 0;
      _sign = NANMATH_ZPOS;
    }
    
    if (_result != NULL) {
      nm_free(_result);
    }
  }
  
  int nanmath_int::testnull() {
    return (int)(this == &nnull);
  }
  
  void nanmath_int::reverse_mem(unsigned char *s, int len) {
    unsigned char t;
    int ix = 0;
    int iy = len - 1;
    while (ix < iy) {
      t = s[ix];
      s[ix] = s[iy];
      s[iy] = t;
      ++ix;
      --iy;
    }
  }
  
}
