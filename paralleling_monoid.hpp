#ifndef PARALLELING_MONOID_HEADER
#define PARALLELING_MONOID_HEADER

#include <cassert>

#include "monoid_sequence.hpp"

/* A paralleling monoid for T is (monoid_sequence<T>::iterator with a fresh
 * zero, max, the fresh zero).  It is useful as a factor monoid when we want
 * elements in one monoid sequence to appear in the same order as corresponding
 * elements in another.
 */
template<typename T>class paralleling_monoid {
protected:
  using iterator_type = typename monoid_sequence<T>::iterator;
  iterator_type*			iterator;

public:
  paralleling_monoid(unsigned zero) :
    iterator{nullptr} {
    assert(!zero);
  }
  paralleling_monoid(const iterator_type&iterator) :
    iterator{new iterator_type{iterator}} {}
  paralleling_monoid(const paralleling_monoid&copy) :
    iterator{copy.iterator ?
      new iterator_type{*copy.iterator} : nullptr} {}

  ~paralleling_monoid() {
    if (iterator) {
      delete iterator;
    }
  }

  paralleling_monoid&operator =(const paralleling_monoid&other) {
    if (&other == this) {
      return *this;
    }
    if (iterator) {
      delete iterator;
    }
    iterator = other.iterator ? new iterator_type{*other.iterator} : nullptr;
    return *this;
  }

  operator iterator_type() {
    assert(iterator);
    return *iterator;
  }
  operator const iterator_type() const {
    assert(iterator);
    return *iterator;
  }

  paralleling_monoid operator +(const paralleling_monoid&other) const {
    return (*this < other) ? other : *this;
  }
  paralleling_monoid&operator +=(const paralleling_monoid&other) {
    if (*this < other) {
      return operator =(other);
    }
    return *this;
  }

  bool operator <(const paralleling_monoid&other) const {
    if (other.iterator) {
      if (iterator) {
	return *iterator < *other.iterator;
      }
      return true;
    }
    return false;
  }
};

#endif
