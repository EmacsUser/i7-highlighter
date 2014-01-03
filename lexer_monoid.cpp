#include <cassert>
#include <cstring> // For memcpy.

#include "lexer_monoid.hpp"

using namespace std;

#define INVALID_INDEX static_cast<unsigned>(-1)

// If W is an index among all superstates that can take I7 comment levels,
// commentable_superstate_map[W] gives the corresponding superstate.
static const lexical_superstate commentable_superstate_map[COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS] = {
  I7,
  I7_EXTENSION_DOCUMENTATION,
  I7_IN_I6,
  I7_IN_I6_COMMENT,
  I7_IN_I6_IN_ROUTINE,
  I7_IN_I6_COMMENT_IN_ROUTINE,
  I7_IN_EXTRACT,
  I7_IN_I6_IN_EXTRACT,
  I7_IN_I6_COMMENT_IN_EXTRACT,
  I7_IN_I6_IN_ROUTINE_IN_EXTRACT,
  I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT
};

// If X is a lexical_superstate that can take I7 comment levels,
// commentable_index_map[X] gives its index among all such superstates.
// Otherwise, it gives INVALID_INDEX.
static const unsigned commentable_index_map[LEXICAL_SUPERSTATE_COUNT] = {
  0, INVALID_INDEX, INVALID_INDEX, 1,
  INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
  INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
  2, INVALID_INDEX, INVALID_INDEX, 3, INVALID_INDEX, INVALID_INDEX,
  4, INVALID_INDEX, INVALID_INDEX, 5, INVALID_INDEX, INVALID_INDEX,
  6, INVALID_INDEX, INVALID_INDEX,
  INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
  INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
  7, INVALID_INDEX, INVALID_INDEX, 8, INVALID_INDEX, INVALID_INDEX,
  9, INVALID_INDEX, INVALID_INDEX, 10, INVALID_INDEX, INVALID_INDEX
};

ostream&operator <<(ostream&out, lexical_superstate superstate) {
  switch (superstate) {
  case I7:
    return out << "7";
  case I7_STRING:
    return out << "\"7\"";
  case I7_SUBSTITUTION:
    return out << "\"[7]\"";
  case I7_EXTENSION_DOCUMENTATION:
    return out << "--7--";
  case I6:
    return out << "(-6-)";
  case I6_CHARACTER:
    return out << "(-'6'-)";
  case I6_STRING:
    return out << "(-\"6\"-)";
  case I6_COMMENT:
    return out << "(-!6-)";
  case I6_IN_ROUTINE:
    return out << "(-[6]-)";
  case I6_CHARACTER_IN_ROUTINE:
    return out << "(-['6']-)";
  case I6_STRING_IN_ROUTINE:
    return out << "(-[\"6\"]-)";
  case I6_COMMENT_IN_ROUTINE:
    return out << "(-[!6]-)";
  case I7_IN_I6:
    return out << "(+7+)";
  case I7_STRING_IN_I6:
    return out << "(+\"7\"+)";
  case I7_SUBSTITUTION_IN_I6:
    return out << "(+\"[7]\"+)";
  case I7_IN_I6_COMMENT:
    return out << "!(+7+)";
  case I7_STRING_IN_I6_COMMENT:
    return out << "!(+\"7\"+)";
  case I7_SUBSTITUTION_IN_I6_COMMENT:
    return out << "!(+\"[7]\"+)";
  case I7_IN_I6_IN_ROUTINE:
    return out << "[(+7+)]";
  case I7_STRING_IN_I6_IN_ROUTINE:
    return out << "[(+\"7\"+)]";
  case I7_SUBSTITUTION_IN_I6_IN_ROUTINE:
    return out << "[(+\"[7]\"+)]";
  case I7_IN_I6_COMMENT_IN_ROUTINE:
    return out << "[!(+7+)]";
  case I7_STRING_IN_I6_COMMENT_IN_ROUTINE:
    return out << "[!(+\"7\"+)]";
  case I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE:
    return out << "[!(+\"[7]\"+)]";
  case I7_IN_EXTRACT:
    return out << "*:7";
  case I7_STRING_IN_EXTRACT:
    return out << "*:\"7\"";
  case I7_SUBSTITUTION_IN_EXTRACT:
    return out << "*:\"[7]\"";
  case I6_IN_EXTRACT:
    return out << "*:(-6-)";
  case I6_CHARACTER_IN_EXTRACT:
    return out << "*:(-'6'-)";
  case I6_STRING_IN_EXTRACT:
    return out << "*:(-\"6\"-)";
  case I6_COMMENT_IN_EXTRACT:
    return out << "*:(-!6-)";
  case I6_IN_ROUTINE_IN_EXTRACT:
    return out << "*:(-[6]-)";
  case I6_CHARACTER_IN_ROUTINE_IN_EXTRACT:
    return out << "*:(-['6']-)";
  case I6_STRING_IN_ROUTINE_IN_EXTRACT:
    return out << "*:(-[\"6\"]-)";
  case I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
    return out << "*:(-[!6]-)";
  case I7_IN_I6_IN_EXTRACT:
    return out << "*:(+7+)";
  case I7_STRING_IN_I6_IN_EXTRACT:
    return out << "*:(+\"7\"+)";
  case I7_SUBSTITUTION_IN_I6_IN_EXTRACT:
    return out << "*:(+\"[7]\"+)";
  case I7_IN_I6_COMMENT_IN_EXTRACT:
    return out << "*:!(+7+)";
  case I7_STRING_IN_I6_COMMENT_IN_EXTRACT:
    return out << "*:!(+\"7\"+)";
  case I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT:
    return out << "*:!(+\"[7]\"+)";
  case I7_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return out << "*:[(+7+)]";
  case I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return out << "*:[(+\"7\"+)]";
  case I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT:
    return out << "*:[(+\"[7]\"+)]";
  case I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
    return out << "*:[!(+7+)]";
  case I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
    return out << "*:[!(+\"7\"+)]";
  case I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT:
    return out << "*:[!(+\"[7]\"+)]";
  default:
    return out << "???";
  }
}

