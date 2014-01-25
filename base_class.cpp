#include <typeinfo>

#include "base_class.hpp"

void base_class::free_as_clone() const {
  delete this;
}

bool base_class::operator ==(const base_class&other) const {
  if (this == &other) {
    return true;
  }
  if (typeid(*this) != typeid(other)) {
    return false;
  }
  return is_equal_to_instance_of_like_class(other);
}
