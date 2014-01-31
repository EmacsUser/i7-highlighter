#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <iostream>

#include <utility>
#include <vector>
#include <unordered_set>

#include "base_class.hpp"
#include "codepoints.hpp"
#include "token.hpp"
#include "monoid_sequence.hpp"
#include "annotation.hpp"
#include "buffer.hpp"
#include "deduction.hpp"
#include "annotation_fact.hpp"
#include "session.hpp"

using token_sequence = monoid_sequence<token>;
using token_iterator = typename token_sequence::iterator;

class token_available : public negative_annotation_fact {
protected:
  ::buffer*				buffer;
  const token_iterator			self;

public:
  token_available(typename ::session&session, ::buffer*buffer, token_iterator self);
  token_available(typename ::session&session, token_iterator self);

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual std::vector<const fact_annotatable*>get_annotatables() const override;
  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  virtual bool is_observation() const override { return true; }

  virtual const base_class*clone() const override;
  virtual size_t hash() const override;
};

class next_token : public annotation_fact {
protected:
  ::buffer*				buffer;
  const token_iterator			self;
  const token_iterator			next;

public:
  next_token(typename ::session&session, ::buffer*buffer, token_iterator self, token_iterator next);
  next_token(typename ::session&session, token_iterator self, token_iterator next);

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual std::vector<const fact_annotatable*>get_annotatables() const override;

  virtual std::vector<fact*>get_immediate_consequences() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  token_iterator get_self() const;
  token_iterator get_next() const;

  virtual bool is_observation() const override { return true; }

  virtual const base_class*clone() const override;
  virtual size_t hash() const override;
};

// These functions are saturating: they return their argument if there is no
// link to a previous/next token_iterator.
token_iterator previous(const token_iterator&iterator);
token_iterator next(const token_iterator&iterator);

class end_of_unit : public combined_annotation_fact {
protected:
  const token_iterator			self;

public:
  end_of_unit(typename ::session&session, token_iterator self, bool in_the_positive_sense);

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual std::vector<const fact_annotatable*>get_annotatables() const override;

public:
  token_iterator get_self() const;

  virtual size_t hash() const override;
};

class end_of_sentence : public end_of_unit {
protected:
  ::buffer*				buffer;

public:
  end_of_sentence(typename ::session&session, ::buffer*buffer, token_iterator self, bool in_the_positive_sense);
  end_of_sentence(typename ::session&session, token_iterator self, bool in_the_positive_sense);

protected:
  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  virtual const base_class*clone() const override;
};

class parseme : public base_class {
public:
  // This version of accepts is to be called when trying to match terminals.
  virtual bool accepts(const token_iterator&iterator) const;
  // This version of accepts is to be called when trying to match nonterminals.
  virtual bool accepts(const parseme&other) const;
};

template<size_t hash_value>class unique_terminal : public parseme {
protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override { return true; }

public:
  virtual size_t hash() const override { return hash_value; }
};

class epsilon_terminal : public unique_terminal<0> {
public:
  virtual bool accepts(const token_iterator&iterator) const override;
  virtual const base_class*clone() const override;
};

class digits_terminal : public unique_terminal<1> {
public:
  virtual bool accepts(const token_iterator&iterator) const override;
  virtual const base_class*clone() const override;
};

class word_terminal : public unique_terminal<2> {
public:
  virtual bool accepts(const token_iterator&iterator) const override;
  virtual const base_class*clone() const override;
};

class name_word_terminal : public unique_terminal<3> {
public:
  virtual bool accepts(const token_iterator&iterator) const override;
  virtual const base_class*clone() const override;
};

class token_terminal : public parseme {
protected:
  const i7_string*			text;

public:
  token_terminal(const i7_string&text);
  ~token_terminal();

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

public:
  const i7_string&get_text() const;

  virtual bool accepts(const token_iterator&iterator) const override;

