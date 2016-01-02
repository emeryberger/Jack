// -*- C++ -*-

#ifndef HALFLOGSIZECLASSMANAGER_H
#define HALFLOGSIZECLASSMANAGER_H

#include <cstdint>
#include <cstdlib>
//#include "log2.h"

#ifdef _WIN32
#include <intrin.h>
#pragma intrinsic(_BitScanReverse64)
#pragma intrinsic(_BitScanReverse)
#endif

using namespace std;

class HalfLogSizeClassManager {
public:

  enum { SizeClasses = 54 };

  static constexpr inline size_t getSizeFromClass(int index)
  {
    auto sz = getThatSize(index);
    return sz;
#if 0
    if (index == 0) {
      return 16UL;
    }
    if (index % 2 == 1) {
      return 32UL << (index / 2);
    }
    size_t v1 = (16UL << index);
    size_t v2 = (32UL << index);
    return (v1 + v2) / 2;
#endif
  }

  static inline unsigned int getClassFromSize(size_t sz) {
    unsigned int ind = 0;
    while (sz > getThatSize(ind)) {
      ind++;
    }
    assert(sz <= getThatSize(ind));
    return ind;
    // return log2(sz) - 4;
  }

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

private:

  inline static constexpr size_t getThatSize(int ind) {
    constexpr std::uint64_t _sizes[SizeClasses] = { 16, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576, 32768, 49152, 65536, 98304, 131072, 196608, 262144, 393216, 524288, 786432, 1048576, 1572864, 2097152, 3145728, 4194304, 6291456, 8388608, 12582912, 16777216, 25165824, 33554432, 50331648, 67108864, 100663296, 134217728, 201326592, 268435456, 402653184, 536870912, 805306368, 1073741824, 1610612736, 2147483648 };
    return _sizes[ind];
  }


};



#endif
