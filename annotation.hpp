#ifndef ANNOTATION_HEADER
#define ANNOTATION_HEADER

#include <typeindex>
#include <unordered_set>
#include <unordered_map>

#include "base_class.hpp"

class annotation : public base_class {};

using annotation_wrapper = clone_wrapper<annotation>;

class annotatable {
protected:
  using specific_annotations_type = std::unordered_multiset<annotation_wrapper>;
  using const_specific_annotations_iterator = typename specific_annotations_type::const_iterator;
  using annotations_type = std::unordered_map<std::type_index, specific_annotations_type>;
  using annotations_iterator = typename annotations_type::iterator;
  using const_annotations_iterator = typename annotations_type::const_iterator;
  static const specific_annotations_type
					no_specific_annotations;

  // Annotation changes are considered semantically const.
  mutable annotations_type		annotations;

public:
  virtual ~annotatable() {}

  bool has_annotation(const ::annotation&annotation) const;
  const annotation*get_annotation(const ::annotation&annotation) const;
  // Annotation changes are considered semantically const.
  virtual void add_annotation(const ::annotation&annotation) const;
  virtual void remove_annotation(const ::annotation&annotation) const;

  const specific_annotations_type&get_annotations(std::type_index type) const;
};

#endif
