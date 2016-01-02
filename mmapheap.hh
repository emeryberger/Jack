#ifndef MMAPHEAP_HH
#define MMAPHEAP_HH

#include "mmapwrapper.hh"

class MmapHeap {
public:
  enum { Alignment = HL::MmapWrapper::Alignment };
  void * malloc(size_t sz) {
    return HL::MmapWrapper::map(sz);
  }
  void free (void *) {
    abort();
  }
  size_t getSize(void *) {
    abort();
  }
};

#endif