ostream&operator <<(ostream&out, lexical_state state) {
  out << state.get_superstate();
  if (state.get_comment_depth()) {
    out << "+" << state.get_comment_depth();
  }
  return out;
}

// Set composition_comment_images (which should be empty on entry) to the
// comment images for superstate in a composition of the two lexer monoid
// elements this and other, where, as a convenience, own_comment_images and
// other_comment_images already reference their comment images.
void lexer_monoid::compose(vector<lexical_state>&composition_comment_images, lexical_superstate superstate, const vector<lexical_state>&own_comment_images, const lexer_monoid&other, const vector<lexical_state>&other_comment_images) const {
  // A element in the preimage should be special-cased if 1) it is already
  // special-cased by own_comment_images, 2) its comment depth plus the
  // comment_depth_change brings it to an uncommented state, or 3) its comment
  // depth plus the comment_depth_change brings it into the special-case range
  // of other_comment_images.
  //
  // Case 1:
  for (const lexical_state&state : own_comment_images) {
    composition_comment_images.push_back(other(state));
  }
  // Case 2:
  //
  // Note that negative comment depths should be impossible.
  if (comment_depth_change + own_comment_images.size() + 1 == 0) {
    composition_comment_images.push_back(other(superstate));
  }
  // Case 3:
  //
  // I was tempted to use insert here, but a loop just read better.  Note that
  // index is the index into other_comment_images; the loop bounds are confusing
  // if you try to read them without knowing that.
  for (unsigned index = own_comment_images.size() + comment_depth_change; index < other_comment_images.size(); ++index) {
    composition_comment_images.push_back(other_comment_images[index]);
  }
  // Finally, clean up any redundancies:
  while (composition_comment_images.size() && composition_comment_images.back() == other({superstate, static_cast<unsigned>(composition_comment_images.size())})) {
    composition_comment_images.pop_back();
  }
}

lexer_monoid::lexer_monoid(int8_t comment_depth_change) :
  images{LEXICAL_SUPERSTATE_LIST},
  comment_depth_change{comment_depth_change} {
  assert (comment_depth_change == -1 || comment_depth_change == 0 || comment_depth_change == 1);
  if (comment_depth_change == 1) {
    for (unsigned i = COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS; i--;) {
      images[commentable_superstate_map[i]] = {commentable_superstate_map[i], 1};
    }
  }
}

lexer_monoid::lexer_monoid(lexical_superstate from, lexical_superstate to, bool also_in_reverse, bool even_in_I7_comments) :
  images{LEXICAL_SUPERSTATE_LIST},
  comment_depth_change{0} {
  assert(!also_in_reverse || !even_in_I7_comments);
  images[from] = to;
  if (also_in_reverse) {
    images[to] = from;
  }
}