  virtual const base_class*clone() const override;
  virtual size_t hash() const override;
};

class nonterminal : public parseme {
protected:
  const i7_string*			kind_name;
  // See http://inform7.com/mantis/view.php?id=1005#c1845.
  unsigned				tier;

public:
  nonterminal(const i7_string&kind_name, unsigned tier);
  nonterminal(const nonterminal&copy);
  nonterminal(const nonterminal&copy, unsigned replacement_tier);
  virtual ~nonterminal();

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

public:
  const i7_string&get_kind_name() const;
  unsigned get_tier() const;

  virtual bool accepts(const parseme&other) const override;

  virtual const base_class*clone() const override;
  virtual size_t hash() const override;
};

class production : public base_class, public fact {
public:
  struct possible_beginning {
    const unsigned			epsilon_count;
    const ::parseme*			parseme;
  };

protected:
  // The left-hand side of the production.
  const nonterminal*			result;
  // The right-hand side of the production, in the form (...|...)(...|...)....
  // This is more general than necessary, but simplifies the logic for dynamic
  // productions because Inform 7 declarations create them in exactly the same
  // form.
  //
  // The sequence is left empty in productions that match atomic literals of
  // kinds with infinite domains—numbers and strings, for instance.
  std::vector<std::vector<const parseme*> >
					alternatives_sequence;
  // The length of the longest prefix in alternatives_sequence that can match
  // all epsilons.  Should not be the length of alternatives_sequence except
  // mid-construction.
  unsigned				epsilon_prefix_length;
  // All of the non-epsilon parsemes that may begin this production.
  std::vector<possible_beginning>	beginnings;
  // The cached hash value.
  size_t				hash_value;
  // Whether or not this production is internally generated, and therefore an
  // observation.
  const bool				internal;

public:
  production(typename ::session&session, const nonterminal&result, bool internal = false);
  production(const production&copy);
  virtual ~production();

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual bool can_begin_sentence() const = 0;
  virtual std::vector<fact*>get_immediate_consequences() const override;

public:
  // Note that the following violates the fact contract.  It is only safe to do
  // so because productions, and internal productions in particular, never
  // annotate.
  virtual bool is_observation() const { return internal; }

  void add_slot();
  void add_alternative_to_last_slot(const parseme&alternative);

  const nonterminal*get_result() const;
  unsigned get_slot_count() const;
  const std::vector<possible_beginning>&get_beginnings() const;
  const std::vector<const parseme*>&get_alternatives(unsigned slot_index) const;

  // This version of accepts is to be called when trying to match terminals.
  bool accepts(unsigned slot_index, const ::token_iterator&iterator) const;
  // This version of accepts is to be called when trying to match nonterminals.
  bool accepts(unsigned slot_index, const ::parseme&parseme) const;
  // This version of can_begin_with is to be called when trying to match terminals.
  std::vector<unsigned>can_begin_with(const ::token_iterator&iterator) const;
  // This version of can_begin_with is to be called when trying to match nonterminals.
  std::vector<unsigned>can_begin_with(const ::parseme&parseme) const;

  // For checking extra constraints like positions relative to end-of-sentence
  // markers.
  bool can_reach_slot_count_at(unsigned slot_count, token_iterator position) const;
  virtual bool can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const = 0;
  virtual size_t hash() const override;
};

// A subsentence matches any token sequence that does not cross a sentence
// boundary.
class subsentence : public production {
public:
  subsentence(typename ::session&session, const nonterminal&result, bool internal = false);
  subsentence(const subsentence&copy);

protected:
  virtual bool can_begin_sentence() const override;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  virtual operator bool() const override;

  virtual bool can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const override;
  virtual const base_class*clone() const override;
};

/* A wording matches any token sequence that begins in plain I7 (possibly within
 * an extract) or documentation, ends in the same lexical state, and does not
 * cross a sentence boundary.
 */
