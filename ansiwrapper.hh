/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2012 by Emery Berger
  http://www.cs.umass.edu/~emery
  emery@cs.umass.edu
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef HL_ANSIWRAPPER_H
#define HL_ANSIWRAPPER_H

#include <assert.h>
#include <string.h>

#include "mallocinfo.hh"


/*
 * @class ANSIWrapper
 * @brief Provide ANSI C behavior for malloc & free.
 *
 * Implements all prescribed ANSI behavior, including zero-sized
 * requests & aligned request sizes to a double word (or long word).
 */

#include "staticfun.hh"

namespace HL {

  template <class SuperHeap>
  class ANSIWrapper : public SuperHeap {
  public:
  
    ANSIWrapper() {
      static_assert((gcd((int) SuperHeap::Alignment, (int) HL::MallocInfo::Alignment) == HL::MallocInfo::Alignment),
		    "Alignment mismatch.");
    }

    inline void * malloc (size_t sz) {
      // Prevent integer underflows. This maximum should (and
      // currently does) provide more than enough slack to compensate for any
      // rounding below (in the alignment section).
      if (sz > HL::MallocInfo::MaxSize) {
	return nullptr;
      }
      if (sz < HL::MallocInfo::MinSize) {
      	sz = HL::MallocInfo::MinSize;
      }
      // Enforce alignment requirements: round up allocation sizes if needed.
      // NOTE: Alignment needs to be a power of two.
      static_assert((HL::MallocInfo::Alignment & (HL::MallocInfo::Alignment - 1)) == 0,
		    "Alignment needs to be a power of two.");

      // Enforce alignment.
      sz = (sz + HL::MallocInfo::Alignment - 1) & ~(HL::MallocInfo::Alignment - 1);

      void * ptr = SuperHeap::malloc (sz);
      assert ((size_t) ptr % HL::MallocInfo::Alignment == 0);
      return ptr;
    }
 
    inline void free (void * ptr) {
      if (ptr != 0) {
	SuperHeap::free (ptr);
      }
    }

    inline void * calloc (const size_t s1, const size_t s2) {
      char * ptr = (char *) malloc (s1 * s2);
      if (ptr) {
      	memset (ptr, 0, s1 * s2);
      }
      return (void *) ptr;
    }
  
    inline void * realloc (void * ptr, const size_t sz) {
      if (ptr == 0) {
      	return malloc (sz);
      }
      if (sz == 0) {
      	free (ptr);
      	return 0;
      }

      size_t objSize = getSize (ptr);
      if (objSize == sz) {
    	return ptr;
      }

      // Allocate a new block of size sz.
      void * buf = malloc (sz);

      // Copy the contents of the original object
      // up to the size of the new block.

      size_t minSize = (objSize < sz) ? objSize : sz;
      if (buf) {
	memcpy (buf, ptr, minSize);
      }

      // Free the old block.
      free (ptr);
      return buf;
    }
  
    inline size_t getSize (void * ptr) {
      if (ptr) {
	return SuperHeap::getSize (ptr);
      } else {
	return 0;
      }
    }
  };

}

#endif
