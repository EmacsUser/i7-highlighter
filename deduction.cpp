#include "deduction.hpp"

using namespace std;

void fact::justify() {
  if (justification_hook()) {
    for (fact&immediate_consequence : get_immediate_consequences()) {
      immediate_consequence.justify();
    }
  }
}

void fact::unjustify() {
  if (unjustification_hook()) {
    for (fact&immediate_consequence : get_immediate_consequences()) {
      immediate_consequence.unjustify();
    }
  }
}
