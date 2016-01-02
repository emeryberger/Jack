/*
 * @file jack.cpp
 * @author Emery Berger <http://www.emeryberger.com>
 *
 * "You don't know jack - about where objects are allocated."
 * (more or less)
 *
 */


////////// Key parameters that control Jack's behavior.

//// Choose which size class range to use.

#define USE_POW2_SIZECLASSES 1

//// Enable randomized allocation.

#define USE_RANDOMIZATION 1

///////////////////////////////////////////////

#include <stdio.h>
#include <iostream>
using namespace std;

#include "ansiwrapper.hh"
#include "lockedheap.hh"
#include "maclock.hh"
#include "mallocator.hh"
#include "mallocinfo.hh"
#include "halflogsizeclassmanager.hh"
#include "pow2sizeclassmanager.hh"

#if USE_POW2_SIZECLASSES
typedef Pow2SizeClassManager Sizer;
#else
typedef HalfLogSizeClassManager Sizer;
#endif

/// The definition of the custom heap.
/// ANSI semantics, locked with one big Mac lock, around a Jack mallocator.

typedef HL::ANSIWrapper<HL::LockedHeap<HL::MacLock, Mallocator<4096, Sizer, USE_RANDOMIZATION>>> theHeap;

//bool isCustomHeapInitialized = false;

/// The custom heap itself.

inline static auto * getCustomHeap() {
  static char buf[sizeof(theHeap)];
  static auto * _theCustomHeap = 
    new (buf) theHeap;
  //  isCustomHeapInitialized = true;
  return _theCustomHeap;
}

#if 0
// Disabled: allocation buffer for pre-initialization (before our custom heap is ready).
enum { MAX_LOCAL_BUFFER_SIZE = 256 * 131072 };
static char initBuffer[MAX_LOCAL_BUFFER_SIZE];
static char * initBufferPtr = initBuffer;
#endif

extern "C" {

  void * xxmalloc (size_t sz) {
    void * ptr = getCustomHeap()->malloc(sz);
#if 0 // for stats only
    static int count = 0;
    count++;
    if (count % 10000 == 0) {
      char buf[255];
      sprintf(buf,"%zu\n",sz);
      write(1,buf,strlen(buf));
    }
#endif
    return ptr;

#if 0
    if (!isCustomHeapInitialized) {
      // We still haven't initialized the heap. Satisfy this memory
      // request from the local buffer.
      void * ptr = initBufferPtr;
      initBufferPtr += sz;
      if (initBufferPtr > initBuffer + MAX_LOCAL_BUFFER_SIZE) {
	abort();
      }
      return ptr;
    } else {
      return getCustomHeap()->malloc(sz);
    }
#endif
  }

  void xxfree (void * ptr) {
    getCustomHeap()->free(ptr);
  }

  size_t xxmalloc_usable_size (void * ptr) {
    return getCustomHeap()->getSize(ptr);
  }

  void xxmalloc_lock() {
    getCustomHeap()->lock();
  }

  void xxmalloc_unlock() {
    getCustomHeap()->unlock();
  }
}


#if TESTRUN
// For testing purposes only, rather than building as a dylib.
int
main()
{
  auto sz = 8;
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 2; j++) {
      auto * p = getCustomHeap()->malloc(sz);
      getCustomHeap()->free(p);
      cout << p << endl;
    }
    sz += 8;
  }
  return 0;
}
#endif