lexer_monoid::lexer_monoid(const lexer_monoid&copy) :
  images{copy.images[0], copy.images[1], copy.images[2], copy.images[3], copy.images[4], copy.images[5], copy.images[6], copy.images[7], copy.images[8], copy.images[9], copy.images[10], copy.images[11], copy.images[12], copy.images[13], copy.images[14], copy.images[15], copy.images[16], copy.images[17], copy.images[18], copy.images[19], copy.images[20], copy.images[21], copy.images[22], copy.images[23], copy.images[24], copy.images[25], copy.images[26], copy.images[27], copy.images[28], copy.images[29], copy.images[30], copy.images[31], copy.images[32], copy.images[33], copy.images[34], copy.images[35], copy.images[36], copy.images[37], copy.images[38], copy.images[39], copy.images[40], copy.images[41], copy.images[42], copy.images[43], copy.images[44], copy.images[45], copy.images[46]},
  comment_depth_change{copy.comment_depth_change} {
  for (unsigned i = COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS; i--;) {
    comment_images[i] = copy.comment_images[i];
  }
}

lexer_monoid&lexer_monoid::operator =(const lexer_monoid&other) {
  memcpy(&images, &other.images, sizeof(images));
  comment_depth_change = other.comment_depth_change;
  for (unsigned i = COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS; i--;) {
    comment_images[i] = other.comment_images[i];
  }
  return *this;
}

lexical_state lexer_monoid::operator ()(lexical_state state) const {
  lexical_superstate superstate = state.get_superstate();
  unsigned comment_depth = state.get_comment_depth();
  if (!comment_depth) {
    return images[superstate];
  }
  const vector<lexical_state>&comment_images_for_superstate = comment_images[commentable_index_map[superstate]];
  if (comment_depth <= comment_images_for_superstate.size()) {
    return comment_images_for_superstate[comment_depth - 1];
  }
  return {superstate, comment_depth + comment_depth_change};
}

lexer_monoid lexer_monoid::operator +(const lexer_monoid&other) const {
  lexer_monoid result{0};
  for (lexical_state preimage = LEXICAL_SUPERSTATE_COUNT; (preimage--).get_superstate();) {
    result.images[preimage.get_superstate()] = other((*this)(preimage));
  }
  result.comment_depth_change = comment_depth_change + other.comment_depth_change;
  for (unsigned i = COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS; i--;) {
    lexical_superstate superstate = commentable_superstate_map[i];
    compose(result.comment_images[i], superstate, comment_images[i], other, other.comment_images[i]);
  }
  return result;
}

lexer_monoid&lexer_monoid::operator +=(const lexer_monoid&other) {
  return (*this) = (*this) + other;
}

ostream&operator <<(ostream&out, const lexer_monoid&element) {
  out << "{ ";
  for (lexical_state preimage = static_cast<lexical_superstate>(0); preimage.get_superstate() < LEXICAL_SUPERSTATE_COUNT; ++preimage) {
    lexical_state postimage = element(preimage);
    if (preimage != postimage) {
      out << preimage << " to " << postimage << ", ";
    }
  }
  for (unsigned i = COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS; i--;) {
    for (unsigned comment_depth = 1; comment_depth <= element.comment_images[i].size(); ++comment_depth) {
      lexical_state preimage = {commentable_superstate_map[i], comment_depth};
      lexical_state postimage = element(preimage);
      if (preimage != postimage) {
	out << preimage << " to " << postimage << ", ";
      }
    }
  }
  return out << "redepth by " << static_cast<int>(element.comment_depth_change) << " }";
}

const lexical_state INITIAL_LEXICAL_STATE = {I7};

lexer_monoid plain_text =
  lexer_monoid{0};

lexer_monoid double_quote =
  lexer_monoid{I7, I7_STRING, true} +
  lexer_monoid{I7_SUBSTITUTION, I7} +
  lexer_monoid{I6, I6_STRING, true} +
  lexer_monoid{I6_IN_ROUTINE, I6_STRING_IN_ROUTINE, true} +
  lexer_monoid{I7_IN_I6, I7_STRING_IN_I6, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6, I7_IN_I6} +
  lexer_monoid{I7_IN_I6_COMMENT, I7_STRING_IN_I6_COMMENT, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT, I7_IN_I6_COMMENT} +
  lexer_monoid{I7_IN_I6_IN_ROUTINE, I7_STRING_IN_I6_IN_ROUTINE, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE, I7_IN_I6_IN_ROUTINE} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE, I7_STRING_IN_I6_COMMENT_IN_ROUTINE, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE, I7_IN_I6_COMMENT_IN_ROUTINE} +
  lexer_monoid{I7_IN_EXTRACT, I7_STRING_IN_EXTRACT, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_IN_EXTRACT, I6_STRING_IN_EXTRACT, true} +
  lexer_monoid{I6_IN_ROUTINE_IN_EXTRACT, I6_STRING_IN_ROUTINE_IN_EXTRACT, true} +
  lexer_monoid{I7_IN_I6_IN_EXTRACT, I7_STRING_IN_I6_IN_EXTRACT, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_EXTRACT, I7_IN_I6_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_EXTRACT, I7_STRING_IN_I6_COMMENT_IN_EXTRACT, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT, I7_IN_I6_COMMENT_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_IN_I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT};

