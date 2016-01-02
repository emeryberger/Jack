#ifndef MALLOCATOR_H
#define MALLOCATOR_H

#include "bitmap.hh"
#include "mallocinfo.hh"
#include "mapper.hh"
#include "mmapheap.hh"
#include "mwc.hh"
#include "realrandomvalue.hh"


template <size_t RefillAmount,
	  class Sizer,
	  bool UseRandomization>
class Mallocator {
public:
  enum { Alignment = Mapper<Sizer>::Alignment };

  Mallocator()
    : _count (0),
      _rng (RealRandomValue::value(), RealRandomValue::value())
  {
    write(1,"You don't know JACK.\n", 21);
    for (auto i = 0; i < Sizer::SizeClasses; i++) {
      _index[i] = 0;
      _pointer[i] = (void **) HL::MmapWrapper::map(Mapper<Sizer>::RangeSize / HL::MallocInfo::MinSize);
      if (Sizer::getSizeFromClass(i) <= HL::MallocInfo::MaxSize) {
	const auto nelts = (uint64_t) Mapper<Sizer>::RangeSize / (uint64_t) Sizer::getSizeFromClass(i);
	if (nelts > 0) {
	  _bitmap[i].reserve (nelts);
	}
      }
    }
  }

  void * malloc(size_t sz) {
    // Compute size class.
    auto ind = Sizer::getClassFromSize(sz);
    // If empty, refill.
    if (_index[ind] == 0) {
      auto newSize = Sizer::getSizeFromClass(ind);
      auto r = RefillAmount < newSize ? newSize : RefillAmount;
      while (r >= newSize) {
	// Grab some memory.
	void * p = _theMapper.malloc(newSize);
	// Mark it as allocated.
	auto position = Mapper<Sizer>::getPosition(p);
	_bitmap[ind].tryToSet(position);
	// Now free it locally.
	free(p);
	r -= newSize;
      }
    }
#if 0
    _count++;
    if (_count % 100000 == 0) {
      char buf[255];
      sprintf(buf, "%d mallocs\n", _count);
      write(1, buf, strlen(buf));
    }
#endif
    // Peel one off the end.
    assert (_index[ind] > 0);
    void * ptr = _pointer[ind][_index[ind]-1];
    _index[ind]--;
    if (ptr == nullptr) { return nullptr; } // should throw exception here!
    // Set the appropriate bit.
    auto position = Mapper<Sizer>::getPosition(ind, ptr);
    _bitmap[ind].tryToSet(position);
    return ptr;
  }

  void free(void * ptr) {
    // Compute size class.
    assert(ptr != nullptr);
    auto ind = Mapper<Sizer>::getIndex(ptr);
    if ((ind < 0) || (ind > Sizer::SizeClasses)) {
      // Invalid object: do nothing.
      return;
    }
    // Canonicalize the pointer just in case.
    ptr = _theMapper.getStart(ptr);
    auto position = Mapper<Sizer>::getPosition(ind, ptr);
    bool wasAllocated = _bitmap[ind].reset(position);
    if (!wasAllocated) {
      // Already freed: return to avoid a double-free error.
      return;
    }
    auto i = _index[ind];
    _pointer[ind][i] = ptr;
    if (UseRandomization) {
      auto j = _rng.next() % (i + 1);
      std::swap (_pointer[ind][i], _pointer[ind][j]);
    }
    _index[ind]++;
  }

  auto getSize(void * ptr) {
    return _theMapper.getSize(ptr);
  }

private:

  int _count;

  BitMap<MmapHeap> _bitmap[Sizer::SizeClasses];
  MWC      _rng;
  void **  _pointer[Sizer::SizeClasses];
  int      _index[Sizer::SizeClasses];
  Mapper<Sizer>   _theMapper;
};

#endif
