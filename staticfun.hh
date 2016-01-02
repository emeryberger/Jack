#ifndef STATICFUN_HH
#define STATICFUN_HH

template <class TYPE>
TYPE constexpr staticif(bool v, TYPE a, TYPE b) {
  return (v ? a : b);
}

template <class TYPE>
TYPE constexpr staticlog(TYPE v) {
  return ((v == 1) ? 0 :
	  (v == 2) ? 1 :
	  (v > 1) ? staticlog(v / 2) + 1 :
	  0);
}

template <class TYPE>
TYPE constexpr gcd(TYPE a, TYPE b) {
  return (b == 0) ? a : gcd(b, a % b);
}

#endif