lexer_monoid left_bracket =
  lexer_monoid{I7_STRING, I7_SUBSTITUTION} +
  lexer_monoid{I6, I6_IN_ROUTINE} +
  lexer_monoid{I7_STRING_IN_I6, I7_SUBSTITUTION_IN_I6} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT, I7_SUBSTITUTION_IN_I6_COMMENT} +
  lexer_monoid{I7_STRING_IN_I6_IN_ROUTINE, I7_SUBSTITUTION_IN_I6_IN_ROUTINE} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE, I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE} +
  lexer_monoid{I7_STRING_IN_EXTRACT, I7_SUBSTITUTION_IN_EXTRACT} +
  lexer_monoid{I6_IN_EXTRACT, I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_IN_EXTRACT, I7_SUBSTITUTION_IN_I6_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_EXTRACT, I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{1};

lexer_monoid right_bracket =
  lexer_monoid{I7_SUBSTITUTION, I7_STRING} +
  lexer_monoid{I6_IN_ROUTINE, I6} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6, I7_STRING_IN_I6} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT, I7_STRING_IN_I6_COMMENT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE, I7_STRING_IN_I6_IN_ROUTINE} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE, I7_STRING_IN_I6_COMMENT_IN_ROUTINE} +
  lexer_monoid{I7_SUBSTITUTION_IN_EXTRACT, I7_STRING_IN_EXTRACT} +
  lexer_monoid{I6_IN_ROUTINE_IN_EXTRACT, I6_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_EXTRACT, I7_STRING_IN_I6_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT, I7_STRING_IN_I6_COMMENT_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{-1};

lexer_monoid documentation_break =
  lexer_monoid{I7, I7_EXTENSION_DOCUMENTATION};

lexer_monoid documentation_break_followed_by_indentation =
  lexer_monoid{I7, I7_IN_EXTRACT};

lexer_monoid indentation =
  lexer_monoid{I7_EXTENSION_DOCUMENTATION, I7_IN_EXTRACT} +
  lexer_monoid{I6_COMMENT, I6} +
  lexer_monoid{I6_COMMENT_IN_ROUTINE, I6_IN_ROUTINE} +
  lexer_monoid{I7_IN_I6_COMMENT, I7_IN_I6, false, true} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT, I7_STRING_IN_I6, false, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT, I7_SUBSTITUTION_IN_I6, false, true} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE, I7_IN_I6_IN_ROUTINE, false, true} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE, I7_STRING_IN_I6_IN_ROUTINE, false, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE, I7_SUBSTITUTION_IN_I6_IN_ROUTINE, false, true} +
  lexer_monoid{I6_COMMENT_IN_EXTRACT, I6_IN_EXTRACT} +
  lexer_monoid{I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_EXTRACT, I7_IN_I6_IN_EXTRACT, false, true} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_EXTRACT, I7_STRING_IN_I6_IN_EXTRACT, false, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT, I7_SUBSTITUTION_IN_I6_IN_EXTRACT, false, true} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_IN_I6_IN_ROUTINE_IN_EXTRACT, false, true} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT, false, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT, false, true};

lexer_monoid bare_newline =
  lexer_monoid{I6_COMMENT, I6} +
  lexer_monoid{I6_COMMENT_IN_ROUTINE, I6_IN_ROUTINE} +
  lexer_monoid{I7_IN_I6_COMMENT, I7_IN_I6, false, true} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT, I7_STRING_IN_I6, false, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT, I7_SUBSTITUTION_IN_I6, false, true} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE, I7_IN_I6_IN_ROUTINE, false, true} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE, I7_STRING_IN_I6_IN_ROUTINE, false, true} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE, I7_SUBSTITUTION_IN_I6_IN_ROUTINE, false, true} +
  lexer_monoid{I7_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_STRING_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_SUBSTITUTION_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_CHARACTER_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_STRING_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_COMMENT_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_CHARACTER_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_STRING_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_IN_I6_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_STRING_IN_I6_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_EXTENSION_DOCUMENTATION};

