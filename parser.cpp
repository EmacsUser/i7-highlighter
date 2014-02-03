#include <cassert>
#include <typeinfo>

#include "utility.hpp"
#include "parser.hpp"
#include "internalizer.hpp"

using namespace std;

static bool is_plain_i7_or_documentation(lexical_state state) {
  if (!state.get_comment_depth()) {
    switch (state.get_superstate()) {
    case I7:
    case I7_EXTENSION_DOCUMENTATION:
    case I7_IN_EXTRACT:
      return true;
    }
  }
  return false;
}

static bool is_in_plain_i7_or_documentation(token_iterator iterator) {
  const token_sequence&source_text = iterator.get_owner();
  return is_plain_i7_or_documentation(source_text.sum_over_interval(source_text.begin(), iterator).get_lexical_effect()(INITIAL_LEXICAL_STATE));
}

// With CYK filtering, the deduction rules for matches split into the following
// cases, with the noted antecedent structures and restrictions:

// Case Ia: Beginning of a match at the beginning of a sentence with a token.
// (sentence ending) (next) (sentence/passage --> production) (token available)
// 0. If the new match will be complete, its end condition must be satisfied.

// Case Ib: Beginning of a match at the beginning of a sentence with a match already made.
// (sentence ending) (next) (sentence/passage --> production) (complete match)
// 0. If the new match will be complete, its end condition must be satisfied.

// Case IIa: Beginning of a continuing match with a token.
// (end of partial match) (next) (need --> production) (token available)
// 0. If the new match will be complete, its end condition must be satisfied.

// Case IIb:  Beginning of a continuing match with a match already made.
// (end of partial match) (next) (need --> production) (complete match)
// 0. If the new match will be complete, its end condition must be satisfied.

// Case IIIa: Continuation of a match with a token.
// (end of partial match) (next) (token available)
// 0. If the new match will be complete, its end condition must be satisfied.
// 1. The new match's crossing condition must be satisfied.

// Case IIIb: Continuation of a match with another match.
// (end of partial match) (next) (complete match)
// 0. If the new match will be complete, its end condition must be satisfied.
// (1. The new match's crossing condition must be satisfied, but this is enforced by the nonterminal hierarchy.)

// Case IIIc: Continuation of a match with an epsilon.
// (end of partial match)
// 0. If the new match will be complete, its end condition must be satisfied.

// Case IV: Conditions 0 and 1 are enforced by guards calling can_reach_slot_count_at(...).


// Case Ia: Beginning of a match at the beginning of a sentence with a token.
// Case IIa: Beginning of a continuing match with a token.
static void begin_matches_with_token(vector<fact*>&results, typename ::session&session, const unordered_set<const production*>&productions, ::buffer*buffer, token_iterator position) {
  assert(buffer);
  for (const ::production*production : productions) {
    for (unsigned slot_index : production->can_begin_with(position)) {
      results.push_back(new potential_match{session, buffer, *production, slot_index + 1, position});
    }
  }
}

// Case IIb:  Beginning of a continuing match with a match already made.
static void begin_matches_with_match(vector<fact*>&results, const unordered_set<const production*>&productions, const match&beginning_match) {
  assert(beginning_match.is_filled());
  const nonterminal&match_result = beginning_match.get_result();
  for (const ::production*production : productions) {
    for (unsigned slot_index : production->can_begin_with(match_result)) {
      results.push_back(new potential_match{*production, slot_index + 1, beginning_match});
    }
  }
}

// Case Ib: Beginning of a match at the beginning of a sentence with a match already made.
static void begin_matches_with_match(vector<fact*>&results, const intersection<const production*>&productions, const match&beginning_match) {
  assert(beginning_match.is_filled());
  const nonterminal&match_result = beginning_match.get_result();
  for (const ::production*production : productions) {
    for (unsigned slot_index : production->can_begin_with(match_result)) {
      results.push_back(new potential_match{*production, slot_index + 1, beginning_match});
    }
  }
}

// Case Ib: Beginning of a match at the beginning of a sentence with a match already made.
// Case IIb:  Beginning of a continuing match with a match already made.
static void begin_matches_with_match(vector<fact*>&results, const unordered_set<const production*>&productions, token_iterator position) {
  for (const annotation_wrapper&beginning_wrapper : position->get_annotations(typeid(match))) {
    const match&candidate_beginning = dynamic_cast<const match&>(static_cast<const annotation&>(beginning_wrapper));
    if (candidate_beginning.is_filled() && candidate_beginning.get_beginning() == position) {
      for (const ::production*production : productions) {
	for (unsigned slot_index : production->can_begin_with(candidate_beginning.get_result())) {
	  results.push_back(new potential_match{*production, slot_index + 1, candidate_beginning});
	}
      }
    }
  }
}

// Case IIb:  Beginning of a continuing match with a match already made.
static void begin_matches_with_match(vector<fact*>&results, typename ::session&session, const match&partial_match, token_iterator position) {
  unordered_set<const production*>candidates;
  for (const parseme*alternative : partial_match.get_continuing_alternatives()) {
    const unordered_set<const production*>&additional_candidates = session.get_productions_resulting_in(alternative);
    candidates.insert(additional_candidates.begin(), additional_candidates.end());
  }
  begin_matches_with_match(results, candidates, position);
}

// Case IIIa: Continuation of a match with a token.
static void continue_matches_with_token(vector<fact*>&results, token_iterator previous_position, token_iterator next_position, bool assume_next_token_is_justified = false) {
  for (const annotation_wrapper&wrapper : previous_position->get_annotations(typeid(match))) {
    const match&candidate_prefix = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
    if (candidate_prefix.can_continue_with(next_position, assume_next_token_is_justified)) {
      results.push_back(new potential_match{candidate_prefix, next_position, assume_next_token_is_justified});
    }
  }
}

