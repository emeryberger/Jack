// -*- C++ -*-

#ifndef POW2SIZECLASSMANAGER_H
#define POW2SIZECLASSMANAGER_H

#include <cstdint>
#include <cstdlib>
//#include "log2.h"

#ifdef _WIN32
#include <intrin.h>
#pragma intrinsic(_BitScanReverse64)
#pragma intrinsic(_BitScanReverse)
#endif

using namespace std;

class Pow2SizeClassManager {
public:

  enum { SizeClasses = 32 };

  //  static constexpr std::uint64_t _sizes[SizeClasses] = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648, 4294967296, 8589934592, 17179869184, 34359738368 };

#ifdef _WIN32
  static constexpr inline size_t getSizeFromClass(int index)
#else
  static constexpr inline size_t getSizeFromClass(int index)
#endif
  {
    return (16UL << index);
    //    return _sizes[index];
  }

  static inline unsigned int getClassFromSize(size_t sz) {
    return log2(sz) - 4;
  }

  //private:

  /// Quickly calculate the CEILING of the log (base 2) of the argument.
#if defined(_WIN32)
  static inline unsigned int log2 (size_t sz) 
  {
    unsigned long retval = 0;
    sz = (sz << 1) - 1;
#if defined(_WIN64)
    _BitScanReverse64(&retval, sz);
#else
    _BitScanReverse(&retval, sz);
#endif
#if 0
    __asm {
      bsr eax, sz
	mov retval, eax
	}
#endif
    return retval;
  }
#elif 0 // defined(__GNUC__) && defined(__i386__)
  static inline unsigned int log2 (size_t sz) 
  {
    sz = (sz << 1) - 1;
    asm ("bsrl %0, %0" : "=r" (sz) : "0" (sz));
    return (unsigned int) sz;
  }
#elif 0 // defined(__GNUC__) && defined(__x86_64__)
  static inline unsigned int log2 (size_t sz) 
  {
    sz = (sz << 1) - 1;
    asm ("bsrq %0, %0" : "=r" (sz) : "0" (sz));
    return (unsigned int) sz;
  }
#elif defined(__GNUC__)
  // Just use the intrinsic.
  static inline constexpr unsigned long log2 (size_t sz) 
  {
    return (sizeof(unsigned long) * 8) - __builtin_clzl((sz << 1) - 1) - 1;
  }
#else
  static inline constexpr unsigned int log2 (size_t v) {
#if 0
    static const int MultiplyDeBruijnBitPosition[32] = 
      {
	0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
      };
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    // 0x218A392CD3D5DBF is a 64-bit deBruijn number.
    return MultiplyDeBruijnBitPosition[(v * 0x077CB531UL) >> 27];
#else
    unsigned int log = 0;
    unsigned int value = 1;
    while (value < v) {
      value <<= 1;
      log++;
    }
    return log;
#endif
  }
#endif


};



#endif
