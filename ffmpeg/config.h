#if defined(__GNUC__) || defined(__INTEL_COMPILER)
 #ifndef __INTEL_COMPILER
  #define ARCH_X86 1
 #endif
 #define HAVE_MMX 1
 #define HAVE_BUILTIN_VECTOR 1
 #define __CPU__ 586
#endif

#ifndef DECODERS_ONLY
 #define CONFIG_ENCODERS 1
#endif
#define CONFIG_DECODERS 1
#define HAVE_MALLOC_H 1
#define HAVE_LRINTF 1
#undef  HAVE_MEMALIGN
#define SIMPLE_IDCT 1
#define CONFIG_ZLIB 1
#define HAVE_W32THREADS 1

#ifdef __GNUC__
// #define printf(...) /**/
// #define fprintf(...) /**/
#else
// #define printf(x) (x) /**/
// #define fprintf(x) (x) /**/
 #ifndef __attribute__
  #define __attribute__(x) /**/
 #endif
 #define lrintf(x) (int)(x)
 #define EMULATE_FAST_INT
#endif
