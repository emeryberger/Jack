#ifndef BITMAP_HH
#define BITMAP_HH

#include "staticfun.hh"

template <class Heap>
class BitMap : private Heap {
public:

  BitMap()
    : _bitarray (nullptr),
      _elements (0)
  {
  }

  /**
   * @brief Sets aside space for a certain number of elements.
   * @param  nelts  the number of elements needed.
   */
  
  void reserve (uint64_t nelts) {
    if (_bitarray) {
      Heap::free (_bitarray);
    }
    // Round up the number of elements.
    _elements = WORDBITS * ((nelts + WORDBITS - 1) / WORDBITS);
    // Allocate the right number of bytes.
    void * buf = Heap::malloc (_elements / 8);
    if (buf == nullptr) {
      char b[255];
      sprintf(b, "OHNOES: %u, %u\n", nelts, _elements / 8);
      write(1,b,strlen(b));
      abort();
    }
    _bitarray = (WORD *) buf;
    clear();
  }

  /// Clears out the bitmap array.
  void clear() {
    if (_bitarray != nullptr) {
      memset (_bitarray, 0, _elements / 8); // 0 = false
    }
  }

  /// @return true iff the bit was not set (but it is now).
  inline bool tryToSet (uint64_t index) {
    uint64_t item, position;
    computeItemPosition (index, item, position);
    const WORD mask = getMask(position);
    unsigned long oldvalue = _bitarray[item];
    _bitarray[item] |= mask;
    return !(oldvalue & mask);
  }

  /// Clears the bit at the given index.
  /// @return True iff the bit was previously set.
  inline bool reset (uint64_t index) {
    uint64_t item, position;
    computeItemPosition (index, item, position);
    unsigned long oldvalue = _bitarray[item];
    WORD newvalue = oldvalue &  ~(getMask(position));
    _bitarray[item] = newvalue;
    return (oldvalue != newvalue);
  }

  inline bool isSet (uint64_t index) const {
    uint64_t item, position;
    computeItemPosition (index, item, position);
    bool result = _bitarray[item] & getMask(position);
    return result;
  }

private:

  /// Given an index, compute its item (word) and position within the word.
  void computeItemPosition (uint64_t index,
			    uint64_t& item,
			    uint64_t& position) const
  {
    const int WordBitShift = staticlog(sizeof(size_t) * 8);
    assert (index < _elements);
    item = index >> WordBitShift;
    position = index & (WORDBITS - 1);
    assert (position == index - (item << WordBitShift));
    assert (item < _elements / WORDBYTES);
  }

  /// A synonym for the datatype corresponding to a word.
  typedef size_t WORD;

  /// To find the bit in a word, do this: word & getMask(bitPosition)
  /// @return a "mask" for the given position.
  inline static WORD getMask (uint64_t pos) {
    return ((WORD) 1) << pos;
  }

  /// The number of bits in a WORD.
  enum { WORDBITS = sizeof(WORD) * 8 };

  /// The number of BYTES in a WORD.
  enum { WORDBYTES = sizeof(WORD) };

  /// The log of the number of bits in a WORD, for shifting.
  //  enum { WORDBITSHIFT = staticlog((int) WORDBITS) }; // StaticLog<WORDBITS>::VALUE };

  /// The bit array itself.
  WORD * _bitarray;
  
  /// The number of elements in the array.
  uint64_t _elements;

};

#endif
