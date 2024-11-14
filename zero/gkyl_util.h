#pragma once

#include <stdbool.h>
#include <slu_mt_ddefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// random number generator
#include <pcg_basic.h>

#ifdef __cplusplus

// extern "C" guards needed when using code from C++
# define EXTERN_C_BEG extern "C" {
# define EXTERN_C_END }

#else

# define EXTERN_C_BEG 
# define EXTERN_C_END

#endif

// restrict keyword in C and C++ are different
#ifdef __cplusplus
# define GKYL_RESTRICT __restrict__
#else
# define GKYL_RESTRICT restrict
#endif

// Maximum configuration-space dimensions supported
#ifndef GKYL_MAX_CDIM
# define GKYL_MAX_CDIM 3
#endif

// Maximum dimensions supported
#ifndef GKYL_MAX_DIM
# define GKYL_MAX_DIM 7
#endif

// Maximum number of supported species
#ifndef GKYL_MAX_SPECIES
# define GKYL_MAX_SPECIES 8
#endif

// Maximum number of supported projection objects
#ifndef GKYL_MAX_PROJ
# define GKYL_MAX_PROJ 4
#endif

// Maximum number of ghost cells in each direction
#ifndef GKYL_MAX_NGHOST
# define GKYL_MAX_NGHOST 8
#endif

// Default alignment boundary
#ifndef GKYL_DEF_ALIGN
# define GKYL_DEF_ALIGN 64
#endif

// CUDA specific defines etc
#ifdef __NVCC__

#include <cuda_runtime.h>

#define GKYL_HAVE_CUDA

#define GKYL_CU_DH __device__ __host__
#define GKYL_CU_D __device__ 

// for directional copies
enum gkyl_cu_memcpy_kind {
  GKYL_CU_MEMCPY_H2H = cudaMemcpyHostToHost,
  GKYL_CU_MEMCPY_H2D = cudaMemcpyHostToDevice,
  GKYL_CU_MEMCPY_D2H = cudaMemcpyDeviceToHost,
  GKYL_CU_MEMCPY_D2D = cudaMemcpyDeviceToDevice
};

#define GKYL_DEFAULT_NUM_THREADS 256

// CUDA helper function to find CUDA errors
#define checkCuda(val)           __checkCudaErrors__ ( (val), #val, __FILE__, __LINE__ )
inline cudaError_t __checkCudaErrors__(cudaError_t code, const char *func, const char *file, int line)
{
  if (code) {
    fprintf(stderr, "CUDA error: %s (code=%u)  \"%s\" at %s:%d \n",
      cudaGetErrorString(code), (unsigned int)code, func, file, line);
    cudaDeviceReset();
    exit(EXIT_FAILURE);
  }
  return code;
}

#else

#undef GKYL_HAVE_CUDA
#define GKYL_CU_DH
#define GKYL_CU_D
#define checkCuda(val) 
// for directional copies
enum gkyl_cu_memcpy_kind {
  GKYL_CU_MEMCPY_H2H,
  GKYL_CU_MEMCPY_H2D,
  GKYL_CU_MEMCPY_D2H,
  GKYL_CU_MEMCPY_D2D,
};

#define GKYL_DEFAULT_NUM_THREADS 1

#endif // CUDA specific defines etc

// This funny looking macro allows getting a pointer to the 'type'
// struct that contains an object 'member' given the 'ptr' to the
// 'member' inside 'type'. (Did I just write this gobbledygook?!)
//
// See https://en.wikipedia.org/wiki/Offsetof
#define container_of(ptr, type, member)                                 \
    ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member)))

// Select type-specific compare function
#define gkyl_compare(a, b, eps)                 \
    _Generic((a),                               \
      float: gkyl_compare_float,                \
      double: gkyl_compare_double)              \
    (a, b, eps)

// a quick-and-dirty macro for testing (mostly) CUDA kernel code
#define GKYL_CU_CHECK(expr, cntr) do {                                  \
      if (!(expr)) {                                                    \
        *cntr += 1;                                                     \
        printf("%s failed! (%s:%d)\n", #expr, __FILE__, __LINE__);      \
      }                                                                 \
    } while (0)

// Computes length of string needed given a format specifier and data. Example:
//
// size_t len = gkyl_calc_strlen("%s-%d", "gkyl", 25);
// 
#define gkyl_calc_strlen(fmt, ...) snprintf(0, 0, fmt, __VA_ARGS__)

// Open file 'fname' with 'mode; into handle 'fp'. Handle is closed
// when block attached to with_file exits
#define with_file(fp, fname, mode)                                             \
  for (bool _break = (fp = fopen(fname, mode), (fp != NULL)); _break;          \
       _break = false, fclose(fp))

// Code

#define GKYL_MIN2(x,y) ((x)<(y) ? (x) : (y))
#define GKYL_MAX2(x,y) ((x)>(y) ? (x) : (y))
#define GKYL_SGN(b) (((b)>=0.) ? 1.0 : -1.0)

EXTERN_C_BEG

/**
 * Kernel floating-point op-counts
 */
struct gkyl_kern_op_count {
  size_t num_sum; // number of + and - operations
  size_t num_prod; // number of * and / operations
};

/**
 * Time-trigger. Typical initialization is:
 * 
 * struct gkyl_tm_trigger tmt = { .dt = tend/nframe };
 */
struct gkyl_tm_trigger {
  int curr; // current counter
  double dt, tcurr; // Time-interval, current time
};

/**
 * Check if the tcurr should trigger and bump internal counters if it
 * does. This only works if sequential calls to this method have the
 * tcurr monotonically increasing.
 *
 * @param tmt Time trigger object
 * @param tcurr Current time.
 * @return 1 if triggered, 0 otherwise
 */
