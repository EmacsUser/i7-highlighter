#include <cassert>
#include <typeinfo>

#include "annotation.hpp"

using namespace std;

const annotatable::specific_annotations_type annotatable::no_specific_annotations;

bool annotatable::has_annotation(const ::annotation&annotation) const {
  const_annotations_iterator i = annotations.find(type_index{typeid(annotation)});
  if (i == annotations.end()) {
    return false;
  }
  return i->second.find(annotation) != i->second.end();
}

const annotation*annotatable::get_annotation(const ::annotation&annotation) const {
  const_annotations_iterator i = annotations.find(type_index{typeid(annotation)});
  assert(i != annotations.end());
  const_specific_annotations_iterator j = i->second.find(annotation);
  assert(j != i->second.end());
  return &static_cast<const ::annotation&>(*j);
}

void annotatable::add_annotation(const ::annotation&annotation) const {
  annotations[type_index{typeid(annotation)}].insert(annotation);
}

void annotatable::remove_annotation(const ::annotation&annotation) const {
  annotations_iterator i = annotations.find(type_index{typeid(annotation)});
  if (i == annotations.end()) {
    return;
  }
  const_specific_annotations_iterator j = i->second.find(annotation);
  if (j != i->second.end()) {
    i->second.erase(j);
    if (i->second.empty()) {
      annotations.erase(i);
    }
  }
}

const annotatable::specific_annotations_type&annotatable::get_annotations(type_index type) const {
  const_annotations_iterator i = annotations.find(type);
  if (i == annotations.end()) {
    return no_specific_annotations;
  }
  return i->second;
}