// Case IIIb: Continuation of a match with another match.
static void continue_matches_with_match(vector<fact*>&results, const match&candidate_prefix, token_iterator next_position, bool assume_next_token_is_justified = false) {
  for (const annotation_wrapper&addendum_wrapper : next_position->get_annotations(typeid(match))) {
    const match&candidate_addendum = dynamic_cast<const match&>(static_cast<const annotation&>(addendum_wrapper));
    if (candidate_prefix.can_continue_with(candidate_addendum, assume_next_token_is_justified)) {
      results.push_back(new potential_match{candidate_prefix, candidate_addendum, assume_next_token_is_justified});
    }
  }
}

// Case IIIb: Continuation of a match with another match.
static void continue_matches_with_match(vector<fact*>&results, token_iterator previous_position, const match&candidate_addendum, bool assume_next_token_is_justified = false) {
  for (const annotation_wrapper&beginning_wrapper : previous_position->get_annotations(typeid(match))) {
    const match&candidate_prefix = dynamic_cast<const match&>(static_cast<const annotation&>(beginning_wrapper));
    if (candidate_prefix.can_continue_with(candidate_addendum, assume_next_token_is_justified)) {
      results.push_back(new potential_match{candidate_prefix, candidate_addendum});
    }
  }
}

// Case IIIb: Continuation of a match with another match.
static void continue_matches_with_match(vector<fact*>&results, token_iterator previous_position, token_iterator next_position, bool assume_next_token_is_justified = false) {
  for (const annotation_wrapper&prefix_wrapper : previous_position->get_annotations(typeid(match))) {
    const match&candidate_prefix = dynamic_cast<const match&>(static_cast<const annotation&>(prefix_wrapper));
    if (candidate_prefix.get_inclusive_end() == previous_position) {
      continue_matches_with_match(results, candidate_prefix, next_position, assume_next_token_is_justified);
    }
  }
}

token_available::token_available(typename ::session&session, ::buffer*buffer, token_iterator self) :
  negative_annotation_fact{session},
  buffer{buffer},
  self{self} {
  assert(self.can_increment());
}

token_available::token_available(typename ::session&session, token_iterator self) :
  negative_annotation_fact{session},
  buffer(nullptr),
  self{self} {
  assert(self.can_increment());
}

bool token_available::is_equal_to_instance_of_like_class(const base_class&other) const {
  const token_available&cast = dynamic_cast<const token_available&>(other);
  return self == cast.self;
}

vector<const fact_annotatable*>token_available::get_annotatables() const {
  return {&*self};
}

void token_available::justification_hook() const {
  assert(buffer);
  if (!operator bool()) {
    buffer->add_terminal_beginning(self);
  }
  negative_annotation_fact::justification_hook();
}

void token_available::unjustification_hook() const {
  assert(buffer);
  negative_annotation_fact::unjustification_hook();
  if (!operator bool()) {
    buffer->remove_terminal_beginning(self);
  }
}

std::vector<fact*>token_available::get_immediate_consequences() const {
  assert(buffer);
  typename ::session&session = dynamic_cast<typename ::session&>(context);
  token_iterator previous = ::previous(self);
  vector<fact*>results;
  if (previous != self) {
    if (!previous.can_increment() || end_of_sentence{session, previous, true}) {
      // Case Ia: Beginning of a match at the beginning of a sentence with a token.
      // (sentence ending) handled by the conditional just above.
      // (next) handled by the call to ::previous and the check on its return value.
      // (sentence/passage --> production) handled by the call to get_sentence_beginnings() below.
      // (token available) is the trigger.
      begin_matches_with_token(results, session, session.get_sentence_beginnings(), buffer, self);
    }
    if (previous.can_increment()) {
      // Case IIa: Beginning of a continuing match with a token.
      // (end of partial match) handled by the call to get_continuing_beginnings(...) below.
      // (next) handled by the call to ::previous and the check on its return value.
      // (need --> production) handled by the call to get_continuing_beginnings(...) below.
      // (token available) is the trigger.
      begin_matches_with_token(results, session, session.get_continuing_beginnings(previous), buffer, self);      
      // Case IIIa: Continuation of a match with a token.
      // (end of partial match) handled by the call to continue_matches_with_token(...) below.
      // (next) handled by the call to ::previous and the check on its return value.
      // (token available) is the trigger.
      continue_matches_with_token(results, previous, self);
    }
  }
  return results;
}

ostream&token_available::print(ostream&out) const {
  return out << "available(" << *self << ")";
}

const base_class*token_available::clone() const {
  return new token_available{dynamic_cast<typename ::session&>(context), buffer, self};
}

size_t token_available::hash() const {
  return reinterpret_cast<size_t>(&*self);
}

next_token::next_token(typename ::session&session, ::buffer*buffer, token_iterator self, token_iterator next) :
  annotation_fact{session},
  buffer{buffer},
  self{self},
  next{next} {
  assert(self.can_increment() || next.can_increment());
  assert(self != next);
}

next_token::next_token(typename ::session&session, token_iterator self, token_iterator next) :
  annotation_fact{session},
  buffer{nullptr},
  self{self},
  next{next} {
  assert(self.can_increment() || next.can_increment());
  assert(self != next);
}

bool next_token::is_equal_to_instance_of_like_class(const base_class&other) const {
  const next_token&cast = dynamic_cast<const next_token&>(other);
  return (self == cast.self) && (next == cast.next);
}