int gkyl_tm_trigger_check_and_bump(struct gkyl_tm_trigger *tmt, double tcurr);

/**
 * Print error message to stderr and exit.
 *
 * @param msg Error message.
 */
void gkyl_exit(const char* msg);

/**
 * Compares two float numbers 'a' and 'b' to check if they are
 * sufficiently close by, where 'eps' is the relative tolerance.
 */
int gkyl_compare_float(float a, float b, float eps);

/**
 * Compares two double numbers 'a' and 'b' to check if they are
 * sufficiently close by, where 'eps' is the relative tolerance.
 */
int gkyl_compare_double(double a, double b, double eps);

/**
 * Copy (small) int arrays.
 *
 * @param n Number of elements to copy
 * @param inp Input array
 * @param out Output array
 */
GKYL_CU_DH
static inline void
gkyl_copy_int_arr(int n, const int* GKYL_RESTRICT inp, int* GKYL_RESTRICT out)
{
  for (int i=0; i<n; ++i) out[i] = inp[i];
}

/**
 * Copy (small) long arrays.
 *
 * @param n Number of elements to copy
 * @param inp Input array
 * @param out Output array
 */
GKYL_CU_DH
static inline void
gkyl_copy_long_arr(int n, const long* GKYL_RESTRICT inp, long* GKYL_RESTRICT out)
{
  for (int i=0; i<n; ++i) out[i] = inp[i];
}

/**
 * Copy (small) double arrays.
 *
 * @param n Number of elements to copy
 * @param inp Input array
 * @param out Output array
 */
GKYL_CU_DH
static inline void
gkyl_copy_double_arr(int n, const double* GKYL_RESTRICT inp, double* GKYL_RESTRICT out)
{
  for (int i=0; i<n; ++i) out[i] = inp[i];
}

/**
 *   Round a/b to nearest higher integer value
 */
GKYL_CU_DH
static inline int
gkyl_int_div_up(int a, int b)
{
  return (a%b != 0) ? (a/b+1) : (a/b);
}

/**
 *   Minmod limiter for choosing the minimal modification between 3 values (usually slopes)
 */
GKYL_CU_DH
static inline double 
gkyl_minmod(double a, double b, double c)
{
  double sa = GKYL_SGN(a);
  double sb = GKYL_SGN(b);
  double sc = GKYL_SGN(c);
  if( (sa==sb) && (sb==sc) ) {
    if (sa<0)
      return GKYL_MAX2(GKYL_MAX2(a,b),c);
    else
      return GKYL_MIN2(GKYL_MIN2(a,b),c);
  }
  else {
     return 0;
  }
}

/**
 * Gets wall-clock time in secs/nanoseconds.
 * 
 * @return Time object.
 */
struct timespec gkyl_wall_clock(void);

/**
 * Difference between two timespec objects.
 *
 * @param tstart Start time
 * @param tend End time 
 * @return Time object representing difference
 */
struct timespec gkyl_time_diff(struct timespec tstart, struct timespec tend);

/**
 * Difference between timespec object and "now", returned in seconds.
 * 
 * @param tm Timespec
 * @return Time in seconds
 */
double gkyl_time_diff_now_sec(struct timespec tm);

/**
 * Compute in secs time stored in timespec object.
 *
 * @param tm Timespec object
 * @return Time in seconds
 */
double gkyl_time_sec(struct timespec tm);

/**
 * Compute time in seconds since epoch.
 *
 * @return Time in seconds
 */
double gkyl_time_now(void);

/**
 * Initialize 32-bit RNG 
 *
 * @param nd_seed true if to use non-deterministic seed, or false for
 *   a determistic seed.
 */
pcg32_random_t gkyl_pcg32_init(bool nd_seed);

/**
 * Returns a unsigned 32-bit random number
 *
 * @param rng Pointer to RNG
 * @return Uniformly distributed 32-bit integer
 */
uint32_t gkyl_pcg32_rand_uint32(pcg32_random_t* rng);

/**
 * Returns a random number in [0,1), rounded to the nearest
 * 1/2^32. This may not be enough resolution for some applications.
 *
 * @param rng Pointer to RNG
 * @return Uniformly distributed double in [0,1)
 */
double gkyl_pcg32_rand_double(pcg32_random_t* rng);

/** 64-bit RNG: concatination of two 32-bit RNGs */
typedef struct { pcg32_random_t gen[2]; } pcg64_random_t;

/**
 * Initialize 64-bit RNG 
 *
 * @param nd_seed true if to use non-deterministic seed, or false for
 *   a determistic seed.
 */
pcg64_random_t gkyl_pcg64_init(bool nd_seed);

/**
 * Returns a unsigned 64-bit random number
 *
 * @param rng Pointer to RNG
 * @return Uniformly distributed 64-bit integer
 */
uint64_t gkyl_pcg64_rand_uint64(pcg64_random_t* rng);

/**
 * Returns a random number in [0,1), rounded to the nearest
 * 1/2^64.
 *
 * @param rng Pointer to RNG
 * @return Uniformly distributed double in [0,1)
 */
double gkyl_pcg64_rand_double(pcg64_random_t *rng);

/**
 * Check if file exists.
 *
 * @param fname Name of file
 * @return true if file exists, false otherwise
 */
bool gkyl_check_file_exists(const char *fname);

/**
 * Get file size in bytes.
 *
 * @param fname Name of file
 * @return file size in bytes
 */
int64_t gkyl_file_size(const char *fname);

/**
 * Read contents of file into a character buffer. The returned buffer
 * must be freed using gkyl_free.
 *
 * @param fname Name of file
 * @param sz On return this has the size of data read
 * @return Data in file as a char array.
 */
char* gkyl_load_file(const char *fname, int64_t *sz);

EXTERN_C_END
