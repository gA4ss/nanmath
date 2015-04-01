#include "nanmath.h"
#include <string.h>

namespace nanmath {
  
//  /* 辅助函数 */
//  static nm_digit f_pow_d(nanmath_int &a, nm_digit d, int n) {
//    if ((n < 0) || (n > DIGIT_BIT)) {
//      a.set_lasterr(NM_VAL);
//      return 0;
//    }
//    
//    if (n == 0) {
//      return 1;
//    }
//    
//    nm_digit res = d;
//    while (n--) {
//      res *= res;
//    }
//    
//    return res;
//  }
  
  /* 进制基表 */
  static const char *s_rmap = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";
  
  nm_digit nanmath_int::getv(int index) {
    nm_digit *p = getp(index);
    if (p == NULL) {
      return (nm_digit)-1;
    }
    return cast_f(nm_digit, ((*p) & NM_MASK));
  }
  
  nm_digit *nanmath_int::getp(int index) {
    if ((_used <= 0) || (_alloc <= 0) || (_dp == NULL)) {
      return NULL;
    }
    
    if ((index < 0) || (index >= _used)) {
      return NULL;
    }
    
    return _dp + index;
  }
  
  char *nanmath_int::result(int radix) {
#if 0
    if ((_used <= 0) || (_alloc <= 0) || (_dp == NULL)) {
      return NULL;
    }
    
    /* 进制数最大64 */
    if (radix < 2 || radix > 64) {
      _lasterr = NM_VAL;
      return NULL;
    }
    
    /* 计算字符串缓存长度 */
    size_t size = _used * DIGIT_BIT + 1;    /* 最大也就这么大了 */
    char *rs = cast(char, nm_malloc(size));
    if (rs == NULL) {
      _lasterr = NM_MEM;
      return NULL;
    }
    
    /* 从最高位开始输出 */
    char *res = rs;
    char v[DIGIT_BIT];
    memset(v, 0, DIGIT_BIT);
    
    size = sizeof(nm_word) * (_used + 1);
    nm_word *nl = cast(nm_word, nm_malloc(size));
    if (nl == NULL) {
      nm_free(rs);
      _lasterr = NM_MEM;
      return NULL;
    }
    
    /* 先转换一下进制吧，保存的是2^DIGIT_BIT进制
     * 要进行转换
     */
    for (int i = _used - 1; i >= 0; i--) {
      nl[i] = cast_f(nm_word, (_dp[i] & NM_MASK) << (i * DIGIT_BIT));
    }
    
    for (int i = _used - 1; i >= 0; i--) {
      nm_word n = (_dp[i] & NM_MASK) << i;
      
      /* 按照基表进行转码 */
      int j = 0;
      nm_digit rem = 0;
      do {
        rem = n % cast_f(nm_digit, radix);
        n /= cast_f(nm_digit, radix);
        v[j++] = *(s_rmap + rem);
      } while (n);
      
      /* 设置结果 */
      j--;  /* 回退一个索引 */
      do {
        *res++ = v[j];
      } while (j--);
    }
    
    return rs;
#endif
    return NULL;
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
  
  void nanmath_int::set(nm_digit v) {
    zero();
    _dp[0] = v & NM_MASK;
    clamp();
  }
  
  int nanmath_int::set_s(const char *str) {
    return set_s(str, 10);
  }
  
  int nanmath_int::set_s(const char *str, int radix) {
    int y, neg;
    char ch;
    
    if (radix < 2 || radix > 64) {
      return NM_VAL;
    }
    
    /* 清0 */
    zero();
    
    /* 读取符号 */
    if (*str == '-') {
      ++str;
      neg = NM_NEG;
    } else {
      neg = NM_ZPOS;
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
        if (mul_d(cast_f(nm_digit, radix)) != NM_OK) {
          return _lasterr;
        }
        
        if (add_d(cast_f(nm_digit, y)) != NM_OK) {
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
    return NM_OK;
  }
  
  /* 增长数值内存到为size */
  int nanmath_int::grow (nm_size size) {
    if (_alloc < size) {
      /* 至少是精度的2倍 */
      size += (NM_PREC * 2) - (size % NM_PREC);
      nm_digit *tmp = cast(nm_digit, nm_realloc(_dp, sizeof(nm_digit) * size));
      if (tmp == NULL) {
        _lasterr = NM_MEM;
        return _lasterr;
      }
      
      _dp = tmp;
      int i = _alloc;
      
      _alloc = (int)size;
      for (; i < _alloc; i++) {
        _dp[i] = 0;
      }
    }
    return NM_OK;
  }

  /* 将d的信息设置到自身中 */
  int nanmath_int::copy(nanmath_int &d) {
    /* 目标缓存不够 */
    int d_used = d.get_used();
    if (_alloc < d_used) {
      if (grow(d_used) != NM_OK)
        return _lasterr;
    }
    
    nm_digit *tmpa = _dp;
    nm_digit *tmpd = cast(nm_digit,d.get_digit());
    
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
    
    return NM_OK;
  }
  
  /* 将自身信息设置到d中 */
  int nanmath_int::paste(nanmath_int &d) {
    /* 目标缓存不够 */
    if (d.get_alloc() < _used) {
     if (d.grow(_used) != NM_OK)
        return _lasterr;
    }
    
    nm_digit *tmpa, *tmpb;
    int n;
    tmpa = _dp;
    tmpb = cast(nm_digit,d.get_digit());

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
    
    return NM_OK;
  }
  
  int nanmath_int::allocs(int size) {
    size += (NM_PREC * 2) - (size % NM_PREC);
    _dp = cast(nm_digit, nm_malloc(sizeof(nm_digit) * size));
    if (_dp == NULL) {
      _lasterr = NM_MEM;
      return _lasterr;
    }
    
    _used = 0;
    _alloc = size;
    _sign = NM_ZPOS;
    
    for (int x = 0; x < size; x++) {
      _dp[x] = 0;
    }
    
    return NM_OK;
  }
  
  int nanmath_int::resize(int size) {
    size += (NM_PREC * 2) - (size % NM_PREC);
    _dp = cast(nm_digit, nm_realloc(_dp, sizeof(nm_digit) * size));
    if (_dp == NULL) {
      _lasterr = NM_MEM;
      return _lasterr;
    }

    _alloc = size;
    _sign = NM_ZPOS;
    
    for (int x = _used; x < size; x++) {
      _dp[x] = 0;
    }
    
    return NM_OK;
  }
  
  int nanmath_int::shrink() {
    nm_digit *tmp;
    
    if (_alloc != _used) {
      if ((tmp = cast(nm_digit, nm_realloc(_dp, sizeof(nm_digit) * _used))) == NULL) {
        return NM_MEM;
      }
      _dp = tmp;
      _alloc = _used;
    }
    return NM_OK;
  }
  
  int nanmath_int::exch(nanmath_int &b) {
    nanmath_int t;
    if (t.copy(*this) != NM_OK) return _lasterr;
    if (copy(b) != NM_OK) return _lasterr;
    if (b.copy(t) != NM_OK) return _lasterr;
    return NM_OK;
  }
  
  int nanmath_int::exch(nanmath_int &a, nanmath_int &b) {
    nanmath_int t;
    if (t.copy(a) != NM_OK) return _lasterr;
    if (a.copy(b) != NM_OK) return _lasterr;
    if (b.copy(t) != NM_OK) return _lasterr;
    return NM_OK;
  }
  
  void nanmath_int::clear() {
    if (_dp != NULL) {
      for (int i = 0; i < _used; i++) {
        _dp[i] = 0;
      }
      
      nm_free(_dp);
      
      _dp = NULL;
      _alloc = _used = 0;
      _sign = NM_ZPOS;
    }
  }
  
  int nanmath_int::testnull() {
    return (int)(this == &nnull);
  }
  
}