vector<const fact_annotatable*>next_token::get_annotatables() const {
  if (self.can_increment()) {
    if (next.can_increment()) {
      return {&*self, &*next};
    }
    return {&*self};
  }
  assert(next.can_increment());
  return {&*next};
}

vector<fact*>next_token::get_immediate_consequences() const {
  assert(buffer);
  typename ::session&session = dynamic_cast<typename ::session&>(context);
  vector<fact*>results;
  if (next.can_increment() && token_available{session, next}) {
    if (!self.can_increment() || end_of_sentence{session, self, true}) {
      // Case Ia: Beginning of a match at the beginning of a sentence with a token.
      // (sentence ending) handled by the conditional just above.
      // (next) is the trigger.
      // (sentence/passage --> production) handled by the call to get_sentence_beginnings() below.
      // (token available) handled by the outer conditional.
      begin_matches_with_token(results, session, session.get_sentence_beginnings(), buffer, next);
      // Case Ib: Beginning of a match at the beginning of a sentence with a match already made.
      // (sentence ending) handled by the conditional just above.
      // (next) is the trigger.
      // (sentence/passage --> production) handled by the call to get_sentence_beginnings() below.
      // (complete match) handled in begin_matches_with_match(...).
      begin_matches_with_match(results, session.get_sentence_beginnings(), next);
    }
    if (self.can_increment()) {
      // Case IIa: Beginning of a continuing match with a token.
      // (end of partial match) handled by the call to get_continuing_beginnings(...) below.
      // (next) is the trigger.
      // (need --> production) handled by the call to get_continuing_beginnings(...) below.
      // (token available) handled by the outer conditional.
      begin_matches_with_token(results, session, session.get_continuing_beginnings(self), buffer, next);
      // Case IIb:  Beginning of a continuing match with a match already made.
      // (end of partial match) handled by the call to get_continuing_beginnings(...) below.
      // (next) is the trigger.
      // (need --> production) handled by the call to get_continuing_beginnings(...) below.
      // (complete match) handled in begin_matches_with_match(...).
      begin_matches_with_match(results, session.get_continuing_beginnings(self), next);
      // Case IIIa: Continuation of a match with a token.
      // (end of partial match) handled by the call to continue_matches_with_token(...) below.
      // (next) is the trigger.
      // (token available) handled by the outer conditional.
      continue_matches_with_token(results, self, next, true);
      // Case IIIb: Continuation of a match with another match.
      // (end of partial match) handled by the call to continue_matches_with_match(...) below.
      // (next) is the trigger.
      // (complete match) handled by the call to continue_matches_with_match(...) below.
      continue_matches_with_match(results, self, next, true);
    }
  }
  return results;
}

ostream&next_token::print(ostream&out) const {
  out << "next(";
  if (self.can_increment()) {
    out << *self;
  } else {
    out << "<>";
  }
  out << ", ";
  if (next.can_increment()) {
    out << *next;
  } else {
    out << "<>";
  }
  return out << ")";
}

token_iterator next_token::get_self() const {
  return self;
}

token_iterator next_token::get_next() const {
  return next;
}

const base_class*next_token::clone() const {
  return new next_token{dynamic_cast<typename ::session&>(context), buffer, self, next};
}

size_t next_token::hash() const {
  return self.can_increment() ? reinterpret_cast<size_t>(&*self) : reinterpret_cast<size_t>(&*next) - 1;
}

// This function is saturating, and acts as the identity if the argument is the
// end of the sequence.  This is to prevent accidental pointer chasing from the
// end of a source text to its beginnning.
token_iterator previous(const token_iterator&iterator) {
  if (iterator.can_increment()) {
    for (const annotation_wrapper&wrapper : iterator->get_annotations(typeid(next_token))) {
      const next_token&link = dynamic_cast<const next_token&>(static_cast<const annotation&>(wrapper));
      if (link.get_next() == iterator) {
	return link.get_self();
      }
      assert(link.get_self() == iterator);
    }
  }
  return iterator;
}

// This function is saturating, and acts as the identity if the argument is the
// end of the sequence.  This is to prevent accidental pointer chasing from the
// beginnning of a source text to its end.
token_iterator next(const token_iterator&iterator) {
  if (iterator.can_increment()) {
    for (const annotation_wrapper&wrapper : iterator->get_annotations(typeid(next_token))) {
      const next_token&link = dynamic_cast<const next_token&>(static_cast<const annotation&>(wrapper));
      if (link.get_self() == iterator) {
	return link.get_next();
      }
      assert(link.get_next() == iterator);
    }
  }
  return iterator;
}

end_of_unit::end_of_unit(typename ::session&session, token_iterator self, bool in_the_positive_sense) :
  combined_annotation_fact{session, in_the_positive_sense},
  self{self} {
  assert(self.can_increment());
}

bool end_of_unit::is_equal_to_instance_of_like_class(const base_class&other) const {
  const end_of_unit&cast = dynamic_cast<const end_of_unit&>(other);
  return
    (in_the_positive_sense == cast.in_the_positive_sense) &&
    (self == cast.self);
}

vector<const fact_annotatable*>end_of_unit::get_annotatables() const {
  return {&*self};
}

token_iterator end_of_unit::get_self() const {
  return self;
}

size_t end_of_unit::hash() const {
  return reinterpret_cast<size_t>(&*self);
}

end_of_sentence::end_of_sentence(typename ::session&session, ::buffer*buffer, token_iterator self, bool in_the_positive_sense) :
  end_of_unit{session, self, in_the_positive_sense},
  buffer{buffer} {}

end_of_sentence::end_of_sentence(typename ::session&session, token_iterator self, bool in_the_positive_sense) :
  end_of_unit{session, self, in_the_positive_sense},
  buffer{nullptr} {}

