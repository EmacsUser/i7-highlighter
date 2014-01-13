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
public:
  virtual void unjustify_all_annotation_facts();
};

#endif
