#ifndef ANNOTATION_FACT_HEADER
#define ANNOTATION_FACT_HEADER

#include <vector>

#include "annotation.hpp"
#include "deduction.hpp"

class annotation_fact : public annotation, public fact {
protected:
  virtual std::vector<const annotatable*>get_annotatables() const = 0;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

public:
  annotation_fact(::context&context) : fact{context} {}

  virtual operator bool() const override;
};

class negative_annotation_fact : public annotation, public fact {
protected:
  virtual std::vector<const annotatable*>get_annotatables() const = 0;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

public:
  negative_annotation_fact(::context&context) : fact{context} {}

  virtual operator bool() const override;
  void surreptitiously_make_false() const;
};

class fact_annotatable : public annotatable {
protected:
  // Annotation changes are considered semantically const.
  mutable annotations_type		justified_negative_annotation_facts;

public:
  virtual void add_annotation(const ::annotation&annotation) const override;
  virtual void remove_annotation(const ::annotation&annotation) const override;

  virtual void predelete();
};

#endif
