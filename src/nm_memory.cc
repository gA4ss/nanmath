#include "nanmath.h"
#include <stdlib.h>


namespace nanmath {
  void *nm_int::nm_calloc(size_t count, size_t size) {
    return calloc(count, size);
  }
  
  void nm_int::nm_free(void *ptr) {
    return free(ptr);
  }
  
  void *nm_int::nm_malloc(size_t size) {
    return malloc(size);
  }
  
  void *nm_int::nm_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
  }
}