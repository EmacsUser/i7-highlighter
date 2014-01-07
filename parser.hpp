#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <vector>

#include "codepoints.hpp"
#include "token.hpp"
#include "monoid_sequence.hpp"
#include "deduction.hpp"
#include "session.hpp"

using token_iterator = typename token_sequence::iterator;

class parseme {
protected:
  // True if this parseme matches an exact token; false if it matches any
  // expression of a given kind.
  bool					terminal;
  // The exact token text matched or the canonical name of the kind matched,
  // according to the terminal flag.  Internalized in vocabulary.  The empty
  // string signifies that it is acceptable to match no token, and is a safe
  // choice for such because token always contain text.
  const i7_string*			value;
  // The kind tier, zero for terminals, but possibly nonzero for nonterminals.
  // See http://inform7.com/mantis/view.php?id=1005#c1845.
  unsigned				tier;

public:
  parseme(const i7_string&terminal_word);
  parseme(const i7_string&nonterminal_kind, unsigned tier);
  parseme(const parseme&copy);
  ~parseme();
  parseme&operator =(const parseme&other);

  bool is_terminal() const;
  bool is_empty_terminal() const;
  const i7_string&get_terminal_word() const;
  const i7_string&get_nonterminal_kind() const;
  unsigned get_tier() const;

  bool accepts(const parseme&other) const;
};

class production {
protected:
  // The left-hand side of the production.
  parseme				result;
  // The right-hand side of the production, in the form (...|...)(...|...)...,
  // where the first set of alternatives must not contain ε.  This is more
  // general than necessary, but simplifies the logic for dynamic productions
  // because Inform declarations create productions in almost exactly the same
  // form.  The only difference is the exception about ε.
  //
  // The sequence is left empty in productions that match atomic literals of
  // kinds with infinite domains—numbers and strings, for instance.
  std::vector<std::vector<parseme> >	alternatives_sequence;

public:
  production(const parseme&result) : result{result} {}

  void add_slot();
  void add_alternative_to_slot(const parseme&alternative);

  const parseme&get_result() const;
  unsigned get_slot_count() const;

  bool accepts(unsigned slot_index, const ::parseme&parseme) const;
  bool can_begin_with(const ::token&token) const;
};

class match : public fact, public annotation {
protected:
  // The production that is matched or partially matched.
  const ::production&			production;
  // The number of slots matched in that production.
  const unsigned			slots_filled;
  // The first token matched, inclusive.
  const token_iterator			beginning;
  // The last token matched, exclusive.
  const token_iterator			end;

public:
  match(typename ::session&session, const ::production&production, const unsigned slots_filled, token_iterator beginning, token_iterator end);
  match(typename ::session&session, const ::production&production, token_iterator beginning);
  match(typename ::session&session, const ::production&production, token_iterator beginning, token_iterator end);

protected:
  match(const match&prefix, bool ignored);
  match(const match&prefix, const match&addendum);

  bool can_continue_with_token() const;
  bool can_continue_with(const match&addendum) const;  

  virtual void justification_hook() const override;
  virtual void unjustification_hook() const override;
  virtual std::vector<fact*>get_immediate_consequences() const override;

public:
  virtual annotation*clone() const override;
  virtual operator bool() const override;

  bool is_complete() const;
  virtual size_t hash() const override;
  virtual bool is_equal_to_instance_of_like_class(const annotation&other) const override;
};

#endif
