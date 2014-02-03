#ifndef ANNOTATION_FACT_HEADER
#define ANNOTATION_FACT_HEADER

#include <iostream>
#include <vector>

#include "annotation.hpp"
#include "deduction.hpp"

class fact_annotatable;

class annotation_fact : public annotation, public fact {
protected:
  virtual std::vector<const fact_annotatable*>get_annotatables() const = 0;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

public:
  annotation_fact(::context&context) : fact{context} {}

  virtual operator bool() const override;
  virtual void justify() const override;
};

class negative_annotation_fact : public annotation, public fact {
protected:
  virtual std::vector<const fact_annotatable*>get_annotatables() const = 0;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

public:
  negative_annotation_fact(::context&context) : fact{context} {}

  virtual operator bool() const override;
  void surreptitiously_make_false() const;
};

class combined_annotation_fact : public annotation_fact {
protected:
  bool					in_the_positive_sense;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

public:
  combined_annotation_fact(::context&context, bool in_the_positive_sense) : annotation_fact{context}, in_the_positive_sense{in_the_positive_sense} {}

  bool is_in_the_positive_sense() const;

  virtual bool is_observation() const final { return true; }
};

// The obvious fourth class, negative_combined_annotation_fact, is missing
// simply because, at the moment we don't need it.

class fact_annotatable : public annotatable {
protected:
  // Annotation changes are considered semantically const.
  mutable annotations_type		justified_negative_annotation_facts;
  mutable bool				predeleted;

public:
  fact_annotatable();

  virtual void add_annotation(const ::annotation&annotation) const override;
  virtual void remove_annotation(const ::annotation&annotation) const override;

  bool has_been_predeleted() const;
  virtual void predelete();

  virtual std::ostream&dump(std::ostream&out) const;
};

#endif