class wording : public production {
public:
  wording(typename ::session&session, const nonterminal&result, bool internal = false);
  wording(const wording&copy);

protected:
  virtual bool can_begin_sentence() const override;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  virtual operator bool() const override;

  virtual bool can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const override;
  virtual const base_class*clone() const override;
};

/* A sentence matches any token sequence that begins in plain I7 (possibly
 * within an extract) or documentation, ends in the same lexical state just
 * before a sentence boundary, and does not cross a sentence boundary.
 *
 * Ordinarily a sentence must also begin just after a sentence boundary, but
 * this requirement is waived when matching part of a larger sentence or
 * passage, a concession made for the sake of sentence-splitting commas.
 */
class sentence : public production {
public:
  sentence(typename ::session&session, const nonterminal&result, bool internal = false);
  sentence(const sentence&copy);

protected:
  virtual bool can_begin_sentence() const override;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  virtual operator bool() const override;

  virtual bool can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const override;
  virtual const base_class*clone() const override;
};

/* A passage matches any token sequence that begins in plain I7 (possibly within
 * an extract) or documentation, usually just after a sentence boundary, and
 * ends in the same lexical state just before a sentence boundary.  It is
 * ordinarily used to agglomerate several sentences that are part of a fixed
 * pattern, like the preamble-body form in definitions, phrases, and rules.
 *
 * Like sentences, passages may begin in places other than after a sentence
 * boundary if they are part of a larger passage.  This makes the parser a
 * little simpler to write, but it's not clear that this caveat is actually
 * useful for anything.
 */
class passage : public production {
public:
  passage(typename ::session&session, const nonterminal&result, bool internal = false);
  passage(const passage&copy);

protected:
  virtual bool can_begin_sentence() const override;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  virtual operator bool() const override;

  virtual bool can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const override;
  virtual const base_class*clone() const override;
};

class potential_match : public annotation_fact {
protected:
  ::buffer*				buffer;
  // The production that is matched or partially matched.
  const ::production*			production;
  // The number of slots matched in that production.
  const unsigned			slots_filled;
  // The first token matched, inclusive.
  const token_iterator			beginning;
  // The last token matched, also inclusive, contrary to the usual convention.
  // (This is because a potential match may be constructed before the link to
  // the exclusive bound is asserted).
  const token_iterator			inclusive_end;

public:
  potential_match(typename ::session&session, ::buffer*buffer, const ::production&production, unsigned slots_filled, token_iterator beginning, token_iterator inclusive_end);
  potential_match(typename ::session&session, ::buffer*buffer, const ::production&production, unsigned slots_filled, token_iterator beginning);
  potential_match(const potential_match&copy);
  potential_match(const potential_match&prefix, token_iterator inclusive_end, bool assume_next_token_is_justified = false);
  potential_match(const ::production&production, unsigned slots_filled, const potential_match&addendum);
  potential_match(const potential_match&prefix, const potential_match&addendum, bool assume_next_token_is_justified = false);
  ~potential_match();

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual std::vector<const fact_annotatable*>get_annotatables() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  const nonterminal&get_result() const;
  const ::production&get_production() const;
  unsigned get_slots_filled() const;
  token_iterator get_beginning() const;
  token_iterator get_inclusive_end() const;

  bool is_filled() const;
  bool is_complete() const;
  bool can_continue_with(token_iterator end, bool assume_next_token_is_justified = false) const;
  bool can_continue_with(const potential_match&addendum, bool assume_next_token_is_justified = false) const;
  const std::vector<const parseme*>&get_continuing_alternatives() const;

  virtual const base_class*clone() const override;
  virtual size_t hash() const override;
};

class match : public potential_match {
public:
  match(const potential_match&copy);

protected:
  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

  virtual std::ostream&print(std::ostream&out) const override;

public:
  bool is_complete() const;

  virtual const base_class*clone() const override;
};

extern internalizer<parseme>parseme_bank;
extern internalizer<production>production_bank;

#endif
