#include <vector>

#include "annotation_fact.hpp"

using namespace std;

void annotation_fact::justification_hook() const {
  for (const ::annotatable*annotatable : get_annotatables()) {
    annotatable->add_annotation(*this);
  }
}

void annotation_fact::unjustification_hook() const {
  for (const ::annotatable*annotatable : get_annotatables()) {
    annotatable->remove_annotation(*this);
  }
}

annotation_fact::operator bool() const {
  return get_annotatables().front()->has_annotation(*this);
}

void negative_annotation_fact::justification_hook() const {
  for (const ::annotatable*annotatable : get_annotatables()) {
    annotatable->remove_annotation(*this);
  }
}

void negative_annotation_fact::unjustification_hook() const {
  surreptitiously_make_false();
}

negative_annotation_fact::operator bool() const {
  return !get_annotatables().front()->has_annotation(*this);
}

void negative_annotation_fact::surreptitiously_make_false() const {
  for (const ::annotatable*annotatable : get_annotatables()) {
    annotatable->add_annotation(*this);
  }
}

void fact_annotatable::unjustify_all_annotation_facts() {
  vector<const annotation_fact*>accumulator;
  for (auto&i : annotations) {
    for (const annotation_wrapper&j : i.second) {
      const annotation_fact*fact = dynamic_cast<const annotation_fact*>(&static_cast<const annotation&>(j));
      if (!fact || !fact->is_observation()) {
	// Continue the outer loop, effectively advancing to the next type.
	break;
      }
      accumulator.push_back(dynamic_cast<const annotation_fact*>(fact->clone()));
    }
  }
  for (const annotation_fact*fact : accumulator) {
    fact->unjustify();
    fact->free_as_clone();
  }
}
