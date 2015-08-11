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

#define NANMATH_32BIT

#define NANMATH_DBG(fmt, a...)    printf("%s,%s(),%d:" fmt "/n", __FILE__,__FUNCTION__,__LINE__, ##a)

#else /* 只有在发布时，才支持64位 */

/* x86 64位支持 */
#if defined(__x86_64__)
#if !defined(NANMATH_64BIT)
#define NANMATH_64BIT
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
#define NANMATH_DIGIT_BIT     DIGIT_BIT
#define NANMATH_MASK          ((((nanmath_digit)1)<<((nanmath_digit)DIGIT_BIT))-((nanmath_digit)1))
#define NANMATH_DIGIT_MAX     NANMATH_MASK

#define NANMATH_LT         -1
#define NANMATH_EQ         0
#define NANMATH_GT         1
    
#define NANMATH_ZPOS       0
#define NANMATH_NEG        1

/* 错误值定义 */
#define NANMATH_OK         0
#define NANMATH_MEM        -2
#define NANMATH_VAL        -3
#define NANMATH_RANGE      NANMATH_VAL
    
#define NANMATH_YES        1   /* YES */
#define NANMATH_NO         0   /* NO */

/* 素数的类型 */
#define NANMATH_PRIME_BBS      0x0001 /* BBS style prime */
#define NANMATH_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define NANMATH_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

/* 定义精度 */
#ifndef NANMATH_PREC
#ifndef NANMATH_LOW_MEM
#define NANMATH_PREC                 32          /* 默认是32位 */
#else
#define NANMATH_PREC                 8           /* 内存少时定义为8位 */
#endif
#endif

#define NANMATH_WARRAY               (1 << (sizeof(nanmath_word) * CHAR_BIT - 2 * DIGIT_BIT + 1))

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
  
#if !defined(NANMATH_64BIT)
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
#define DIGIT_BIT             28              /* 相当于一个2^28进制的数 */
#else
  /* 64位 */
  typedef unsigned long       nanmath_digit;
  typedef unsigned long       nanmath_word __attribute__ ((mode(TI)));   /* 128位 */
