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
  for (const ::annotatable*annotatable : get_annotatables()) {
    annotatable->add_annotation(*this);
  }
}

negative_annotation_fact::operator bool() const {
  return !get_annotatables().front()->has_annotation(*this);
}

void negative_annotation_fact::surreptitiously_make_false() const {
  unjustification_hook();
}
