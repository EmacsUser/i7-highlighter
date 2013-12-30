#include "protocol.hpp"
#include "lexical_highlights.hpp"

enum lexical_supersuperstate {
  I7_IN_CONTEXT,
  I6_IN_CONTEXT,
  I7_COMMENT_IN_CONTEXT,
  I6_COMMENT_IN_CONTEXT,
  DOCUMENTATION_IN_CONTEXT,
  CHARACTER_IN_CONTEXT,
  STRING_IN_CONTEXT,
  SUBSTITUTION_IN_CONTEXT
};

lexical_supersuperstate supersuperstate_of(lexical_state state) {
  if (state.get_comment_depth()) {
    return I7_COMMENT_IN_CONTEXT;
  }
  switch (state.get_superstate()) {
  case I7:
  case I7_IN_I6:
  case I7_IN_EXTRACT:
  case I7_IN_I6_IN_ROUTINE:
  case I7_IN_I6_IN_EXTRACT:
  case I7_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return I7_IN_CONTEXT;
  case I6:
  case I6_IN_ROUTINE:
  case I6_IN_EXTRACT:
  case I6_IN_ROUTINE_IN_EXTRACT:
    return I6_IN_CONTEXT;
  case I6_COMMENT:
  case I6_COMMENT_IN_ROUTINE:
  case I6_COMMENT_IN_EXTRACT:
  case I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
    return I6_COMMENT_IN_CONTEXT;
  case I7_EXTENSION_DOCUMENTATION:
    return DOCUMENTATION_IN_CONTEXT;
  case I6_CHARACTER:
  case I6_CHARACTER_IN_ROUTINE:
  case I6_CHARACTER_IN_EXTRACT:
  case I6_CHARACTER_IN_ROUTINE_IN_EXTRACT:
    return CHARACTER_IN_CONTEXT;
  case I7_STRING:
  case I6_STRING:
  case I6_STRING_IN_ROUTINE:
  case I7_STRING_IN_I6:
  case I7_STRING_IN_I6_IN_ROUTINE:
  case I7_STRING_IN_EXTRACT:
  case I6_STRING_IN_EXTRACT:
  case I6_STRING_IN_ROUTINE_IN_EXTRACT:
  case I7_STRING_IN_I6_IN_EXTRACT:
  case I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return STRING_IN_CONTEXT;
  case I7_SUBSTITUTION:
  case I7_SUBSTITUTION_IN_I6:
  case I7_SUBSTITUTION_IN_I6_IN_ROUTINE:
  case I7_SUBSTITUTION_IN_EXTRACT:
  case I7_SUBSTITUTION_IN_I6_IN_EXTRACT:
  case I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return SUBSTITUTION_IN_CONTEXT;
  }
  return I7_IN_CONTEXT;
}

bool is_extract_state(lexical_state state) {
  switch (state.get_superstate()) {
  case I7_IN_EXTRACT:
  case I7_IN_I6_IN_EXTRACT:
  case I7_IN_I6_IN_ROUTINE_IN_EXTRACT:
  case I6_IN_EXTRACT:
  case I6_IN_ROUTINE_IN_EXTRACT:
  case I6_COMMENT_IN_EXTRACT:
  case I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
  case I6_CHARACTER_IN_EXTRACT:
  case I6_CHARACTER_IN_ROUTINE_IN_EXTRACT:
  case I7_STRING_IN_EXTRACT:
  case I6_STRING_IN_EXTRACT:
  case I6_STRING_IN_ROUTINE_IN_EXTRACT:
  case I7_STRING_IN_I6_IN_EXTRACT:
  case I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT:
  case I7_SUBSTITUTION_IN_EXTRACT:
  case I7_SUBSTITUTION_IN_I6_IN_EXTRACT:
  case I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return true;
  }
  return false;
}

