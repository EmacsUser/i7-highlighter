#ifndef DEDUCTION_HEADER
#define DEDUCTION_HEADER

#include <vector>

/* A context represents a setting in which facts can be evaluated as true or
 * false.  Usually it is or points to the object that stores the setting's
 * state.
 */
class context {
public:
  virtual ~context() {}
};

/* A fact is equivalent to a predicate on a context; it always evaluates to true
 * or false.  Some facts, ``observations'', can be directly observed from the
 * underlying data, while others, ``deductions'', are deduced from observations
 * and only directly observable in the sense that the context caches their truth
 * values.  The code structure here assumes that the deduction process is
 * monotone (truths can only be deduced from truths), acyclic (deduction cannot
 * lead to circular reasoning), sound (if a fact can be observed or deduced, it
 * is true), and complete (if a fact cannot be observed or deduced, it is
 * false).
 */
class fact {
protected:
  ::context&				context;
  /* A deduction's justification hook is run whenever a new argument for that
   * fact becomes true; it is responsible for noting the deduction's truth.  It
   * should return true iff the fact was previously false.  The default, to
   * return true without doing anything else, is the expected behavior for
   * observations.
   */
  virtual bool justification_hook() const { return true; }
  /* A deduction's unjustification hook is run whenever an argument for that
   * fact becomes false; it is responsible for noting the deduction's falsity if
   * no other arguments survive.  It should return true iff the fact became
   * false.  The default, to return true without doing anything else, is the
   * expected behavior for observations.
   */
  virtual bool unjustification_hook() const { return true; }
  /* The immediate consequences of a fact are those facts for which a direct
   * argument could be constructed in the current context, assuming that this
   * fact were made true.  Usually the implementation of this method is not in
   * the source file for the fact class itself but in a file that knows how all
   * the various fact classes are to relate, the source file for the context,
   * for instance.
   */
  virtual std::vector<fact>get_immediate_consequences() const = 0;
public:
  fact(::context&context) : context{context} {}
  virtual ~fact() {}
  /* The bool operator determines whether a fact is true or false. */
  virtual operator bool() const = 0;
  /* Except in the internals of the fact class, the justify method should only
   * be called on observations; it signals that the observation was false but
   * has become true. */
  void justify();
  /* Except in the internals of the fact class, the unjustify method should only
   * be called on observations; it signals that the observation was true but has
   * become false. */
  void unjustify();
};

#endif
