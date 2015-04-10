//
//  nanmath.h
//  nanmath
//
//  Created by logic.yan on 15/3/9.
//  Copyright (c) 2015年 naga. All rights reserved.
//

#ifndef nanmath_nanmath_h
#define nanmath_nanmath_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>

#ifdef DEBUG

#define NM_32BIT

#define NANMATH_DBG(fmt, a...)    printf("%s,%s(),%d:" fmt "/n", __FILE__,__FUNCTION__,__LINE__, ##a)

#else /* 只有在发布时，才支持64位 */

/* x86 64位支持 */
#if defined(__x86_64__)
#if !defined(NM_64BIT)
#define NM_64BIT
#endif
#endif/* end __x86_64__ */


#define NANMATH_DBG(fmt, a...)

#endif/* end DEBUG */

/* nan检查 */
#ifdef USE_NANMATH_CHECK
#define nanmath_assert(x)          assert(x)
#define nanmath_check(x, e)        (assert(x), e)
#else
#define nanmath_assert(x)
#define nanmath_check(x, e)        (e)
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

/* 类型强制转换 */
#define cast(t,x)  ((t*)(x))
#define cast_f(t,x)  ((t)(x))

/* 位类型的最大值 */
#define NM_DIGIT_BIT     DIGIT_BIT
#define NM_MASK          ((((nanmath_digit)1)<<((nanmath_digit)DIGIT_BIT))-((nanmath_digit)1))
#define NM_DIGIT_MAX     NM_MASK

#define NM_LT         -1
#define NM_EQ         0
#define NM_GT         1
    
#define NM_ZPOS       0
#define NM_NEG        1

/* 错误值定义 */
#define NM_OK         0
#define NM_MEM        -2
#define NM_VAL        -3
#define NM_RANGE      NM_VAL
    
#define NM_YES        1   /* YES */
#define NM_NO         0   /* NO */

/* 素数的类型 */
#define NM_PRIME_BBS      0x0001 /* BBS style prime */
#define NM_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define NM_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

/* 定义精度 */
#ifndef NM_PREC
#ifndef NM_LOW_MEM
#define NM_PREC                 32          /* 默认是32位 */
#else
#define NM_PREC                 8           /* 内存少时定义为8位 */
#endif
#endif

#define NM_WARRAY               (1 << (sizeof(nanmath_word) * CHAR_BIT - 2 * DIGIT_BIT + 1))

/* 最大的缓存区长度 */
#ifndef MAX_BUFF_SIZE
#define MAX_BUFF_SIZE 256
#endif

namespace nanmath {
  
  
  /* 一个'nanmath_digit'必须可以存储 DIGIT_BIT + 1位的长度
   * 一个'nanmath_word'必须可以存储 2*DIGIT_BIT + 1位的长度
   *
   * 一个'nanmath_digit'最少要保证可以存储7位
   */
  
#if !defined(NM_64BIT)
  /* 32位 */
#if defined(_MSC_VER) || defined(__BORLANDC__)
  typedef unsigned __int64    ulong64;
  typedef signed __int64      long64;
#else
  typedef unsigned long long  ulong64;
  typedef signed long long    long64;
#endif
  
  typedef unsigned long       nanmath_digit;
  typedef ulong64             nanmath_word;
#define DIGIT_BIT           28              /* 相当于一个2^28进制的数 */
#else
  /* 64位 */
  typedef unsigned long       nanmath_digit;
  typedef unsigned long       nanmath_word __attribute__ ((mode(TI)));   /* 128位 */
#define DIGIT_BIT           60              /* 相当于一个2^60进制的数 */
#endif
  
  typedef nanmath_word         nanmath_size;
  
  /* 一个空值 */
  class nanmath_int;
  extern nanmath_int nnull;
  
  typedef int nm_prime_callback(unsigned char *dst, int len, void *dat);
  
  /* 定义多精度整型 */
  class nanmath_int {
  public:
    /*
     * 初始化函数
     * nm_construct.cc
     */
    nanmath_int();
    nanmath_int(nanmath_digit v);
    nanmath_int(nanmath_int &v);
    virtual ~nanmath_int();
    
  protected:
    virtual void init();
    
  public:
    /* 
     * 获取当前多精度整数信息
     */
    virtual int get_used() { return _used; };
    virtual int get_alloc() { return _alloc; };
    virtual int get_sign() { return _sign; };
    virtual const nanmath_digit *get_digit() { return _dp; };
    
    /* FIXME: 不安全的操作 */
    virtual void set_used(int v) { _used = v; };
    virtual void set_alloc(int v) { _alloc = v; };
    virtual void set_sign(int v) { _sign = v; };
    virtual void set_digit(nanmath_digit *v) { _dp = v; };
    
  public:
    /*
     * 内存相关函数
     * nm_memory.cc
     */
    virtual void *nm_calloc(size_t count, size_t size);
    virtual void nm_free(void *ptr);
    virtual void *nm_malloc(size_t size);
    virtual void *nm_realloc(void *ptr, size_t size);
    
