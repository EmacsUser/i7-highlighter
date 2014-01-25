#ifndef BASE_CLASS_HEADER
#define BASE_CLASS_HEADER

#include "hashable.hpp"

class base_class {
protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const = 0;
public:
  virtual ~base_class() {}
  virtual const base_class*clone() const = 0;
  virtual void free_as_clone() const;
  bool operator ==(const base_class&other) const;
  virtual size_t hash() const = 0;
};

template<typename T>class clone_wrapper {
 protected:
  const T*				value;

 public:
  clone_wrapper(const T&value) : value{dynamic_cast<const T*>(value.clone())} {}
  clone_wrapper(const clone_wrapper&copy) : value{dynamic_cast<const T*>(copy.value->clone())} {}
  ~clone_wrapper() {
    value->free_as_clone();
  }
  clone_wrapper&operator =(const clone_wrapper&other) {
    if (this != &other) {
      value->free_as_clone();
      value = dynamic_cast<T*>(other.value->clone());
    }
    return *this;
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
    static hash<T>			subhash;
    size_t operator()(const clone_wrapper<T>&instance) const {
      return subhash(static_cast<const T&>(instance));
    }
  };
}

template<typename T>std::hash<T> std::hash<clone_wrapper<T> >::subhash;

#endif
