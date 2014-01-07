#include <cassert>
#include <typeinfo>
#include <typeindex>

#include "parser.hpp"
#include "internalizer.hpp"
#include "token.hpp"

using namespace std;

parseme::parseme(const i7_string&terminal_word) :
  terminal{true},
  value{&vocabulary.acquire(terminal_word)},
  tier{0} {}

parseme::parseme(const i7_string&nonterminal_kind, unsigned tier) :
  terminal{false},
  value{&vocabulary.acquire(nonterminal_kind)},
  tier{tier} {}

parseme::parseme(const parseme&copy) :
  terminal{copy.terminal},
  value{&vocabulary.acquire(*copy.value)},
  tier{copy.tier} {}

parseme::~parseme() {
  vocabulary.release(*value);
}

parseme&parseme::operator =(const parseme&other) {
  if (&other == this) {
    return *this;
  }
  vocabulary.release(*value);
  terminal = other.terminal;
  value = &vocabulary.acquire(*other.value);
  tier = other.tier;
  return *this;
}

bool parseme::is_terminal() const {
  return terminal;
}
bool parseme::is_empty_terminal() const {
  return terminal && value->empty();
}
const i7_string&parseme::get_terminal_word() const {
  assert(terminal);
  return *value;
}
const i7_string&parseme::get_nonterminal_kind() const {
  assert(!terminal);
  return *value;
}
unsigned parseme::get_tier() const {
  return tier;
}

bool parseme::accepts(const parseme&other) const {
  if (terminal != other.terminal) {
    return false;
  }
  if (value != other.value) {
    return false;
  }
  return tier >= other.tier;
}

void production::add_slot() {
  alternatives_sequence.push_back({});
}
void production::add_alternative_to_slot(const parseme&alternative) {
  assert(alternatives_sequence.size());
  alternatives_sequence.back().push_back(alternative);
}

const parseme&production::get_result() const {
  return result;
}

unsigned production::get_slot_count() const {
  return alternatives_sequence.size();
}

bool production::accepts(unsigned slot_index, const ::parseme&parseme) const {
  assert(slot_index < alternatives_sequence.size());
  for (const ::parseme&alternative : alternatives_sequence[slot_index]) {
    if (alternative.accepts(parseme)) {
      return true;
    }
  }
  return false;
}

bool production::can_begin_with(const ::token&token) const {
  return alternatives_sequence.size() && accepts(0, {*token.get_text()});
}

static token_iterator next(const token_iterator iterator) {
  token_iterator result = iterator;
  assert(result.can_increment());
  ++result;
  return result;
}

match::match(typename ::session&session, const ::production&production, const unsigned slots_filled, const token_iterator beginning, const token_iterator end) :
  fact{session},
  production{production},
  slots_filled{slots_filled},
  beginning{beginning},
  end{end} {}

match::match(typename ::session&session, const ::production&production, token_iterator beginning) :
  fact{session},
  production{production},
  slots_filled{1},
  beginning{beginning},
  end{next(beginning)} {
  assert(production.can_begin_with(*beginning));
}

match::match(typename ::session&session, const ::production&production, token_iterator beginning, token_iterator end) :
  fact{session},
  production{production},
  slots_filled{0},
  beginning{beginning},
  end{end} {
  assert(!production.get_slot_count());
}

match::match(const match&prefix) :
  fact{prefix.context},
  production{prefix.production},
  slots_filled{prefix.slots_filled + 1},
  beginning{prefix.beginning},
  end{next(prefix.end)} {}

match::match(const match&prefix, const match&addendum) :
  fact{prefix.context},
  production{prefix.production},
  slots_filled{prefix.slots_filled + 1},
  beginning{prefix.beginning},
  end{addendum.end} {
  assert(&prefix.context == &addendum.context);
  assert(prefix.end == addendum.beginning);
}

bool match::can_continue_with_token() const {
  return
    !is_complete() &&
    end.can_increment() &&
    production.accepts(slots_filled, {*end->get_text()});
}
bool match::can_continue_with(const match&addendum) const {
  assert(&session == &addendum.context);
  return
    !is_complete() &&
    (end == addendum.beginning) &&
    addendum.is_complete() &&
    production.accepts(slots_filled, addendum.production.get_result());
}

void match::justification_hook() const {
  beginning->add_annotation(*this);
  if (end.can_increment()) {
    end->add_annotation(*this);
  }
}
void match::unjustification_hook() const {
  beginning->remove_annotation(*this);
  if (end.can_increment()) {
    end->remove_annotation(*this);
  }
}

vector<fact*>match::get_immediate_consequences() const {
  vector<fact*>results;
  if (end.can_increment()) {
    if (can_continue_with_token()) {
      results.push_back(new match{*this});
    }
    for (const annotation_wrapper&wrapper : end->get_annotations(type_index(typeid(*this)))) {
      const match&candidate_addendum = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
      if (can_continue_with(candidate_addendum)) {
	results.push_back(new match{*this});
      }
    }
  }
  return results;
}

annotation*match::clone() const {
  return new match{*this};
}

match::operator bool() const {
  return beginning->has_annotation(*this);
}

bool match::is_complete() const {
  return slots_filled == production.get_slot_count();
}

size_t match::hash() const {
  return reinterpret_cast<size_t>(&*beginning) + reinterpret_cast<size_t>(&*end) + slots_filled;
}

bool match::is_equal_to_instance_of_like_class(const annotation&other) const {
  const match&cast = dynamic_cast<const match&>(other);
  return
    (&production == &cast.production) &&
    (slots_filled == cast.slots_filled) &&
    (beginning == cast.beginning) &&
    (end == cast.end);
}
