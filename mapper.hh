#ifndef MAPPER_H
#define MAPPER_H

#if defined(__linux) || defined(__APPLE__) || defined(__SVR4)
#include <sys/mman.h>
#include <unistd.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

template <class Sizer>
class Mapper {
private:
  enum { AnonFileDescriptor = -1 };
public:
  enum { Alignment = 16 };

  Mapper() {
    // Map the range of memory.  Crucially, we map it with no swap
    // space reserved (in Windows parlance, "reserved but
    // uncommitted"), i.e., we do not require that there really be
    // this much swap space pre-allocated.
#if defined(_WIN32)
    // For now, we assume that the heap is NOT executable.  If we
    // want it to be, change to PAGE_EXECUTE_READWRITE.
    auto ptr = VirtualAlloc((LPVOID) baseAddress, RangeSize * Sizer::SizeClasses, MEM_RESERVE | MEM_RESET | MEM_LARGE_PAGES, PAGE_READWRITE);
    if (!ptr) {
      abort();
    }
#else
    void * r = mmap((void *) baseAddress,
		    RangeSize * Sizer::SizeClasses,
		    PROT_READ | PROT_WRITE,
		    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED | MAP_NORESERVE,
		    AnonFileDescriptor, 0);
    if (MAP_FAILED == r) {
      abort();
    }
#endif
    // Allocate the ranges of address space.
    for (auto i = 0; i < Sizer::SizeClasses; i++) {
      _info[i]._freeptr   = (void *) (baseAddress + i * RangeSize);
      _info[i]._remaining = RangeSize;
    }
  }

  inline static auto getSizeClass(void * ptr) {
    const uintptr_t p = (uintptr_t) ptr - baseAddress;
    return (p / RangeSize);
  }

  inline static auto * getStart(void * ptr) {
    // Align the pointer appropriately, just in case it's a free to
    // the middle of an object.
    auto cl = getSizeClass(ptr);
    auto uptr = (uintptr_t) ptr;
    auto start = baseAddress + cl * RangeSize; // start of memory for this size class
    auto offset = uptr - start;                // offset from start for this pointer
    auto pointerBase = Sizer::getSizeFromClass(cl) * (offset / Sizer::getSizeFromClass(cl));
    void * p = (void *) (start + pointerBase);
    return p;

    return (void *) ((uintptr_t) ptr & ~(Sizer::getSizeFromClass(((uintptr_t) ptr - baseAddress) >> log2RangeSize)-1));
  }

  inline static int getIndex(void * ptr) {
    int index = ((uintptr_t) ptr - baseAddress) / RangeSize;
    return index;
  }

  inline static auto getBase(int cl) {
    return (uintptr_t) baseAddress + RangeSize * cl;
  }

  uint32_t static getPosition(void * ptr) {
    auto cl = getSizeClass(ptr);
    auto start = (baseAddress + cl * RangeSize);
    uint32_t pos = ((uintptr_t) ptr - start) / (uint32_t) Sizer::getSizeFromClass(cl);
    return pos;
  }

  uint32_t static getPosition(int cl, void * ptr) {
    auto start = (baseAddress + cl * RangeSize);
    uint32_t pos = ((uintptr_t) ptr - start) / (uint32_t) Sizer::getSizeFromClass(cl);
    return pos;
  }

  static size_t getSizeFromClass(int cl) {
    return Sizer::getSizeFromClass(cl);
  }

  auto * malloc(size_t sz) {
    // Compute size class.
    assert(sz >= HL::MallocInfo::MinSize);
    auto index = Sizer::getClassFromSize(sz);
    sz = Sizer::getSizeFromClass(index);
    auto& remaining = _info[index]._remaining;
    auto& freeptr   = _info[index]._freeptr;
    void * ptr = nullptr;
    if (remaining >= sz) {
      ptr = freeptr;
      freeptr = (void *) ((uintptr_t) freeptr + sz);
      remaining -= sz;
    }
    return ptr;
  }

  auto isValid(void * ptr) {
    if (((uintptr_t) ptr < baseAddress) ||
	((uintptr_t) ptr > baseAddress + RangeSize * Sizer::SizeClasses)) {
      // Out of bounds.
      return false;
    }
    // EDB FIX ME - kick out frees to middle for now. debugging only.
    //    if (getStart(ptr) != ptr) {
    //      return false;
    //    }
    return true;
  }

  void free(void * ptr) {
    abort();
  }

  inline static auto inRange(void * ptr) {
    if (((uintptr_t) ptr < baseAddress) ||
	((uintptr_t) ptr > baseAddress + RangeSize * Sizer::SizeClasses)) {
      return false;
    }
    return true;
  }

  inline static auto getSize(void * ptr) {
#if 0
    if (!inRange(ptr)) {
      return 0;
    }
#endif
    int index = ((uintptr_t) ptr - baseAddress) / RangeSize;
    if ((index < 0) || (index > Sizer::SizeClasses)) {
      // out of range.
      return 0UL;
    }
    auto szFromClass = Sizer::getSizeFromClass(index);
    auto alignedPtr  = getStart(ptr); // (void *) ((uintptr_t) ptr & ~(szFromClass-1));
    auto distance    = (uintptr_t) ptr - (uintptr_t) alignedPtr;
    return szFromClass - distance;
  }

  static const size_t RangeSize   = 0x10000000000; // 1TB per size class
  //  static const size_t RangeSize   = 4UL * 1024UL * 1024UL * 1024UL; // 4GB per size class

private:

  static const uint64_t baseAddress = 0x10000000000; // start of allocated objects
  static const auto log2RangeSize = staticlog(RangeSize);

  /// Per size-class stuff.
  struct AllocationInfo {
    void *   _freeptr;
    uint64_t _remaining;
  };

  AllocationInfo     _info[Sizer::SizeClasses];
};

#endif
