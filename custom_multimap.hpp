#ifndef CUSTOM_MULTIMAP_HEADER
#define CUSTOM_MULTIMAP_HEADER

#include <unordered_set>
#include <unordered_map>

template<typename K, typename V>class custom_multimap {
public:
  using value_set_type = std::unordered_set<V>;

protected:
  using map_type = std::unordered_map<K, value_set_type>;

  static const value_set_type		empty_set;

  map_type				map;

public:
  void insert(const K&key, const V&value) {
    typename map_type::iterator iterator = map.find(key);
    if (iterator == map.end()) {
      iterator = map.insert({key, value_set_type{}}).first;
    }
    iterator->second.insert(value);
  }

  bool erase(const K&key, const V&value) {
    typename map_type::iterator iterator = map.find(key);
    if (iterator == map.end()) {
      return false;
    }
    bool result = iterator->second.erase(value);
    if (iterator->second.empty()) {
      map.erase(iterator);
    }
    return result;
  }

  const value_set_type&operator[](const K&key) const {
    typename map_type::const_iterator iterator = map.find(key);
    if (iterator == map.end()) {
      return empty_set;
    }
    return iterator->second;
  }
};

template<typename K, typename V>const typename custom_multimap<K, V>::value_set_type custom_multimap<K, V>::empty_set;

#endif