void end_of_sentence::justification_hook() const {
  assert(buffer);
  end_of_unit::justification_hook();
  buffer->add_sentence_ending(self);
}

void end_of_sentence::unjustification_hook() const {
  assert(buffer);
  buffer->remove_sentence_ending(self);
  end_of_unit::unjustification_hook();
}

std::vector<fact*>end_of_sentence::get_immediate_consequences() const {
  typename ::session&session = dynamic_cast<typename ::session&>(context);
  vector<fact*>results;
  if (in_the_positive_sense) {
    token_iterator next = ::next(self);
    if ((next != self) && next.can_increment() && token_available{session, next}) {
      // Case Ia: Beginning of a match at the beginning of a sentence with a token.
      // (sentence ending) is the trigger.
      // (next) handled by the call to ::next and the check on its return value.
      // (sentence/passage --> production) handled by the call to get_sentence_beginnings() below.
      // (token available) handled by the conditional just above.
      begin_matches_with_token(results, session, session.get_sentence_beginnings(), buffer, next);
      // Case Ib: Beginning of a match at the beginning of a sentence with a match already made.
      // (sentence ending) is the trigger.
      // (next) handled by the call to ::next and the check on its return value.
      // (sentence/passage --> production) handled by the call to get_sentence_beginnings() below.
      // (complete match) handled in begin_matches_with_match(...).
      begin_matches_with_match(results, session.get_sentence_beginnings(), next);
    }
  }
  // Case IV: Conditions 0 and 1 are enforced by guards calling can_reach_slot_count_at(...).
  for (const annotation_wrapper&wrapper : self->get_annotations(typeid(potential_match))) {
    const potential_match&candidate_match = dynamic_cast<const potential_match&>(static_cast<const annotation&>(wrapper));
    if (candidate_match.get_inclusive_end() == self && candidate_match.get_production().can_reach_slot_count_at(candidate_match.get_slots_filled(), self, in_the_positive_sense)) {
      results.push_back(new match{candidate_match});
    }
  }
  return results;
}

ostream&end_of_sentence::print(ostream&out) const {
  return out << (in_the_positive_sense ? "end_of_sentence(" : "not_end_of_sentence(") << &(*self) << " = " << *self << ")";
}

const base_class*end_of_sentence::clone() const {
  return new end_of_sentence{dynamic_cast<typename ::session&>(context), buffer, self, in_the_positive_sense};
}

bool parseme::accepts(const token_iterator&iterator) const {
  return false;
}

bool parseme::accepts(const parseme&other) const {
  return operator ==(other);
}

bool epsilon_terminal::accepts(const token_iterator&iterator) const {
  return false;
}

const base_class*epsilon_terminal::clone() const {
  return new epsilon_terminal{};
}

bool digits_terminal::accepts(const token_iterator&iterator) const {
  for (i7_codepoint codepoint : *iterator->get_text()) {
    if (!is_i7_digit(codepoint)) {
      return false;
    }
  }
  return true;
}

const base_class*digits_terminal::clone() const {
  return new digits_terminal{};
}

bool word_terminal::accepts(const token_iterator&iterator) const {
  return true;
}

const base_class*word_terminal::clone() const {
  return new word_terminal{};
}

bool name_word_terminal::accepts(const token_iterator&iterator) const {
  switch ((*iterator->get_text())[0]) {
  case ',':
  case '"':
  case '(':
  case ')':
    return false;
  }
  return true;
}

const base_class*name_word_terminal::clone() const {
  return new name_word_terminal{};
}

token_terminal::token_terminal(const i7_string&text) :
  text{&vocabulary.acquire(text)} {}

token_terminal::~token_terminal() {
  vocabulary.release(*text);
}

bool token_terminal::is_equal_to_instance_of_like_class(const base_class&other) const {
  const token_terminal&cast = dynamic_cast<const token_terminal&>(other);
  return text == cast.text;
}

const i7_string&token_terminal::get_text() const {
  return *text;
}

bool token_terminal::accepts(const token_iterator&iterator) const {
  return iterator->get_text() == text;
}

const base_class*token_terminal::clone() const {
  return new token_terminal{*text};
}

size_t token_terminal::hash() const {
  return reinterpret_cast<size_t>(text);
}

nonterminal::nonterminal(const i7_string&kind_name, unsigned tier) :
  kind_name{&vocabulary.acquire(kind_name)},
  tier{tier} {}

nonterminal::nonterminal(const nonterminal&copy) :
  kind_name{&vocabulary.acquire(*copy.kind_name)},
  tier{copy.tier} {}

nonterminal::nonterminal(const nonterminal&copy, unsigned replacement_tier) :
  kind_name{&vocabulary.acquire(*copy.kind_name)},
  tier{replacement_tier} {}

nonterminal::~nonterminal() {
  vocabulary.release(*kind_name);
}

bool nonterminal::is_equal_to_instance_of_like_class(const base_class&other) const {
  const nonterminal&cast = dynamic_cast<const nonterminal&>(other);
  return (kind_name == cast.kind_name) && (tier == cast.tier);
}

const i7_string&nonterminal::get_kind_name() const {
  return *kind_name;
}

unsigned nonterminal::get_tier() const {
  return tier;
}

bool nonterminal::accepts(const parseme&other) const {
  const nonterminal*cast = dynamic_cast<const nonterminal*>(&other);
  return cast && (kind_name == cast->kind_name) && (tier >= cast->tier);
}

const base_class*nonterminal::clone() const {
  return new nonterminal{*kind_name, tier};
}

size_t nonterminal::hash() const {
  return reinterpret_cast<size_t>(kind_name) + tier;
}

