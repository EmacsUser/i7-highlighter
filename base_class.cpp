#include <typeinfo>

#include "base_class.hpp"

bool base_class::operator ==(const base_class&other) const {
  if (this == &other) {
    return true;
  }
  if (typeid(*this) != typeid(other)) {
    return false;
  }
  return is_equal_to_instance_of_like_class(other);
}
