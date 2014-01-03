#include "delimiters.hpp"

using namespace std;

namespace {
  enum lexical_supersuperstate {
    UNNESTED_I7_IN_CONTEXT,
    I6_IN_CONTEXT,
    I7_IN_I6_IN_CONTEXT,
    I6_COMMENT_IN_CONTEXT,
    I7_IN_I6_COMMENT_IN_CONTEXT,
    I7_STRING_IN_CONTEXT,
    OTHER_IN_CONTEXT
  };
}

static lexical_supersuperstate supersuperstate_of(lexical_state state) {
  switch (state.get_superstate()) {
  case I7:
  case I7_IN_EXTRACT:
    return UNNESTED_I7_IN_CONTEXT;
  case I6:
  case I6_IN_ROUTINE:
  case I6_IN_EXTRACT:
  case I6_IN_ROUTINE_IN_EXTRACT:
    return I6_IN_CONTEXT;
  case I7_IN_I6:
  case I7_IN_I6_IN_ROUTINE:
  case I7_IN_I6_IN_EXTRACT:
  case I7_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return I7_IN_I6_IN_CONTEXT;
  case I6_COMMENT:
  case I6_COMMENT_IN_ROUTINE:
  case I6_COMMENT_IN_EXTRACT:
  case I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
    return I6_COMMENT_IN_CONTEXT;
  case I7_IN_I6_COMMENT:
  case I7_IN_I6_COMMENT_IN_ROUTINE:
  case I7_IN_I6_COMMENT_IN_EXTRACT:
  case I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
    return I7_IN_I6_COMMENT_IN_CONTEXT;
  case I7_STRING:
  case I7_STRING_IN_I6:
  case I7_STRING_IN_I6_IN_ROUTINE:
  case I7_STRING_IN_EXTRACT:
  case I7_STRING_IN_I6_IN_EXTRACT:
  case I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return I7_STRING_IN_CONTEXT;
  }
  return OTHER_IN_CONTEXT;
}

static stack_monoid<delimiter>get_delimiter_effect(const lexical_state&before, const monoid_sequence<token>::iterator&position, const lexical_state&after) {
  if (before.get_comment_depth() < after.get_comment_depth()) {
    assert(before.get_comment_depth() + 1 == after.get_comment_depth());
    return {{I7_COMMENT_DELIMITER, position}, false};
  }
  if (before.get_comment_depth() > after.get_comment_depth()) {
    assert(before.get_comment_depth() == 1 + after.get_comment_depth());
    return {{I7_COMMENT_DELIMITER, position}, true};
  }
  switch (supersuperstate_of(before.get_superstate())) {
  case UNNESTED_I7_IN_CONTEXT:
    switch (supersuperstate_of(after.get_superstate())) {
    case I6_IN_CONTEXT:
      return {{I6_DELIMITER, position}, false};
    case I7_STRING_IN_CONTEXT:
      return {{I7_STRING_DELIMITER, position}, false};
    }
    break;
  case I6_IN_CONTEXT:
    switch (supersuperstate_of(after.get_superstate())) {
    case UNNESTED_I7_IN_CONTEXT:
      return {{I6_DELIMITER, position}, true};
    case I7_IN_I6_IN_CONTEXT:
      return {{I7_DELIMITER, position}, false};
    case I6_COMMENT_IN_CONTEXT:
      return {{I6_COMMENT_DELIMITER, position}, false};
    }
    break;
  case I7_IN_I6_IN_CONTEXT:
    switch (supersuperstate_of(after.get_superstate())) {
    case UNNESTED_I7_IN_CONTEXT:
      return stack_monoid<delimiter>{{I7_DELIMITER, position}, true} + stack_monoid<delimiter>{{I6_DELIMITER, position}, true};
    case I6_IN_CONTEXT:
      return {{I7_DELIMITER, position}, true};
    case I7_STRING_IN_CONTEXT:
      return {{I7_STRING_DELIMITER, position}, false};
    }
    break;
  case I6_COMMENT_IN_CONTEXT:
    switch (supersuperstate_of(after.get_superstate())) {
    case UNNESTED_I7_IN_CONTEXT:
      return stack_monoid<delimiter>{{I6_COMMENT_DELIMITER, position}, true} + stack_monoid<delimiter>{{I6_DELIMITER, position}, true};
    case I6_IN_CONTEXT:
      return {{I6_COMMENT_DELIMITER, position}, true};
    case I7_IN_I6_COMMENT_IN_CONTEXT:
      return {{I7_DELIMITER, position}, false};
    }
    break;
  case I7_IN_I6_COMMENT_IN_CONTEXT:
    switch (supersuperstate_of(after.get_superstate())) {
    case UNNESTED_I7_IN_CONTEXT:
      return stack_monoid<delimiter>{{I7_DELIMITER, position}, true} + stack_monoid<delimiter>{{I6_COMMENT_DELIMITER, position}, true} + stack_monoid<delimiter>{{I6_DELIMITER, position}, true};
    case I6_COMMENT_IN_CONTEXT:
      return {{I7_DELIMITER, position}, true};
    case I7_STRING_IN_CONTEXT:
      return {{I7_STRING_DELIMITER, position}, false};
    }
    break;
  case I7_STRING_IN_CONTEXT:
    switch (supersuperstate_of(after.get_superstate())) {
    case UNNESTED_I7_IN_CONTEXT:
    case I7_IN_I6_IN_CONTEXT:
    case I7_IN_I6_COMMENT_IN_CONTEXT:
      return {{I7_STRING_DELIMITER, position}, true};
    }
    break;
  }
  return stack_monoid<delimiter>{0};
}

