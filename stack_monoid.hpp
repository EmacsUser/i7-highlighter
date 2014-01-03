#ifndef STACK_MONOID_HEADER
#define STACK_MONOID_HEADER

#include <cassert>

#include "monoid_sequence.hpp"

/* An element of the stack monoid represents a sequence of pushes and pops with
 * symbol type T.  Pops are particular, and specify which symbol is meant to be
 * popped.  In case of a mismatch, either the pop is ignored or the topmost
 * symbol is discarded and the pop retried.  The latter happens if, letting X be
 * the symbol to pop and Y be the actual top symbol, X.trumps(Y) returns true.
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
    vector<T>::const_iterator
      pops_begin = other.pops.begin(),
      pops_end = other.pops.end();
    for (; pops_begin != pops_end && pushes.size(); ++pops_begin) {
      while (*pops_begin != pushes.back()) {
	if (pops_begin->trumps(pushes.back)) {
	  pushes.pop_back();
	  if (!pushes.size()) {
	    goto done_with_cancellation;
	  }
	} else {
	  goto ignore_pop;
	}
      }
      pushes.pop_back();
    ignore_pop:
    }
  done_with_cancellation:
    pops.insert(pops.end(), pops_begin, pops_end);
    pushes.insert(pushes.end(), other.pushes.begin(), other.pushes.end());
  }
};

#endif
