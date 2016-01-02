#ifndef MWC_HH
#define MWC_HH

class MWC {
public:

  MWC (uint32_t seed1, uint32_t seed2)
    : z (seed1), w (seed2), b (0)
  {}

  inline uint8_t next() {
    if (b == 0) {
      // These magic numbers are derived from a note by George Marsaglia.
      uint32_t znew = (z=36969*(z&65535)+(z>>16));
      uint32_t wnew = (w=18000*(w&65535)+(w>>16));
      x = (znew << 16) + wnew;
      b = sizeof(uint32_t) / sizeof(uint8_t);
    }
    uint8_t q = x;
    x >>= 8;
    b--;
    return q;
  }
  
private:

  uint32_t z;
  uint32_t w;
  uint32_t x;
  uint32_t b;

};

#endif
