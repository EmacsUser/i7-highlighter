#include "deduction.hpp"

using namespace std;

void fact::justify() {
  bool change = !operator bool();
  justification_hook();
  if (change) {
    for (fact*immediate_consequence : get_immediate_consequences()) {
      immediate_consequence->justify();
      delete immediate_consequence;
    }
  }
}

void fact::unjustify() {
  unjustification_hook();
  if (!operator bool()) {
    for (fact*immediate_consequence : get_immediate_consequences()) {
      immediate_consequence->unjustify();
      delete immediate_consequence;
    }
  }
}