  public:
    /*  
     * 出错处理相关
     * nm_error.cc
     */
    const char * const error_to_string(int code);  /* 错误消息转换 */
    void rsle();
    int get_lasterr();
    char *get_lasterr_func();
    int set_lasterr(int err, char *fn);
    
  public:
    /* 
     * 外部功能接口
     * nm_tools.cc
     */
    virtual nanmath_digit getv(int index);                /* 获取索引对应的值,如果是-1则出错 */
    virtual nanmath_digit *getp(int index);               /* 获取索引对应的值的指针,如果是NULL则出错 */
    virtual char *result(int radix=10);                   /* 打印结果,由外部释放 */
    virtual void clamp();                                 /* 缩减无用位 */
    virtual void spread();                                /* 保证值对应位 */
    virtual void set(nanmath_digit v);                    /* 设置单精度位 */
    virtual int set_s(const char *str, int radix=10);     /* 按照radix给定的基,来设定数字 */
    virtual int copy(nanmath_int &d);                     /* 从d中拷贝到自身中 */
    virtual int paste(nanmath_int &d);                    /* 将自身的值黏贴到d中 */
    virtual int grow(nanmath_size size);                  /* 重新分配内存到size */
    virtual int allocs(int size);                         /* 按照尺寸分配内存 */
    virtual int resize(int size);                         /* 清除原有内存，并分配内存 */
    virtual int shrink();                                 /* 使用位与分配位相同 */
    virtual int exch(nanmath_int &b);                     /* 交换自身与b数值 */
    virtual int exch(nanmath_int &a, nanmath_int &b);     /* 交换a与b数值 */
    virtual void clear();                                 /* 清除内存 */
    virtual int testnull();                               /* 测试是否为空 */
    
  protected:
    void reverse_mem(unsigned char *s, int len);
  
  public:
    /*
     * 运算符重载接口
     */
      
  public:
    
    /*
     * 加法单元
     * nm_add.cc
     */
    virtual int add_d(nanmath_digit b);
    virtual int add(nanmath_int &b);
    virtual int add(nanmath_int &a, nanmath_int &b);
    
    /*
     * 减法单元
     * nm_sub.cc
     */
    virtual int sub_d(nanmath_digit b);
    virtual int sub(nanmath_int &b);
    virtual int sub(nanmath_int &a, nanmath_int &b);
    
    /*
     * 乘法单元
     * nm_mul.cc
     */
    virtual int mul_2();
    virtual int mul_d(nanmath_digit b);
    virtual int mul(nanmath_int &b);
    virtual int mul(nanmath_int &a, nanmath_int &b);
    
    /*
     * 除法单元
     * nm_div.cc
     */
    virtual int div_2();
    virtual int div_d(nanmath_digit v, nanmath_digit *r=NULL);
    virtual int div(nanmath_int &v, nanmath_int &r=nnull);
    virtual int div(nanmath_int &a, nanmath_int &b, nanmath_int& r);
    
    /*
     * 取模单元
     * nm_mod.cc
     */
    
    /*
     * 逻辑运算
     */
    virtual int lsh(nanmath_size b);
    virtual int rsh(nanmath_size b);
    virtual int lsh_d(nanmath_size b);
    virtual int rsh_d(nanmath_size b);
    virtual int count_bits();                 /* 计算总共用了多少位,最高位是多少 */
    
    /*
     * 指数运算
     */
    //virtual int expt_d(nanmath_digit b);
    
    /*
     * 对数运算
     */
    
    /*
     * 开方运算
     */
    
    /*
     * 比较运算
     * nm_cmp.cc
     */
    virtual int cmp(nanmath_int &b);
    virtual int cmp_d(nanmath_digit b);
    virtual int cmp_mag(nanmath_int &a, nanmath_int &b);
    
    /*
     * 数学辅助
     * nm_math.cc
     */
    virtual void zero();
    virtual int iszero();
    virtual int iseven();
    virtual int sodd();
    virtual int abs();
    
    /*
     * 一些支持运算的底层算法
     */
  protected:
    static int s_add(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    static int s_sub(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    
    static int karatsuba_mul(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    static int s_mul_digs(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    #define s_mul(a, b, c) s_mul_digs(a, b, c, (a).get_used() + (b).get_used() + 1)
    static int s_mul_high_digs(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    static int s_mul_digs_(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    static int s_mul_high_digs_(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    
    
    /*
     * 数据定义区域
     */
    
  protected:
    char _funcname[MAX_BUFF_SIZE];
    int _lasterr;      /* 最后一次错误 */
    char *_result;
    
  protected:
    /*
     * 快速乘法算法参数设定
     */
    int _karatsuba_mul_threshold;
    int _karatsuba_sqr_threshold;
  
#ifdef DEBUG
  public:
#else
  private:
#endif
    int _used;        /* 使用了多少位 */
    int _alloc;       /* 分配的总数量 */
    int _sign;        /* 标志位 */
    nanmath_digit *_dp;		/* 队列 */
  };
  
#if 0
  /* 实现操作符号重载 */
  class nm_int : public nanmath_int {
  public:
    nm_int();
    nm_int(nanmath_digit v);
    nm_int(nm_int &v);
    virtual ~nm_int();
  };
#endif
  
}

#endif
