#ifndef INTERNALIZER_HEADER
#define INTERNALIZER_HEADER

#include <cassert>
#include <unordered_map>

template<typename T>class internalizer {
protected:
  struct key_type {
    const T*value;
    bool operator <(const key_type&other) const {
      return *value < *other.value;
    }
    bool operator ==(const key_type&other) const {
      return *value == *other.value;
    }
  };
  struct key_type_hash {
    size_t operator()(const key_type&key) const {
      static std::hash<T>subhash;
      return subhash(*key.value);
    }
  };
  using map_type = std::unordered_map<key_type, unsigned, key_type_hash>;
  using value_type = typename map_type::value_type;
  using insertion_result_type = std::pair<typename map_type::iterator, bool>;
  map_type				elements;

public:
  const T&lookup(const T&key) const {
    typename map_type::const_iterator iterator = elements.find({&key});
    assert(iterator != elements.end());
    return *(iterator->first.value);
  }

  const T&acquire(const T&key) {
    typename map_type::iterator iterator = elements.find({&key}), end = elements.end();
    if (iterator != end) {
      ++(iterator->second);
      return *(iterator->first.value);
    }
    T*internalization = new T{key};
    insertion_result_type result = elements.insert(value_type{{internalization}, 1});
    assert(result.second);
    return *internalization;
  }

  void release(const T&key) {
    typename map_type::iterator iterator = elements.find({&key});
    assert(iterator != elements.end());
    if (!--(iterator->second)) {
      delete iterator->first.value;
      elements.erase(iterator);
    }
  }
};

#endif