production::production(typename ::session&session, const nonterminal&result, bool internal) :
  fact{session},
  result{dynamic_cast<const nonterminal*>(&parseme_bank.acquire(result))},
  epsilon_prefix_length{0},
  hash_value{0},
  internal{internal} {}

production::production(const production&copy) :
  fact{dynamic_cast<typename ::session&>(copy.context)},
  result{dynamic_cast<const nonterminal*>(&parseme_bank.acquire(*copy.result))},
  alternatives_sequence{copy.alternatives_sequence},
  epsilon_prefix_length{copy.epsilon_prefix_length},
  beginnings{copy.beginnings},
  hash_value{copy.hash_value},
  internal{copy.internal} {
  for (const vector<const parseme*>&alternatives : alternatives_sequence) {
    for (const parseme*alternative : alternatives) {
      const parseme*internalization = &parseme_bank.acquire(*alternative);
      assert(internalization == alternative);
      (void)internalization;
    }
  }
}

production::~production() {
  parseme_bank.release(*result);
  for (const vector<const parseme*>&alternatives : alternatives_sequence) {
    for (const parseme*alternative : alternatives) {
      parseme_bank.release(*alternative);
    }
  }
}

bool production::is_equal_to_instance_of_like_class(const base_class&other) const {
  const production&cast = dynamic_cast<const production&>(other);
  return
    (hash_value == cast.hash_value) && // for early rejection
    (result == cast.result) &&
    (alternatives_sequence == cast.alternatives_sequence);
}

std::vector<fact*>production::get_immediate_consequences() const {
  typename ::session&session = dynamic_cast<typename ::session&>(context);
  bool can_begin_sentence = this->can_begin_sentence();
  vector<fact*>results;
  for (const auto&i : session.get_buffers()) {
    ::buffer*buffer = i.second;
    if (can_begin_sentence) {
      unordered_set<const production*>affected_sentence_beginnings = session.get_sentence_beginnings_relying_on(*this);
      for (token_iterator previous : buffer->get_sentence_endings()) {
	token_iterator next = ::next(previous);
	if (next.can_increment() && next != previous && token_available{session, next}) {
	  // Case Ia: Beginning of a match at the beginning of a sentence with a token.
	  // (sentence ending) handled by the two surrounding loops.
	  // (next) handled by the call to ::next and the check on its return value.
	  // (sentence/passage --> production) is the trigger, with the various subcases covered by get_sentence_beginnings_relying_on(...).
	  // (token available) handled by the conditional just above.
	  begin_matches_with_token(results, session, affected_sentence_beginnings, buffer, next);
	  // Case Ib: Beginning of a match at the beginning of a sentence with a match already made.
	  // (sentence ending) handled by the two surrounding loops.
	  // (next) handled by the call to ::next and the check on its return value.
	  // (sentence/passage --> production) is the trigger, with the various subcases covered by get_sentence_beginnings_relying_on(...).
	  // (complete match) handled in begin_matches_with_match(...).
	  begin_matches_with_match(results, affected_sentence_beginnings, next);
	}
      }
    }
    unordered_set<const production*>affected_result_beginnings = session.get_result_beginnings_relying_on(*this);
    for (const match*prefix : buffer->get_partial_matches_needing(result)) {
      token_iterator previous = prefix->get_inclusive_end();
      token_iterator next = ::next(previous);
      if (next.can_increment() && next != previous && token_available{session, next}) {
	// Case IIa: Beginning of a continuing match with a token.
	// (end of partial match) handled by get_partial_matches_needing(...).  (It only needs to be called on the topmost result.)
	// (next) handled by the call to ::next and the check on its return value.
	// (need --> production) is the trigger, with the various subcases covered by get_result_beginnings_relying_on(...).
	// (token available) handled by the conditional just above.
	begin_matches_with_token(results, session, affected_result_beginnings, buffer, next);
	// Case IIb:  Beginning of a continuing match with a match already made.
	// (end of partial match) handled by get_partial_matches_needing(...).  (It only needs to be called on the topmost result.)
	// (next) handled by the call to ::next and the check on its return value.
	// (need --> production) is the trigger, with the various subcases covered by get_result_beginnings_relying_on(...).
	// (complete match) handled in begin_matches_with_match(...).
	begin_matches_with_match(results, affected_result_beginnings, next);
      }
    }
  }
  return results;
}

void production::add_slot() {
  assert(alternatives_sequence.empty() || alternatives_sequence.back().size());
  alternatives_sequence.push_back({});
  hash_value <<= 1;
}

void production::add_alternative_to_last_slot(const parseme&alternative) {
  assert(alternatives_sequence.size());
  const parseme*internalization = &parseme_bank.acquire(alternative);
  alternatives_sequence.back().push_back(internalization);
  if (epsilon_prefix_length + 1 >= alternatives_sequence.size()) {
    if (dynamic_cast<const epsilon_terminal*>(internalization)) {
      epsilon_prefix_length = alternatives_sequence.size();
    } else {
      beginnings.push_back({static_cast<unsigned>(alternatives_sequence.size() - 1), internalization});
    }
  }
  hash_value += reinterpret_cast<size_t>(internalization);
}

const nonterminal*production::get_result() const {
  return result;
}

unsigned production::get_slot_count() const {
  return alternatives_sequence.size();
}

const std::vector<production::possible_beginning>&production::get_beginnings() const {
  return beginnings;
}

const vector<const parseme*>&production::get_alternatives(unsigned slot_index) const {
  return alternatives_sequence[slot_index];
}