bool is_self_nested(lexical_state state) {
  switch (state.get_superstate()) {
  case I7_IN_I6:
  case I7_IN_I6_IN_ROUTINE:
  case I7_IN_I6_IN_EXTRACT:
  case I7_IN_I6_IN_ROUTINE_IN_EXTRACT:
  case I7_STRING_IN_I6:
  case I7_STRING_IN_I6_IN_ROUTINE:
  case I7_STRING_IN_I6_IN_EXTRACT:
  case I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT:
  case I7_SUBSTITUTION_IN_I6:
  case I7_SUBSTITUTION_IN_I6_IN_ROUTINE:
  case I7_SUBSTITUTION_IN_I6_IN_EXTRACT:
  case I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return true;
  }
  return false;
}

highlight_code get_highlight_code(lexical_state before, lexical_state after) {
  switch (supersuperstate_of(after)) {
  case I7_IN_CONTEXT:
    switch (supersuperstate_of(before)) {
    case I6_IN_CONTEXT:
      if (is_self_nested(after)) {
	return HIGHLIGHT_I7_DELIMITER;
      }
      return HIGHLIGHT_I6_DELIMITER;
    case I7_COMMENT_IN_CONTEXT:
      return HIGHLIGHT_COMMENT_DELIMITER;
    case I6_COMMENT_IN_CONTEXT:
      return HIGHLIGHT_I6_DELIMITER;
    case DOCUMENTATION_IN_CONTEXT:
      return HIGHLIGHT_ORDINARY_I7;
    case STRING_IN_CONTEXT:
    case SUBSTITUTION_IN_CONTEXT:
      return HIGHLIGHT_STRING_LITERAL_DELIMITER;
    }
    return HIGHLIGHT_ORDINARY_I7;
  case I6_IN_CONTEXT:
    switch (supersuperstate_of(before)) {
    case I7_IN_CONTEXT:
      if (is_self_nested(before)) {
	return HIGHLIGHT_I7_DELIMITER;
      }
      return HIGHLIGHT_I6_DELIMITER;
    case CHARACTER_IN_CONTEXT:
      return HIGHLIGHT_CHARACTER_LITERAL_DELIMITER;
    case STRING_IN_CONTEXT:
      return HIGHLIGHT_STRING_LITERAL_DELIMITER;
    }
    return HIGHLIGHT_ORDINARY_I6;
  case I7_COMMENT_IN_CONTEXT:
  case I6_COMMENT_IN_CONTEXT:
    switch (supersuperstate_of(before)) {
    case I7_IN_CONTEXT:
    case I6_IN_CONTEXT:
    case DOCUMENTATION_IN_CONTEXT:
      return HIGHLIGHT_COMMENT_DELIMITER;
    }
    return HIGHLIGHT_COMMENT;
  case DOCUMENTATION_IN_CONTEXT:
    if (!is_extract_state(before)) {
      switch (supersuperstate_of(before)) {
      case I7_IN_CONTEXT:
	return HIGHLIGHT_DOCUMENTATION_DELIMITER;
      }
    }
    return HIGHLIGHT_DOCUMENTATION;
  case CHARACTER_IN_CONTEXT:
    switch (supersuperstate_of(before)) {
    case I6_IN_CONTEXT:
      return HIGHLIGHT_CHARACTER_LITERAL_DELIMITER;
    }
    return HIGHLIGHT_CHARACTER_LITERAL;
  case STRING_IN_CONTEXT:
    switch (supersuperstate_of(before)) {
    case I7_IN_CONTEXT:
    case I6_IN_CONTEXT:
      return HIGHLIGHT_STRING_LITERAL_DELIMITER;
    case SUBSTITUTION_IN_CONTEXT:
      return HIGHLIGHT_SUBSTITUTION_DELIMITER;
    }
    return HIGHLIGHT_STRING_LITERAL;
  case SUBSTITUTION_IN_CONTEXT:
    switch (supersuperstate_of(before)) {
    case STRING_IN_CONTEXT:
      return HIGHLIGHT_SUBSTITUTION_DELIMITER;
    }
    return HIGHLIGHT_SUBSTITUTION;
  }
  return HIGHLIGHT_ORDINARY_I7;
}
