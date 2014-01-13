#include <cassert>

#include "type_indices.hpp"
#include "parser.hpp"
#include "internalizer.hpp"

using namespace std;

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

vector<const annotatable*>token_available::get_annotatables() const {
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
  vector<fact*>results;
  // Case I: The token begins a match
  const parseme*key = parseme_bank.lookup(token_terminal{*self->get_text()});
  if (key) {
    for (const ::production*production : dynamic_cast<typename ::session&>(context).get_productions(key)) {
      for (unsigned slot_index : production->can_begin_with(*self)) {
    	results.push_back(new match{dynamic_cast<typename ::session&>(context), buffer, *production, slot_index + 1, self});
      }
    }
  }
  // Case II: The token continues a match
  token_iterator previous = ::previous(self);
  if (previous != self) {
    for (const annotation_wrapper&wrapper : previous->get_annotations(match_type_index)) {
      const match&candidate_prefix = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
      if (candidate_prefix.can_continue_with(self)) {
	results.push_back(new match{candidate_prefix, self});
      }
    }
  }
  // Done
  return results;
}

base_class*token_available::clone() const {
  return new token_available{dynamic_cast<typename ::session&>(context), self};
}

size_t token_available::hash() const {
  return reinterpret_cast<size_t>(&*self);
}

next_token::next_token(typename ::session&session, token_iterator self, token_iterator next) :
  annotation_fact{session},
  self{self},
  next{next} {
  assert(self.can_increment());
  assert(self != next);
}

bool next_token::is_equal_to_instance_of_like_class(const base_class&other) const {
  const next_token&cast = dynamic_cast<const next_token&>(other);
  return (self == cast.self) && (next == cast.next);
}

vector<const annotatable*>next_token::get_annotatables() const {
  if (next.can_increment()) {
    return {&*self, &*next};
  }
  return {&*self};
}

vector<fact*>next_token::get_immediate_consequences() const {
  vector<fact*>results;
  for (const annotation_wrapper&wrapper : self->get_annotations(match_type_index)) {
    const match&candidate_prefix = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
    // Case I: The next token continues a match
    if (candidate_prefix.can_continue_with(next)) {
      results.push_back(new match{candidate_prefix, next});
    }
    // Case II: The next token begins a match that continues another
    for (const annotation_wrapper&next_wrapper : next->get_annotations(match_type_index)) {
      const match&candidate_addendum = dynamic_cast<const match&>(static_cast<const annotation&>(next_wrapper));
      if (candidate_prefix.can_continue_with(candidate_addendum)) {
	results.push_back(new match{candidate_prefix, candidate_addendum});
      }
    }
  }
  return results;
}

token_iterator next_token::get_self() const {
  return self;
}

token_iterator next_token::get_next() const {
  return next;
}

base_class*next_token::clone() const {
  return new next_token{dynamic_cast<typename ::session&>(context), self, next};
}

size_t next_token::hash() const {
  return reinterpret_cast<size_t>(&*self); // + reinterpret_cast<size_t>(&*next);
}

token_iterator previous(const token_iterator&iterator) {
  for (const annotation_wrapper&wrapper : iterator->get_annotations(next_token_type_index)) {
    const next_token&link = dynamic_cast<const next_token&>(static_cast<const annotation&>(wrapper));
    if (link.get_next() == iterator) {
      return link.get_self();
    }
    assert(link.get_self() == iterator);
  }
  return iterator;
}

token_iterator next(const token_iterator&iterator) {
  for (const annotation_wrapper&wrapper : iterator->get_annotations(next_token_type_index)) {
    const next_token&link = dynamic_cast<const next_token&>(static_cast<const annotation&>(wrapper));
    if (link.get_self() == iterator) {
      return link.get_next();
    }
    assert(link.get_next() == iterator);
  }
  return iterator;
}

bool parseme::accepts(const parseme&other) const {
  return operator ==(other);
}

bool epsilon_terminal::is_equal_to_instance_of_like_class(const base_class&other) const {
  return true;
}

bool epsilon_terminal::accepts(const token_iterator&iterator) const {
  return false;
}

base_class*epsilon_terminal::clone() const {
  return new epsilon_terminal{};
}

size_t epsilon_terminal::hash() const {
  return 0;
}

bool something_unrecognized_terminal::is_equal_to_instance_of_like_class(const base_class&other) const {
  return true;
}

bool something_unrecognized_terminal::accepts(const token_iterator&iterator) const {
  return true; // TODO: avoid non-I7, a colon preceding a newline, others?
}

