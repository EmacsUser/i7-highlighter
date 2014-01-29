#ifndef HASHABLE_HEADER
#define HASHABLE_HEADER

#include <functional>

namespace std {
  template<typename T>struct hash {
    size_t operator()(const T&instance) const {
      return instance.hash();
    }
  };
}

#endif
