#ifndef PARSER_HEADER
#define PARSER_HEADER

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

  virtual std::vector<const annotatable*>get_annotatables() const override;
  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

public:
  virtual base_class*clone() const override;
  virtual size_t hash() const override;
};

class next_token : public annotation_fact {
protected:
  const token_iterator			self;
  const token_iterator			next;

public:
  next_token(typename ::session&session, token_iterator self, token_iterator next);

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual std::vector<const annotatable*>get_annotatables() const override;

  virtual std::vector<fact*>get_immediate_consequences() const override;

public:
  token_iterator get_self() const;
  token_iterator get_next() const;

  virtual base_class*clone() const override;
  virtual size_t hash() const override;
};

// These functions are saturating: they return their argument if there is no
// link to a previous/next token_iterator.
token_iterator previous(const token_iterator&iterator);
token_iterator next(const token_iterator&iterator);

class parseme : public base_class {
public:
  virtual bool accepts(const token_iterator&iterator) const = 0;
  virtual bool accepts(const parseme&other) const;
};

class epsilon_terminal : public parseme {
protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

public:
  virtual bool accepts(const token_iterator&iterator) const override;

  virtual base_class*clone() const override;
  virtual size_t hash() const override;
};

class something_unrecognized_terminal : public parseme {
protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

public:
  virtual bool accepts(const token_iterator&iterator) const override;
  virtual bool accepts(const parseme&other) const override;

  virtual base_class*clone() const override;
  virtual size_t hash() const override;
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

  virtual base_class*clone() const override;
  virtual size_t hash() const override;
};

class nonterminal : public parseme {
protected:
  const i7_string*			kind_name;
  // See http://inform7.com/mantis/view.php?id=1005#c1845.
  unsigned				tier;

public:
  nonterminal(const i7_string&kind_name, unsigned tier);
  nonterminal(const nonterminal&copy, unsigned replacement_tier);
  virtual ~nonterminal();

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

public:
  const i7_string&get_kind_name() const;
  unsigned get_tier() const;

  virtual bool accepts(const token_iterator&iterator) const override;
  virtual bool accepts(const parseme&other) const override;

  virtual base_class*clone() const override;
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
  nonterminal				result;
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

public:
  production(typename ::session&session, const nonterminal&result);
  ~production();

protected:
  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

public:
  void add_slot();
  void add_alternative_to_last_slot(const parseme&alternative);

  const nonterminal&get_result() const;
  unsigned get_slot_count() const;
  const std::vector<possible_beginning>&get_beginnings() const;
  const std::vector<const parseme*>&get_alternatives(unsigned slot_index) const;

  bool accepts(unsigned slot_index, const ::parseme&parseme) const;
  std::vector<unsigned>can_begin_with(const ::token&token) const;
  std::vector<unsigned>can_begin_with(const ::parseme&parseme) const;

  virtual operator bool() const override;

  virtual base_class*clone() const override;
  virtual size_t hash() const override;
};

class match : public annotation_fact {
protected:
  ::buffer*				buffer;
  // The production that is matched or partially matched.
  const ::production*			production;
  // The number of slots matched in that production.
  const unsigned			slots_filled;
  // The first token matched, inclusive.
  const token_iterator			beginning;
  // The last token matched, also inclusive, contrary to the usual convention.
  // (This is because a match may be constructed before the link to the
  // exclusive bound is asserted).
  const token_iterator			inclusive_end;

public:
  match(typename ::session&session, ::buffer*buffer, const ::production&production, const unsigned slots_filled, token_iterator beginning, token_iterator inclusive_end);
  match(typename ::session&session, ::buffer*buffer, const ::production&production, const unsigned slots_filled, token_iterator beginning);
  ~match();

protected:
  friend class token_available;
  friend class next_token;

  match(const match&prefix, token_iterator inclusive_end);
  match(const ::production&production, const match&addendum);
  match(const match&prefix, const match&addendum);

  bool can_continue_with(token_iterator end) const;
  bool can_continue_with(const match&addendum) const;

  virtual bool is_equal_to_instance_of_like_class(const base_class&other) const override;

  virtual std::vector<const annotatable*>get_annotatables() const override;
  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

public:
  bool is_complete() const;

  virtual base_class*clone() const override;
  virtual size_t hash() const override;
};

extern internalizer<parseme>parseme_bank;
extern internalizer<production>production_bank;

#endif