bool something_unrecognized_terminal::accepts(const parseme&other) const {
  const token_terminal*cast = dynamic_cast<const token_terminal*>(&other);
  if (!cast) {
    return parseme::accepts(other);
  }
  return true;
}

base_class*something_unrecognized_terminal::clone() const {
  return new something_unrecognized_terminal{};
}

size_t something_unrecognized_terminal::hash() const {
  return 1;
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

base_class*token_terminal::clone() const {
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

bool nonterminal::accepts(const token_iterator&iterator) const {
  return false;
}

bool nonterminal::accepts(const parseme&other) const {
  const nonterminal*cast = dynamic_cast<const nonterminal*>(&other);
  return cast && (kind_name == cast->kind_name) && (tier >= cast->tier);
}

base_class*nonterminal::clone() const {
  return new nonterminal{*kind_name, tier};
}

size_t nonterminal::hash() const {
  return reinterpret_cast<size_t>(kind_name) + tier;
}

production::production(typename ::session&session, const nonterminal&result) :
  fact{session},
  result{result},
  epsilon_prefix_length{0},
  hash_value{0} {}

production::~production() {
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

void production::justification_hook() const {
  dynamic_cast<typename ::session&>(context).add_production(*this);
}

void production::unjustification_hook() const {
  dynamic_cast<typename ::session&>(context).remove_production(*this);
}

vector<fact*>production::get_immediate_consequences() const {
  vector<fact*>results;
  // Case I: A token begins a match or a match begins another match
  for (const auto&buffer_mapping : dynamic_cast<typename ::session&>(context).get_buffers()) {
    for (const possible_beginning&beginning : beginnings) {
      for (token_iterator position : buffer_mapping.second->get_parseme_beginnings(*beginning.parseme)) {
	results.push_back(new match{dynamic_cast<typename ::session&>(context), buffer_mapping.second, *this, beginning.epsilon_count + 1, position});
      }
    }
  }
  // Done
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
  if (epsilon_prefix_length >= alternatives_sequence.size()) {
    if (dynamic_cast<const epsilon_terminal*>(internalization)) {
      epsilon_prefix_length = alternatives_sequence.size();
    } else {
      beginnings.push_back({static_cast<unsigned>(alternatives_sequence.size() - 1), internalization});
    }
  }
  hash_value += reinterpret_cast<size_t>(internalization);
}

const nonterminal&production::get_result() const {
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

bool production::accepts(unsigned slot_index, const ::parseme&parseme) const {
  assert(slot_index < alternatives_sequence.size());
  for (const ::parseme*alternative : alternatives_sequence[slot_index]) {
    if (alternative->accepts(parseme)) {
      return true;
    }
  }
  return false;
}

std::vector<unsigned>production::can_begin_with(const ::token&token) const {
  std::vector<unsigned>results;
  if (beginnings.size()) { // for early rejection
    return {};
  }
  return can_begin_with(token_terminal{*token.get_text()});
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

production::operator bool() const {
  const unordered_set<const production*>&productions = dynamic_cast<typename ::session&>(context).get_productions();
  return productions.find(production_bank.lookup(*this)) != productions.end();
}

base_class*production::clone() const {
  return new production{*this};
}

size_t production::hash() const {
  return hash_value;
}

match::match(typename ::session&session, ::buffer*buffer, const ::production&production, const unsigned slots_filled, const token_iterator beginning, const token_iterator inclusive_end) :
  annotation_fact{session},
  buffer{buffer},
  production{&production_bank.acquire(production)},
  slots_filled{slots_filled},
  beginning{beginning},
  inclusive_end{inclusive_end} {
  assert(production);
}

match::match(typename ::session&session, ::buffer*buffer, const ::production&production, unsigned slots_filled, token_iterator beginning) :
  annotation_fact{session},
  buffer{buffer},
  production{&production_bank.acquire(production)},
  slots_filled{slots_filled},
  beginning{beginning},
  inclusive_end{beginning} {
  assert(production);
  assert(production.can_begin_with(*beginning).size());
}

match::~match() {
  production_bank.release(*production);
}

match::match(const match&prefix, token_iterator inclusive_end) :
  annotation_fact{prefix.context},
  buffer{prefix.buffer},
  production{&production_bank.acquire(*prefix.production)},
  slots_filled{prefix.slots_filled + 1},
  beginning{prefix.beginning},
  inclusive_end{inclusive_end} {
  assert(beginning != inclusive_end);
  assert(prefix.inclusive_end == inclusive_end || next(prefix.inclusive_end) == inclusive_end);
}

match::match(const ::production&production, const match&addendum) :
  annotation_fact{addendum.context},
  buffer{addendum.buffer},
  production{&production_bank.acquire(production)},
  slots_filled{1},
  beginning{addendum.beginning},
  inclusive_end{addendum.inclusive_end} {
  assert(production);
  assert(production.can_begin_with(addendum.production->get_result()).size());
}

match::match(const match&prefix, const match&addendum) :
  annotation_fact{prefix.context},
  buffer{prefix.buffer},
  production{&production_bank.acquire(*prefix.production)},
  slots_filled{prefix.slots_filled + 1},
  beginning{prefix.beginning},
  inclusive_end{addendum.inclusive_end} {
  assert(&prefix.context == &addendum.context);
  assert(prefix.buffer == addendum.buffer);
  assert(prefix.inclusive_end != addendum.beginning);
  assert(next(prefix.inclusive_end) == addendum.beginning);
}

bool match::can_continue_with(token_iterator end) const {
  return
    !is_complete() &&
    end.can_increment() &&
    production->accepts(slots_filled, token_terminal{*end->get_text()}) &&
    (end != inclusive_end) &&
    (end == next(inclusive_end));
}

bool match::can_continue_with(const match&addendum) const {
  assert(&session == &addendum.context);
  return
    !is_complete() &&
    addendum.is_complete() &&
    production->accepts(slots_filled, addendum.production->get_result()) &&
    (inclusive_end != addendum.beginning) &&
    (next(inclusive_end) == addendum.beginning);
}

bool match::is_equal_to_instance_of_like_class(const base_class&other) const {
  const match&cast = dynamic_cast<const match&>(other);
  return
    (slots_filled == cast.slots_filled) &&
    (beginning == cast.beginning) &&
    (inclusive_end == cast.inclusive_end) &&
    (production == cast.production); // last because this is most expensive
}

vector<const annotatable*>match::get_annotatables() const {
  if (beginning == inclusive_end) {
    return {&*beginning};
  }
  return {&*beginning, &*inclusive_end};
}

void match::justification_hook() const {
  assert(buffer);
  if (!operator bool()) {
    buffer->add_parseme_beginning(production->get_result(), beginning);
  }
  annotation_fact::justification_hook();
}

void match::unjustification_hook() const {
  assert(buffer);
  annotation_fact::unjustification_hook();
  if (!operator bool()) {
    buffer->remove_parseme_beginning(production->get_result(), beginning);
  }
}

vector<fact*>match::get_immediate_consequences() const {
  vector<fact*>results;
  if (is_complete()) {
    // Case I: The complete match begins another
    const parseme*key = parseme_bank.lookup(production->get_result());
    if (key) {
      for (const ::production*production : dynamic_cast<typename ::session&>(context).get_productions(key)) {
	results.push_back(new match{*production, *this});
      }
    }
    // Case II: The complete match continues another
    for (const annotation_wrapper&wrapper : beginning->get_annotations(match_type_index)) {
      const match&candidate_prefix = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
      if (candidate_prefix.can_continue_with(*this)) {
	results.push_back(new match{candidate_prefix, *this});
      }
    }
  } else {
    // Case III: The match itself continues with an epsilon
    static epsilon_terminal epsilon;
    if (production->accepts(slots_filled, epsilon)) {
      results.push_back(new match{*this, inclusive_end});
    }
    token_iterator end = next(inclusive_end);
    if (end != inclusive_end) {
      // Case IV: The match itself continues with a token
      if (can_continue_with(end)) {
	results.push_back(new match{*this, end});
      }
      // Case V: The match itself continues with another match
      for (const annotation_wrapper&wrapper : end->get_annotations(match_type_index)) {
	const match&candidate_addendum = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
	if (can_continue_with(candidate_addendum)) {
	  results.push_back(new match{*this, candidate_addendum});
	}
      }
    }
  }
  // Done
  return results;
}

bool match::is_complete() const {
  return slots_filled == production->get_slot_count();
}

base_class*match::clone() const {
  return new match{dynamic_cast<typename ::session&>(context), buffer, *production, slots_filled, beginning, inclusive_end};
}

size_t match::hash() const {
  return reinterpret_cast<size_t>(production) + reinterpret_cast<size_t>(&*beginning) + reinterpret_cast<size_t>(&*inclusive_end) + slots_filled;
}

internalizer<parseme>parseme_bank;
internalizer<production>production_bank;
