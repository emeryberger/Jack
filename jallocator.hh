#ifndef JALLOCATOR_H
#define JALLOCATOR_H

#include <algorithm>
#include <array>
#include <string.h>

#include "bitmap.hh"
#include "mallocinfo.hh"
#include "mapper.hh"
#include "mmapheap.hh"
#include "mwc.hh"
#include "realrandomvalue.hh"


#define MESSAGE(x) write(1,x,strlen(x))

template <size_t RefillAmount,
	  class Sizer,
	  bool UseRandomization>
class Jallocator {
public:
  enum { Alignment = Mapper<Sizer>::Alignment };

  Jallocator()
    : _freedCount (0)
  {
    MESSAGE("You don't know JACK (jallocator).\n");
    for (auto i = 0; i < Sizer::SizeClasses; i++) {
      _lastPage[i] = nullptr;
      _remainingInPage[i] = (int *) HL::MmapWrapper::map(Mapper<Sizer>::RangeSize / 
							 HL::MmapWrapper::Size);
    }
  }

  void * malloc(size_t sz) {
    //// JUST GET FRESH MEMORY AND NEVER REUSE IT.
    void * p = _theMapper.malloc(sz);
    // If this is a "small" object (smaller than a page),
    // and it's on a new page, set the remainder for that page to HL::MmapWrapper::Size bytes.
    if (sz <= HL::MmapWrapper::Size) {
      auto ind = Mapper<Sizer>::getIndex(p);
      if (((uintptr_t) p & ~(HL::MmapWrapper::Size-1)) != ((uintptr_t) _lastPage[ind])) {
	auto offset = (uintptr_t) p - Mapper<Sizer>::getBase(ind);
	_remainingInPage[ind][offset / HL::MmapWrapper::Size] = HL::MmapWrapper::Size;
	_lastPage[ind] = (void *) ((uintptr_t) p & ~(HL::MmapWrapper::Size-1));
      }
    }
    return p;
  }

  void free(void * ptr) {

    auto ind = Mapper<Sizer>::getIndex(ptr);
    auto position = Mapper<Sizer>::getPosition(ind, ptr);
    auto size = Mapper<Sizer>::getSizeFromClass(ind);

    // We are freeing a small object. Track the amount available on
    // the page.  Once it becomes empty, free it.
    if (size <= HL::MmapWrapper::Size) {
      auto base = Mapper<Sizer>::getBase(ind);
      auto start = base + position * size;
      // Clear it out to enable greater compressibility.
      memset((void *) start, 0, size);
      auto offset = (uintptr_t) ptr - base;
      _remainingInPage[ind][offset / HL::MmapWrapper::Size] -= _theMapper.getSize(ptr);
      if (_remainingInPage[ind][offset / HL::MmapWrapper::Size] == 0) {
	auto startPage = (void *) ((uintptr_t) ptr & ~(HL::MmapWrapper::Size-1));
	addToFreed(startPage);
      }
    } else {
      // Unmap immediately.
      //      HL::MmapWrapper::release (ptr, size);
      HL::MmapWrapper::unmap (ptr, size);
    }
  }

  auto getSize(void * ptr) {
    return _theMapper.getSize(ptr);
  }

private:

  void addToFreed(void * ptr) {
    if (_freedCount < FREED_BUFFER_LENGTH) {
      _freed[_freedCount] = ptr;
      _freedCount++;
    }
    if (_freedCount == FREED_BUFFER_LENGTH) {
      //      MESSAGE("DUMP\n");
      // Dump it.
      // First, sort to see if we can get large contiguous ranges.
      std::sort(_freed.begin(), _freed.end());
      // Now try to free big ranges.
      int i = 1;
      size_t sz = HL::MmapWrapper::Size;
      void * start = _freed[0];
      void * last = (void *) ((uintptr_t) start + HL::MmapWrapper::Size);
      while (i < FREED_BUFFER_LENGTH) {
	void * p = _freed[i];
	if (p == last) {
	  // Add to end of sequence.
	  sz += HL::MmapWrapper::Size;
	  last = (void *) ((uintptr_t) p + HL::MmapWrapper::Size);
	} else {
	  // Out of sequence.
	  // We had an old sequence - dump it and start a new one.
	  //	  HL::MmapWrapper::release (start, sz);
	  HL::MmapWrapper::unmap (start, sz);
	  start = p;
	  last = (void *) ((uintptr_t) p + HL::MmapWrapper::Size);
	  sz = HL::MmapWrapper::Size;
	}
	i++;
      }
      // Free remainder.
      // HL::MmapWrapper::release (start, sz);
      HL::MmapWrapper::unmap (start, sz);
      _freedCount = 0;
    }
  }

  enum { FREED_BUFFER_LENGTH = 4096 };

  int _freedCount;
  std::array<void *, FREED_BUFFER_LENGTH> _freed;
  int * _remainingInPage[Sizer::SizeClasses];
  void * _lastPage[Sizer::SizeClasses];
  Mapper<Sizer>   _theMapper;
};

#endif
