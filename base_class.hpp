#ifndef BASE_CLASS_HEADER
#define BASE_CLASS_HEADER

#include <functional>
#include <type_traits>

class base_class {
protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const = 0;
public:
  virtual ~base_class() {}
  virtual base_class*clone() const = 0;
  bool operator ==(const base_class&other) const;
  virtual size_t hash() const = 0;
};

namespace std {
  template<typename T>struct hash {
    using sfinae = typename enable_if<is_base_of<base_class, T>::value, T>::type;
    size_t operator()(const T&instance) const {
      return instance.hash();
    }
  };
}

template<typename T>class clone_wrapper {
 protected:
  T*					value;

 public:
  clone_wrapper(const T&value) : value{dynamic_cast<T*>(value.clone())} {}
  ~clone_wrapper() {
    delete value;
  }
  operator T&() {
    return *value;
  }
  operator const T&() const {
    return *value;
  }
  bool operator ==(const clone_wrapper&other) const {
    return *value == *other.value;
  }
};

namespace std {
  template<typename T>struct hash<clone_wrapper<T> > {
    using sfinae = typename enable_if<is_base_of<base_class, T>::value, T>::type;
    static hash<T>			subhash;
    size_t operator()(const clone_wrapper<T>&instance) const {
      return subhash(static_cast<const T&>(instance));
    }
  };
}

template<typename T>std::hash<T> std::hash<clone_wrapper<T> >::subhash;

#endif
