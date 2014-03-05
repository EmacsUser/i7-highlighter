#ifndef STACK_MONOID_HEADER
#define STACK_MONOID_HEADER

#include <cassert>
#include <utility>

#include "monoid_sequence.hpp"

/* An element of the stack monoid represents a sequence of pushes and pops with
 * symbol type T.  Pops can be particular and specify which symbol is meant to
 * be popped, but symbols satisfying the predicate E are exempt and silently
 * ignored if doing so can resync pushes and pops.
 *
 * This monoid has no natural ordering, and therefore cannot be used with
 * monoid_sequence except in a product monoid.
 */
template<typename T, const std::function<bool(const T&)>&exemption>class stack_monoid {
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

  const std::vector<T>&get_pops() const {
    return pops;
  }
  const std::vector<T>&get_pushes() const {
    return pushes;
  }

  bool is_empty() const {
    return pops.empty() && pushes.empty();
  }
  bool is_unit_pop() const {
    return pushes.size() == 0 && pops.size() == 1;
  }
  bool is_unit_push() const {
    return pops.size() == 0 && pushes.size() == 1;
  }
  const T&get_unit() const {
    if (is_unit_pop()) {
      return pops.front();
    }
    assert(is_unit_push());
    return pushes.front();
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
      if (exemption(*pops_begin)) {
	continue;
      }
      while (exemption(pushes.back())) {
	pushes.pop_back();
      }
      assert(*pops_begin == pushes.back());
      pushes.pop_back();
    }
    pops.insert(pops.end(), pops_begin, pops_end);
    pushes.insert(pushes.end(), other.pushes.begin(), other.pushes.end());
    return *this;
  }
};

#endif
