#ifndef ANNOTATION_HEADER
#define ANNOTATION_HEADER

#include <cassert>

#include <typeinfo>
#include <typeindex>
#include <unordered_set>
#include <unordered_map>

class annotation_wrapper;

class annotation {
protected:
  friend struct std::hash<annotation_wrapper>;
  virtual size_t hash() const = 0;
  virtual bool is_equal_to_instance_of_like_class(const annotation&other) const = 0;

public:
  virtual ~annotation() {}
  virtual annotation*clone() const = 0;
  bool operator ==(const annotation&other) const;
};

class annotation_wrapper {
protected:
  friend struct std::hash<annotation_wrapper>;
  const ::annotation*			annotation;

public:
  annotation_wrapper(const ::annotation&annotation) :
    annotation{annotation.clone()} {}
  operator const ::annotation&() const {
    return *annotation;
  }
  bool operator ==(const annotation_wrapper&other) const {
    return *annotation == *other.annotation;
  }
};

namespace std {
  template<>struct hash<annotation_wrapper> {
    size_t operator()(const ::annotation_wrapper&annotation_wrapper) const {
      return annotation_wrapper.annotation->hash();
    }
  };
}

class annotatable {
protected:
  using specific_annotations_type = std::unordered_multiset<annotation_wrapper>;
  using const_specific_annotations_iterator = typename specific_annotations_type::const_iterator;
  using annotations_type = std::unordered_map<std::type_index, specific_annotations_type>;
  using annotations_iterator = typename annotations_type::iterator;
  using const_annotations_iterator = typename annotations_type::const_iterator;
  static const specific_annotations_type
					no_specific_annotations;

  annotations_type			annotations;

public:
  virtual ~annotatable() {}

  bool has_annotation(const ::annotation&annotation) const;
  // Annotation changes are considered semantically const.
  void add_annotation(const ::annotation&annotation) const;
  void remove_annotation(const ::annotation&annotation) const;

  const specific_annotations_type&get_annotations(std::type_index type) const;
};

#endif