#define DIGIT_BIT             60              /* 相当于一个2^60进制的数 */
#endif
  
  typedef nanmath_word        nanmath_size;
  
  /* 一个空值 */
  class nanmath_int;
  extern nanmath_int nnull;
  
  typedef int nm_prime_callback(unsigned char *dst, int len, void *dat);
  
  /* 定义多精度整型 */
  class nanmath_int {
  public:
    /*
     * 初始化函数
     * nanmath_int.cc
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
     * nanmath_memory.cc
     */
    virtual void *nm_calloc(size_t count, size_t size);
    virtual void nm_free(void *ptr);
    virtual void *nm_malloc(size_t size);
    virtual void *nm_realloc(void *ptr, size_t size);
    
  public:
    /*  
     * 出错处理相关
     * nanmath_error.cc
     */
    virtual const char * const error_to_string(int code);  /* 错误消息转换 */
    
  public:
    /* 
     * 外部功能接口
     * nanmath_tools.cc
     */
    virtual int setv(int index, nanmath_digit v);         /* 设置索引对应的值 */
    virtual nanmath_digit getv(int index, int *ec=NULL);  /* 获取索引对应的值,如果是-1则出错 */
    virtual nanmath_digit *getp(int index, int *ec=NULL); /* 获取索引对应的值的指针,如果是NULL则出错 */
    virtual char *result(int radix=10, int *ec=NULL);     /* 打印结果,由外部释放 */
    virtual void clamp();                                 /* 缩减无用位 */
    virtual void spread();                                /* 保证值对应位 */
    virtual void set(nanmath_digit v);                    /* 设置单精度位 */
    virtual int set_s(const char *str, int radix=10);     /* 按照radix给定的基,来设定数字 */
    virtual int set_b(const unsigned char *b, int c);     /* 读取bin中的数字 */
    virtual int set_sb(const unsigned char *b, int c);    /* 读取bin中的数字,带符号 */
    virtual int copy(nanmath_int &d);                     /* 从d中拷贝到自身中 */
    virtual int paste(nanmath_int &d);                    /* 将自身的值黏贴到d中 */
    virtual int grow(nanmath_size size);                  /* 重新分配内存到size */
    virtual int allocs(int size);                         /* 按照尺寸分配内存 */
    virtual int resize(int size);                         /* 清除原有内存，并分配内存 */
    virtual int shrink();                                 /* 使用位与分配位相同 */
    virtual int exch(nanmath_int &b);                     /* 交换自身与b数值 */
    virtual void clear();                                 /* 清除内存 */
    virtual int testnull();                               /* 测试是否为空 */
    virtual unsigned long get_int();                      /* 获取nanmath_int的低32位整数 */
    virtual int rand(nanmath_int v, int digits);          /* 获取一个digits个位的随机数 */
    
    
    /*
     * 字节序保存
     */
    virtual int to_big_byte_order(unsigned char *b);
    virtual int to_big_byte_order_n(unsigned char *b, int *outlen);
    virtual int to_big_signed_byte_order(unsigned char *b);
    virtual int to_big_signed_byte_order_n(unsigned char *b, int *outlen);
    virtual int get_big_unsigned_size();
    virtual int get_big_signed_size();
    
  protected:
    void reverse_mem(unsigned char *s, int len);
  
  public:
    /*
     * 运算符重载接口
     */
      
  public:
    
    /*
     * 加法单元
     * nanmath_add.cc
     */
    virtual int add_d(nanmath_digit b);
    virtual int add(nanmath_int &b);
    virtual int add(nanmath_int &a, nanmath_int &b);
    
    /*
     * 减法单元
     * nanmath_sub.cc
     */
    virtual int sub_d(nanmath_digit b);
    virtual int sub(nanmath_int &b);
    virtual int sub(nanmath_int &a, nanmath_int &b);
    
    /*
     * 乘法单元
     * nanmath_mul.cc
     */
    virtual int mul_2();
    virtual int mul_2x(nanmath_digit b);
    virtual int mul_d(nanmath_digit b);
    virtual int mul(nanmath_int &b);
    virtual int mul(nanmath_int &a, nanmath_int &b);
    
    /*
     * 除法单元
     * nanmath_div.cc
     */
    virtual int div_2();
    virtual int div_2x(nanmath_digit b, nanmath_int &r=nnull);
    virtual int div_d(nanmath_digit v, nanmath_digit *r=NULL);
    virtual int div(nanmath_int &v, nanmath_int &r=nnull);
    virtual int div(nanmath_int &a, nanmath_int &b, nanmath_int &r=nnull);
    
    /*
     * 取模单元
     * nanmath_mod.cc
     */
    virtual int mod_2x(nanmath_digit v);
    virtual int mod_d(nanmath_digit v, nanmath_digit *r=NULL);
    virtual int mod(nanmath_int &v);
    virtual int mod(nanmath_int &a, nanmath_int &b);
    virtual int addmod(nanmath_int &b, nanmath_int &c);
    virtual int addmod(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    virtual int submod(nanmath_int &b, nanmath_int &c);
    virtual int submod(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    virtual int sqrmod(nanmath_int &b);
    virtual int sqrmod(nanmath_int &a, nanmath_int &b);
    virtual int mulmod(nanmath_int &b, nanmath_int &c);
    virtual int mulmod(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    virtual int invmod(nanmath_int &b);
    virtual int invmod(nanmath_int &a, nanmath_int &b);
    virtual int exptmod(nanmath_int &b, nanmath_int &c);
    virtual int exptmod(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    
    /*
     * 分解单元
     * nanmath_reduce.cc
     */
    int reduce(nanmath_int &m, nanmath_int &mu);
    int reduce_2k(nanmath_int &n, nanmath_digit d);
    int reduce_2k_l(nanmath_int &n, nanmath_int &d);
    
    /*
     * 逻辑运算
     */
    virtual int lsh(nanmath_size b);
    virtual int rsh(nanmath_size b);
    virtual int lsh_d(nanmath_size b);
    virtual int rsh_d(nanmath_size b);
    virtual int count_bits();                 /* 计算总共用了多少位,最高位是多少 */
    virtual int count_lsb();
    virtual int and_d(nanmath_digit d);
    virtual int and_v(nanmath_int &b);
    virtual int or_d(nanmath_digit d);
    virtual int or_v(nanmath_int &b);
    virtual int xor_d(nanmath_digit d);
    virtual int xor_v(nanmath_int &b);
    
    /*
     * 指数运算
     */
    virtual int expt_d(nanmath_digit b);
    virtual int bin_expt(int b);
    
    /*
     * 对数运算
     */
    
    /*
     * 开方运算
     * nanmath_sqrt.cc
     */
    virtual int sqrt();
    
    /*
     * 平方运算
     * nanmath_sqr.cc
     */
    virtual int sqr();
    virtual int sqr(nanmath_int &b);
    
    /*
     * 比较运算
     * nanmath_cmp.cc
     */
    virtual int cmp(nanmath_int &b);
    virtual int cmp_d(nanmath_digit b);
    
    /*
     * 数学辅助
     * nanmath_math.cc
     */
    virtual void zero(int s=0);
    virtual int iszero();
    virtual int iseven();
    virtual int isodd();
    virtual int abs();
    virtual int neg(nanmath_int &b);
    virtual int is_square(nanmath_int &v, int &ret);
    
    
    /*
     * 一些支持运算的底层算法
     */
  public:
    static int s_exch(nanmath_int &a, nanmath_int &b);
    
    static int s_cmp_mag(nanmath_int &a, nanmath_int &b);
    
    static int s_add(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    static int s_sub(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    
    static int karatsuba_mul(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    static int s_mul_digs(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    static int s_mul(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    static int s_mul_high_digs(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    static int s_mul_digs_(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    static int s_mul_high_digs_(nanmath_int &a, nanmath_int &b, nanmath_int &c, int digs);
    
    static int karatsuba_sqr(nanmath_int &a, nanmath_int &b);
    static int s_sqr_fast(nanmath_int &a, nanmath_int &b);
    static int s_sqr(nanmath_int &a, nanmath_int &b);
    
    static int s_invmod_fast(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    static int s_invmod_slow(nanmath_int &a, nanmath_int &b, nanmath_int &c);
    
    static int s_exptmod(nanmath_int &G, nanmath_int &X, nanmath_int &P, nanmath_int &Y, int redmode=0);
    
    /* reduce */
    static int s_reduce_setup(nanmath_int &a, nanmath_int &b);
    static int s_reduce_2k_setup(nanmath_int &a, nanmath_digit *d);
    static int s_reduce_2k_setup_l(nanmath_int &a, nanmath_int &d);
    static int s_reduce_is_2k(nanmath_int &a);
    static int s_reduce_is_2k_l(nanmath_int &a);
    static int s_reduce(nanmath_int &a, nanmath_int &b, nanmath_int &c, int redmode=0);
    
    /*
     * 数据定义区域
     */
    
  protected:
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
