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

#ifndef HL_LOCKEDHEAP_H
#define HL_LOCKEDHEAP_H

#include <cstddef>
#include <mutex>

namespace HL {

  template <class LockType, class Super>
  class LockedHeap : public Super {
  public:

    enum { Alignment = Super::Alignment };

    inline void * malloc (size_t sz) {
      std::lock_guard<LockType> l (thelock);
      return Super::malloc (sz);
    }

    inline void free (void * ptr) {
      std::lock_guard<LockType> l (thelock);
      Super::free (ptr);
    }

    inline size_t getSize (void * ptr) const {
      std::lock_guard<LockType> l (thelock);
      return Super::getSize (ptr);
    }

    inline size_t getSize (void * ptr) {
      std::lock_guard<LockType> l (thelock);
      return Super::getSize (ptr);
    }

    inline void lock() {
      thelock.lock();
    }

    inline void unlock() {
      thelock.unlock();
    }

  private:
    //    char dummy[128]; // an effort to avoid false sharing.
    LockType thelock;
  };

}

#endif