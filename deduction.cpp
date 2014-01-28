#include <cassert>

#include "deduction.hpp"

using namespace std;

void fact::justify() const {
  bool change = !operator bool();
  justification_hook();
  if (change) {
    for (fact*immediate_consequence : get_immediate_consequences()) {
      immediate_consequence->justify();
      delete immediate_consequence;
    }
  }
}

void fact::unjustify() const {
  unjustification_hook();
  if (!operator bool()) {
    for (fact*immediate_consequence : get_immediate_consequences()) {
      assert(*immediate_consequence);
      immediate_consequence->unjustify();
      delete immediate_consequence;
    }
  }
}

ostream&operator <<(ostream&out, const ::fact&fact) {
  return fact.print(out);
}
