#ifndef STACK_MONOID_HEADER
#define STACK_MONOID_HEADER

#include <cassert>

#include "monoid_sequence.hpp"

/* An element of the stack monoid represents a sequence of pushes and pops with
 * symbol type T.  Pops are particular, and specify which symbol is meant to be
 * popped.
 *
 * This monoid has no natural ordering, and therefore cannot be used with
 * monoid_sequence except in a product monoid.
 */
template<typename T>class stack_monoid {
protected:
  std::vector<T>			pops;
  std::vector<T>			pushes;

public:
  stack_monoid(unsigned zero) { assert(!zero); }
  stack_monoid(const T&symbol, bool is_pop) {
    if (is_pop) {
      pops.push_back(symbol);
    } else {
      pushes.push_back(symbol);
    }
  }

  stack_monoid operator +(const stack_monoid&other) const {
    stack_monoid result{*this};
    return result += other;
  }

  stack_monoid&operator +=(const stack_monoid&other) {
    typename std::vector<T>::const_iterator
      pops_begin = other.pops.begin(),
      pops_end = other.pops.end();
    for (; pops_begin != pops_end && pushes.size(); ++pops_begin) {
      assert(*pops_begin == pushes.back());
      pushes.pop_back();
    }
    pops.insert(pops.end(), pops_begin, pops_end);
    pushes.insert(pushes.end(), other.pushes.begin(), other.pushes.end());
  }
};

#endif