lexer_monoid left_cyclops =
  lexer_monoid{I7, I6} +
  lexer_monoid{I7_IN_EXTRACT, I6_IN_EXTRACT};

lexer_monoid right_cyclops =
  lexer_monoid{I6, I7} +
  lexer_monoid{I6_CHARACTER, I7} +
  lexer_monoid{I6_STRING, I7} +
  lexer_monoid{I6_COMMENT, I7} +
  lexer_monoid{I7_IN_I6, I7} +
  lexer_monoid{I7_STRING_IN_I6, I7} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6, I7} +
  lexer_monoid{I7_IN_I6_COMMENT, I7} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT, I7} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT, I7} +
  lexer_monoid{I6_IN_ROUTINE, I7} +
  lexer_monoid{I6_CHARACTER_IN_ROUTINE, I7} +
  lexer_monoid{I6_STRING_IN_ROUTINE, I7} +
  lexer_monoid{I6_COMMENT_IN_ROUTINE, I7} +
  lexer_monoid{I7_IN_I6_IN_ROUTINE, I7} +
  lexer_monoid{I7_STRING_IN_I6_IN_ROUTINE, I7} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE, I7} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE, I7} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE, I7} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE, I7} +
  lexer_monoid{I6_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_CHARACTER_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_STRING_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_COMMENT_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_CHARACTER_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_STRING_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I7_IN_EXTRACT};

lexer_monoid single_quote =
  lexer_monoid{I6, I6_CHARACTER, true} +
  lexer_monoid{I6_IN_ROUTINE, I6_CHARACTER_IN_ROUTINE, true} +
  lexer_monoid{I6_IN_EXTRACT, I6_CHARACTER_IN_EXTRACT, true} +
  lexer_monoid{I6_IN_ROUTINE_IN_EXTRACT, I6_CHARACTER_IN_ROUTINE_IN_EXTRACT, true};

lexer_monoid bang =
  lexer_monoid{I6, I6_COMMENT} +
  lexer_monoid{I6_IN_ROUTINE, I6_COMMENT_IN_ROUTINE} +
  lexer_monoid{I6_IN_EXTRACT, I6_COMMENT_IN_EXTRACT} +
  lexer_monoid{I6_IN_ROUTINE_IN_EXTRACT, I6_COMMENT_IN_ROUTINE_IN_EXTRACT};

lexer_monoid left_crosseyed_cyclops =
  lexer_monoid{I6, I7_IN_I6} +
  lexer_monoid{I6_IN_ROUTINE, I7_IN_I6_IN_ROUTINE} +
  lexer_monoid{I6_IN_EXTRACT, I7_IN_I6_IN_EXTRACT} +
  lexer_monoid{I6_IN_ROUTINE_IN_EXTRACT, I7_IN_I6_IN_ROUTINE_IN_EXTRACT};

lexer_monoid right_crosseyed_cyclops =
  lexer_monoid{I7_IN_I6, I6} +
  lexer_monoid{I7_STRING_IN_I6, I6} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6, I6} +
  lexer_monoid{I7_IN_I6_COMMENT, I6_COMMENT} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT, I6_COMMENT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT, I6_COMMENT} +
  lexer_monoid{I7_IN_I6_IN_ROUTINE, I6_IN_ROUTINE} +
  lexer_monoid{I7_STRING_IN_I6_IN_ROUTINE, I6_IN_ROUTINE} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE, I6_IN_ROUTINE} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE, I6_COMMENT_IN_ROUTINE} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE, I6_COMMENT_IN_ROUTINE} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE, I6_COMMENT_IN_ROUTINE} +
  lexer_monoid{I7_IN_I6_IN_EXTRACT, I6_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_IN_EXTRACT, I6_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_EXTRACT, I6_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_EXTRACT, I6_COMMENT_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_EXTRACT, I6_COMMENT_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT, I6_COMMENT_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_IN_ROUTINE_IN_EXTRACT, I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT, I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT, I6_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I6_COMMENT_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I6_COMMENT_IN_ROUTINE_IN_EXTRACT} +
  lexer_monoid{I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, I6_COMMENT_IN_ROUTINE_IN_EXTRACT};
