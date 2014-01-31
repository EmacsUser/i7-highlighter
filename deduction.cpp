#include <cassert>

#include "deduction.hpp"

using namespace std;

void fact::justify() const {
  assert(is_observation());
  bool change = !operator bool();
  justification_hook();
  if (change) {
    justification_propagate();
  }
}

void fact::unjustify() const {
  assert(is_observation());
  unjustification_hook();
  if (!operator bool()) {
    unjustification_propagate();
  }
}

void fact::justification_propagate() const {
  vector<fact*>immediate_consequences = get_immediate_consequences();
  vector<fact*>propagating_immediate_consequences;
  for (fact*immediate_consequence : immediate_consequences) {
    assert(!immediate_consequence->is_observation());
    bool change = !*immediate_consequence;
    immediate_consequence->justification_hook();
    if (change) {
      propagating_immediate_consequences.push_back(immediate_consequence);
    } else {
      delete immediate_consequence;
    }
  }
  for (fact*immediate_consequence : propagating_immediate_consequences) {
    immediate_consequence->justification_propagate();
    delete immediate_consequence;
  }
}

void fact::unjustification_propagate() const {
  vector<fact*>immediate_consequences = get_immediate_consequences();
  vector<fact*>propagating_immediate_consequences;
  for (fact*immediate_consequence : immediate_consequences) {
    assert(!immediate_consequence->is_observation());
    assert(*immediate_consequence);
    immediate_consequence->unjustification_hook();
    if (!*immediate_consequence) {
      propagating_immediate_consequences.push_back(immediate_consequence);
    } else {
      delete immediate_consequence;
    }
  }
  for (fact*immediate_consequence : propagating_immediate_consequences) {
    immediate_consequence->unjustification_propagate();
    delete immediate_consequence;
  }
}

ostream&operator <<(ostream&out, const ::fact&fact) {
  return fact.print(out);
}
