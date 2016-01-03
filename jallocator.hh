#ifndef JALLOCATOR_H
#define JALLOCATOR_H

#include "bitmap.hh"
#include "mallocinfo.hh"
#include "mapper.hh"
#include "mmapheap.hh"
#include "mwc.hh"
#include "realrandomvalue.hh"


template <size_t RefillAmount,
	  class Sizer,
	  bool UseRandomization>
class Jallocator {
public:
  enum { Alignment = Mapper<Sizer>::Alignment };

  Jallocator()
    : _count (0),
      _rng (RealRandomValue::value(), RealRandomValue::value())
  {
    write(1,"You don't know JACK (jallocator).\n", 21);
    for (auto i = 0; i < Sizer::SizeClasses; i++) {
      _lastPage[i] = nullptr;
      _remainingInPage[i] = (int *) HL::MmapWrapper::map(Mapper<Sizer>::RangeSize / 4096);
    }
  }

  void * malloc(size_t sz) {
    // Compute size class.
    auto ind = Sizer::getClassFromSize(sz);

    //// JUST GET FRESH MEMORY AND NEVER REUSE IT.
    auto newSize = Sizer::getSizeFromClass(ind);
    void * p = _theMapper.malloc(newSize);
    if (sz <= 4096) {
      if (((uintptr_t) p & ~4095) != ((uintptr_t) _lastPage[ind])) {
	auto offset = (uintptr_t) p - Mapper<Sizer>::getBase(ind);
	_remainingInPage[ind][offset / 4096] = 4096;
	_lastPage[ind] = (void *) ((uintptr_t) p & ~4095);
      }
    }
    return p;
  }

  void free(void * ptr) {

    auto ind = Mapper<Sizer>::getIndex(ptr);
    auto size = Mapper<Sizer>::getSizeFromClass(ind);
    if (size <= 4096) {
      auto offset = (uintptr_t) ptr - Mapper<Sizer>::getBase(ind);
      _remainingInPage[ind][offset / 4096] -= _theMapper.getSize(ptr);
      if (_remainingInPage[ind][offset / 4096] == 0) {
	auto startPage = (void *) ((uintptr_t) ptr & ~4095);
	// Unmap once we drop to 0.
	HL::MmapWrapper::unmap (startPage, 4096);
      }
    } else {
      // Unmap immediately.
      HL::MmapWrapper::unmap (ptr, size);
    }
  }

  auto getSize(void * ptr) {
    return _theMapper.getSize(ptr);
  }

private:

  int _count;

  int * _remainingInPage[Sizer::SizeClasses];
  void * _lastPage[Sizer::SizeClasses];

  MWC      _rng;
  Mapper<Sizer>   _theMapper;
};

#endif
