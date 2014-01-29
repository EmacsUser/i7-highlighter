#include <cassert>
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

void fact_annotatable::add_annotation(const ::annotation&annotation) const {
  annotatable::add_annotation(annotation);
  const ::negative_annotation_fact*negative_annotation_fact = dynamic_cast<const ::negative_annotation_fact*>(&annotation);
  if (negative_annotation_fact && negative_annotation_fact->is_observation()) {
    annotations_iterator i = justified_negative_annotation_facts.find(type_index{typeid(annotation)});
    if (i == justified_negative_annotation_facts.end()) {
      return;
    }
    const_specific_annotations_iterator j = i->second.find(annotation);
    if (j != i->second.end()) {
      i->second.erase(j);
      if (i->second.empty()) {
	justified_negative_annotation_facts.erase(i);
      }
    }
  }
}

void fact_annotatable::remove_annotation(const ::annotation&annotation) const {
  const ::negative_annotation_fact*negative_annotation_fact = dynamic_cast<const ::negative_annotation_fact*>(&annotation);
  if (negative_annotation_fact && negative_annotation_fact->is_observation()) {
    justified_negative_annotation_facts[type_index{typeid(annotation)}].insert(annotation);
  }
  annotatable::remove_annotation(annotation);
}

void fact_annotatable::predelete() {
  vector<const annotation_fact*>positive_accumulator;
  vector<const negative_annotation_fact*>negative_accumulator;
  for (auto&i : annotations) {
    for (const annotation_wrapper&j : i.second) {
      const annotation_fact*fact = dynamic_cast<const annotation_fact*>(&static_cast<const annotation&>(j));
      if (!fact || !fact->is_observation()) {
	// Continue the outer loop, effectively advancing to the next type.
	break;
      }
      positive_accumulator.push_back(dynamic_cast<const annotation_fact*>(fact->clone()));
    }
  }
  for (auto&i : justified_negative_annotation_facts) {
    for (const annotation_wrapper&j : i.second) {
      const negative_annotation_fact*fact = dynamic_cast<const negative_annotation_fact*>(&static_cast<const annotation&>(j));
      assert(fact);
      negative_accumulator.push_back(dynamic_cast<const negative_annotation_fact*>(fact->clone()));
    }
  }
  for (const annotation_fact*fact : positive_accumulator) {
    fact->unjustify();
    fact->free_as_clone();
  }
  for (const negative_annotation_fact*fact : negative_accumulator) {
    fact->unjustify();
    fact->free_as_clone();
  }
}