bool production::accepts(unsigned slot_index, const ::token_iterator&iterator) const {
  assert(slot_index < alternatives_sequence.size());
  for (const ::parseme*alternative : alternatives_sequence[slot_index]) {
    if (alternative->accepts(iterator)) {
      return true;
    }
  }
  return false;
}

bool production::accepts(unsigned slot_index, const ::parseme&parseme) const {
  assert(slot_index < alternatives_sequence.size());
  for (const ::parseme*alternative : alternatives_sequence[slot_index]) {
    if (alternative->accepts(parseme)) {
      return true;
    }
  }
  return false;
}

std::vector<unsigned>production::can_begin_with(const ::token_iterator&iterator) const {
  std::vector<unsigned>results;
  for (const possible_beginning&beginning : beginnings) {
    if (beginning.parseme->accepts(iterator)) {
      results.push_back(beginning.epsilon_count);
    }
  }
  return results;
}

std::vector<unsigned>production::can_begin_with(const ::parseme&parseme) const {
  std::vector<unsigned>results;
  for (const possible_beginning&beginning : beginnings) {
    if (beginning.parseme->accepts(parseme)) {
      results.push_back(beginning.epsilon_count);
    }
  }
  return results;
}

bool production::can_reach_slot_count_at(unsigned slot_count, token_iterator position) const {
  return can_reach_slot_count_at(slot_count, position, !position.can_increment() || end_of_sentence{dynamic_cast<typename ::session&>(context), position, true});
}

size_t production::hash() const {
  return hash_value;
}

subsentence::subsentence(typename ::session&session, const nonterminal&result, bool internal) :
  production{session, result, internal} {}

subsentence::subsentence(const subsentence&copy) :
  production{copy} {}

bool subsentence::can_begin_sentence() const {
  typename ::session&session = dynamic_cast<typename ::session&>(context);
  return session.can_begin_sentence_with(this);
}

void subsentence::justification_hook() const {
  dynamic_cast<typename ::session&>(context).add_subsentence(*this);
}

void subsentence::unjustification_hook() const {
  dynamic_cast<typename ::session&>(context).remove_subsentence(*this);
}

ostream&subsentence::print(ostream&out) const {
  return out << "subsentence(" << ASSUME_EIGHT_BIT(result->get_kind_name()) << "...)";
}

subsentence::operator bool() const {
  const unordered_set<const subsentence*>&subsentences = dynamic_cast<typename ::session&>(context).get_subsentences();
  return subsentences.find(dynamic_cast<const subsentence*>(production_bank.lookup(*this))) != subsentences.end();
}

bool subsentence::can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const {
  return (slot_count == alternatives_sequence.size()) || !assumed_end_of_sentence_state;
}

const base_class*subsentence::clone() const {
  return new subsentence{*this};
}

wording::wording(typename ::session&session, const nonterminal&result, bool internal) :
  production{session, result, internal} {}

wording::wording(const wording&copy) :
  production{copy} {}

bool wording::can_begin_sentence() const {
  typename ::session&session = dynamic_cast<typename ::session&>(context);
  return session.can_begin_sentence_with(this);
}

void wording::justification_hook() const {
  dynamic_cast<typename ::session&>(context).add_wording(*this);
}

void wording::unjustification_hook() const {
  dynamic_cast<typename ::session&>(context).remove_wording(*this);
}

ostream&wording::print(ostream&out) const {
  return out << "wording(" << ASSUME_EIGHT_BIT(result->get_kind_name()) << "...)";
}

wording::operator bool() const {
  const unordered_set<const wording*>&wordings = dynamic_cast<typename ::session&>(context).get_wordings();
  return wordings.find(dynamic_cast<const wording*>(production_bank.lookup(*this))) != wordings.end();
}

bool wording::can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const {
  if ((slot_count == 1 || slot_count == alternatives_sequence.size()) &&
      !is_in_plain_i7_or_documentation(position)) {
    return false;
  }
  return (slot_count == alternatives_sequence.size()) || !assumed_end_of_sentence_state;
}

const base_class*wording::clone() const {
  return new wording{*this};
}

sentence::sentence(typename ::session&session, const nonterminal&result, bool internal) :
  production{session, result, internal} {}

sentence::sentence(const sentence&copy) :
  production{copy} {}

bool sentence::can_begin_sentence() const {
  return true;
}

void sentence::justification_hook() const {
  dynamic_cast<typename ::session&>(context).add_sentence(*this);
}

void sentence::unjustification_hook() const {
  dynamic_cast<typename ::session&>(context).remove_sentence(*this);
}

ostream&sentence::print(ostream&out) const {
  return out << "sentence(" << ASSUME_EIGHT_BIT(result->get_kind_name()) << "...)";
}

sentence::operator bool() const {
  const unordered_set<const sentence*>&sentences = dynamic_cast<typename ::session&>(context).get_sentences();
  return sentences.find(dynamic_cast<const sentence*>(production_bank.lookup(*this))) != sentences.end();
}

bool sentence::can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const {
  if ((slot_count == 1 || slot_count == alternatives_sequence.size()) &&
      !is_in_plain_i7_or_documentation(position)) {
    return false;
  }
  return (slot_count == alternatives_sequence.size()) == assumed_end_of_sentence_state;
}

const base_class*sentence::clone() const {
  return new sentence{*this};
}

passage::passage(typename ::session&session, const nonterminal&result, bool internal) :
  production{session, result, internal} {}

passage::passage(const passage&copy) :
  production{copy} {}

bool passage::can_begin_sentence() const {
  return true;
}

void passage::justification_hook() const {
  dynamic_cast<typename ::session&>(context).add_passage(*this);
}

void passage::unjustification_hook() const {
  dynamic_cast<typename ::session&>(context).remove_passage(*this);
}

