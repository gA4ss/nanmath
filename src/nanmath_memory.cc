#include "nanmath.h"
#include <stdlib.h>
#include <string.h>

namespace nanmath {
  void *nanmath_int::nm_calloc(size_t count, size_t size) {
    void *r = calloc(count, size);
    if (r != NULL) {
      memset(r, 0, size * count);
    }
    return r;
  }
  
  void nanmath_int::nm_free(void *ptr) {
    return free(ptr);
  }
  
  void *nanmath_int::nm_malloc(size_t size) {
    void *r = malloc(size);
    if (r != NULL) {
      memset(r, 0, size);
    }
    return r;
  }
  
  void *nanmath_int::nm_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
  }
}