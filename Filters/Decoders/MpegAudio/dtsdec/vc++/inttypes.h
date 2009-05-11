typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed __int64 int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

#ifdef _M_AMD64
typedef signed __int64 intptr_t;
typedef unsigned __int64 uintptr_t;
#else
typedef signed int intptr_t;
typedef unsigned int uintptr_t;
#endif