ostream&passage::print(ostream&out) const {
  return out << "passage(" << ASSUME_EIGHT_BIT(result->get_kind_name()) << "...)";
}

passage::operator bool() const {
  const unordered_set<const passage*>&passages = dynamic_cast<typename ::session&>(context).get_passages();
  return passages.find(dynamic_cast<const passage*>(production_bank.lookup(*this))) != passages.end();
}

bool passage::can_reach_slot_count_at(unsigned slot_count, token_iterator position, bool assumed_end_of_sentence_state) const {
  if ((slot_count == 1 || slot_count == alternatives_sequence.size()) &&
      !is_in_plain_i7_or_documentation(position)) {
    return false;
  }
  if (slot_count == alternatives_sequence.size()) {
    return assumed_end_of_sentence_state;
  }
  return true;
}

const base_class*passage::clone() const {
  return new passage{*this};
}

potential_match::potential_match(typename ::session&session, ::buffer*buffer, const ::production&production, unsigned slots_filled, const token_iterator beginning, const token_iterator inclusive_end) :
  annotation_fact{session},
  buffer{buffer},
  production{&production_bank.acquire(production)},
  slots_filled{slots_filled},
  beginning{beginning},
  inclusive_end{inclusive_end} {
  assert(production);
}

potential_match::potential_match(typename ::session&session, ::buffer*buffer, const ::production&production, unsigned slots_filled, token_iterator beginning) :
  annotation_fact{session},
  buffer{buffer},
  production{&production_bank.acquire(production)},
  slots_filled{slots_filled},
  beginning{beginning},
  inclusive_end{beginning} {
  assert(production);
  assert(production.can_begin_with(beginning).size());
}

potential_match::potential_match(const potential_match&copy) :
  annotation_fact{copy.context},
  buffer{copy.buffer},
  production{&production_bank.acquire(*copy.production)},
  slots_filled{copy.slots_filled},
  beginning{copy.beginning},
  inclusive_end{copy.inclusive_end} {}

potential_match::~potential_match() {
  production_bank.release(*production);
}

potential_match::potential_match(const potential_match&prefix, token_iterator inclusive_end, bool assume_next_token_is_justified) :
  annotation_fact{prefix.context},
  buffer{prefix.buffer},
  production{&production_bank.acquire(*prefix.production)},
  slots_filled{prefix.slots_filled + 1},
  beginning{prefix.beginning},
  inclusive_end{inclusive_end} {
  assert(beginning != inclusive_end);
  assert(assume_next_token_is_justified || prefix.inclusive_end == inclusive_end || next(prefix.inclusive_end) == inclusive_end);
}

potential_match::potential_match(const ::production&production, unsigned slots_filled, const potential_match&addendum) :
  annotation_fact{addendum.context},
  buffer{addendum.buffer},
  production{&production_bank.acquire(production)},
  slots_filled{slots_filled},
  beginning{addendum.beginning},
  inclusive_end{addendum.inclusive_end} {
  assert(production);
  assert(production.can_begin_with(addendum.get_result()).size());
}

potential_match::potential_match(const potential_match&prefix, const potential_match&addendum, bool assume_next_token_is_justified) :
  annotation_fact{prefix.context},
  buffer{prefix.buffer},
  production{&production_bank.acquire(*prefix.production)},
  slots_filled{prefix.slots_filled + 1},
  beginning{prefix.beginning},
  inclusive_end{addendum.inclusive_end} {
  assert(&prefix.context == &addendum.context);
  assert(prefix.buffer == addendum.buffer);
  assert(prefix.inclusive_end != addendum.beginning);
  assert(assume_next_token_is_justified || next(prefix.inclusive_end) == addendum.beginning);
}

bool potential_match::is_equal_to_instance_of_like_class(const base_class&other) const {
  const potential_match&cast = dynamic_cast<const potential_match&>(other);
  return
    (slots_filled == cast.slots_filled) &&
    (beginning == cast.beginning) &&
    (inclusive_end == cast.inclusive_end) &&
    (production == cast.production); // last because this is most expensive
}

vector<const fact_annotatable*>potential_match::get_annotatables() const {
  if (beginning == inclusive_end) {
    return {&*beginning};
  }
  return {&*beginning, &*inclusive_end};
}

vector<fact*>potential_match::get_immediate_consequences() const {
  vector<fact*>results;
  if (production->can_reach_slot_count_at(slots_filled, inclusive_end)) {
    // Case IV: Conditions 0 and 1 are enforced by guards calling can_reach_slot_count_at(...).
    results.push_back(new match{*this});
  }
  return results;
}

ostream&potential_match::print(ostream&out) const {
  out << "potential_match(" << *production << ", " << slots_filled << "/" << production->get_slot_count() << ", ";
  if (beginning.can_increment()) {
    out << *beginning;
  } else {
    out << "<>";
  }
  out << " ... ";
  if (inclusive_end.can_increment()) {
    out << *inclusive_end;
  } else {
    out << "<>";
  }
  return out << ")" << flush;
}

const nonterminal&potential_match::get_result() const {
  return *(production->get_result());
}

const production&potential_match::get_production() const {
  return *production;
}

unsigned potential_match::get_slots_filled() const {
  return slots_filled;
}

token_iterator potential_match::get_beginning() const {
  return beginning;
}

token_iterator potential_match::get_inclusive_end() const {
  return inclusive_end;
}

bool potential_match::is_filled() const {
  return slots_filled == production->get_slot_count();
}

bool potential_match::is_complete() const {
  if (is_filled() && (*this)) {
    for (const annotation_wrapper&wrapper : inclusive_end->get_annotations(typeid(match))) {
      const match&candidate_completion = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
      if (is_equal_to_instance_of_like_class(candidate_completion)) {
	return true;
      }
    }
  }
  return false;
}

bool potential_match::can_continue_with(token_iterator end, bool assume_next_token_is_justified) const {
  return
    !is_filled() &&
    end.can_increment() &&
    production->accepts(slots_filled, token_terminal{*end->get_text()}) &&
    (end != inclusive_end) &&
    (assume_next_token_is_justified || end == next(inclusive_end));
}

bool potential_match::can_continue_with(const potential_match&addendum, bool assume_next_token_is_justified) const {
  assert(&context == &addendum.context);
  return
    !is_filled() &&
    addendum.is_filled() &&
    production->accepts(slots_filled, *addendum.production->get_result()) &&
    (inclusive_end != addendum.beginning) &&
    (assume_next_token_is_justified || next(inclusive_end) == addendum.beginning);
}

const vector<const parseme*>&potential_match::get_continuing_alternatives() const {
  return production->get_alternatives(slots_filled);
}

const base_class*potential_match::clone() const {
  return new potential_match{*this};
}

size_t potential_match::hash() const {
  return reinterpret_cast<size_t>(production) + reinterpret_cast<size_t>(&*beginning) + reinterpret_cast<size_t>(&*inclusive_end) + slots_filled;
}

match::match(const potential_match&copy) :
  potential_match{copy} {}

void match::justification_hook() const {
  assert(buffer);
  annotation_fact::justification_hook();
  if (!operator bool()) {
    buffer->add_parseme_beginning(*production->get_result(), beginning);
    if (!is_filled()) {
      buffer->add_partial_match(dynamic_cast<const match*>(beginning->get_annotation(*this)));
    }
  }
}

void match::unjustification_hook() const {
  assert(buffer);
  if (!operator bool()) {
    buffer->remove_parseme_beginning(*production->get_result(), beginning);
    if (!is_filled()) {
      buffer->remove_partial_match(dynamic_cast<const match*>(beginning->get_annotation(*this)));
    }
  }
  annotation_fact::unjustification_hook();
}

vector<fact*>match::get_immediate_consequences() const {
  typename ::session&session = dynamic_cast<typename ::session&>(context);
  token_iterator previous = ::previous(beginning);
  vector<fact*>results;
  if (is_filled()) {
    if (previous != beginning && (!previous.can_increment() || end_of_sentence{session, previous, true})) {
      intersection<const ::production*>candidates{session.get_sentence_beginnings(), session.get_productions_beginning_with(&get_result())};
      // Case Ib: Beginning of a match at the beginning of a sentence with a match already made.
      // (sentence ending) handled by the conditional just above.
      // (next) handled by the call to ::previous and the check on its return value.
      // (sentence/passage --> production) handled by the intersection computed just above.
      // (complete match) is the trigger.
      begin_matches_with_match(results, candidates, *this);
    }
    if (previous != beginning && previous.can_increment()) {
      // Case IIb:  Beginning of a continuing match with a match already made.
      // (end of partial match) handled by the call to get_continuing_beginnings(...) below.
      // (next) handled by the call to ::previous and the check on its return value.
      // (need --> production) handled by the call to get_continuing_beginnings(...) below.
      // (complete match) is the trigger.
      begin_matches_with_match(results, session.get_continuing_beginnings(previous), *this);
      // Case IIIb: Continuation of a match with another match.
      // (end of partial match) handled by the call to continue_matches_with_match(...) below.
      // (next) handled by the call to ::previous and the check on its return value.
      // (complete match) is the trigger.
      continue_matches_with_match(results, previous, *this);
    }
  } else {
    token_iterator end = ::next(inclusive_end);
    if (end.can_increment() && end != inclusive_end && token_available{session, end}) {
      // Case IIa: Beginning of a continuing match with a token.
      // (end of partial match) is the trigger.
      // (next) handled by the call to ::next and the check on its return value.
      // (need --> production) handled by the call to get_continuing_beginnings(...) below.
      // (token available) handled by the conditional just above.
      begin_matches_with_token(results, session, session.get_continuing_beginnings(*this), buffer, end);
      // Case IIb:  Beginning of a continuing match with a match already made.
      // (end of partial match) is the trigger.
      // (next) handled by the call to ::next and the check on its return value.
      // (need --> production) handled by the call to begin_matches_with_match(...) below.
      // (complete match) handled by the call to begin_matches_with_match(...) below.
      begin_matches_with_match(results, session, *this, end);
      if (can_continue_with(end)) {
	// Case IIIa: Continuation of a match with a token.
	// (end of partial match) is the trigger.
	// (next) handled by the call to ::next and the check on its return value.
	// (token available) handled by the conditional just above.
	results.push_back(new potential_match{*this, end});
      }
      // Case IIIb: Continuation of a match with another match.
      // (end of partial match) is the trigger.
      // (next) handled by the call to ::next and the check on its return value.
      // (complete match) handled by the call to continue_matches_with_match(...) below.
      continue_matches_with_match(results, *this, end);
    }
    static epsilon_terminal epsilon;
    if (production->accepts(slots_filled, epsilon)) {
      // Case IIIc: Continuation of a match with an epsilon.
      // (end of partial match) is the trigger.
      results.push_back(new potential_match{*this, inclusive_end});
    }
  }
  return results;
}

ostream&match::print(ostream&out) const {
  return potential_match::print(out << "complete_");
}

bool match::is_complete() const {
  return is_filled() && *this;
}

const base_class*match::clone() const {
  return new match{*this};
}

internalizer<parseme>parseme_bank;
internalizer<production>production_bank;
