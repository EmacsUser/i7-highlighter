#ifndef LEXER_MONOID_HEADER
#define LEXER_MONOID_HEADER

#include <cstdint>
#include <vector>
#include <iostream>

/* A lexical superstate partially charactizes a point between two codepoints: it
 * tells us what sort of lexical entities that position falls within, except
 * that it omits I7 comments, as these can nest infinitely.  Seen another way,
 * if the lexer is a pushdown automaton, then the lexical superstate is the
 * automaton's current vertex, and the I7 comment nesting level is the depth of
 * its stack.
 *
 * We define them with a macro for the sake of initializers in lexer_monoid.cpp.
 *
 * In the following constant names, ``extract'' means any code embedded in
 * documentation, whether pastable or not, part of an example or not.  ``Movable
 * comment levels'' are comment levels that may be preserved across a superstate
 * change.
 */

#define LEXICAL_SUPERSTATE_LIST \
  /* States involving neither I6 nor an extract */ \
  I7, /* can take I7 comment levels (occurrence 0) */ \
  I7_STRING, \
  I7_SUBSTITUTION, \
  I7_EXTENSION_DOCUMENTATION, /* can take I7 comment levels (occurrence 1) */ \
  /* States involving I6 without a routine context or embedded I7 */ \
  I6, \
  I6_CHARACTER, \
  I6_STRING, \
  I6_COMMENT, \
  /* States involving I6 with a routine context but without embedded I7 */ \
  I6_IN_ROUTINE, \
  I6_CHARACTER_IN_ROUTINE, \
  I6_STRING_IN_ROUTINE, \
  I6_COMMENT_IN_ROUTINE, \
  /* States involving I6 without a routine context but with embedded I7 */ \
  I7_IN_I6, /* can take I7 comment levels (occurrence 2) */ \
  I7_STRING_IN_I6, \
  I7_SUBSTITUTION_IN_I6, \
  I7_IN_I6_COMMENT, /* can take movable I7 comment levels (occurrences 3 and 0) */ \
  I7_STRING_IN_I6_COMMENT, \
  I7_SUBSTITUTION_IN_I6_COMMENT, \
  /* States involving I6 with a routine context and embedded I7 */ \
  I7_IN_I6_IN_ROUTINE, /* can take I7 comment levels (occurrence 4) */ \
  I7_STRING_IN_I6_IN_ROUTINE, \
  I7_SUBSTITUTION_IN_I6_IN_ROUTINE, \
  I7_IN_I6_COMMENT_IN_ROUTINE, /* can take movable I7 comment levels (occurrences 5 and 1) */ \
  I7_STRING_IN_I6_COMMENT_IN_ROUTINE, \
  I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE, \
  /* States involving no I6, but in an extract */ \
  I7_IN_EXTRACT, /* can take I7 comment levels (occurrence 6) */ \
  I7_STRING_IN_EXTRACT, \
  I7_SUBSTITUTION_IN_EXTRACT, \
  /* States involving I6 in an extract and without a routine context or embedded I7 */ \
  I6_IN_EXTRACT, \
  I6_CHARACTER_IN_EXTRACT, \
  I6_STRING_IN_EXTRACT, \
  I6_COMMENT_IN_EXTRACT, \
  /* States involving I6 in an extract and with a routine context but without embedded I7 */ \
  I6_IN_ROUTINE_IN_EXTRACT, \
  I6_CHARACTER_IN_ROUTINE_IN_EXTRACT, \
  I6_STRING_IN_ROUTINE_IN_EXTRACT, \
  I6_COMMENT_IN_ROUTINE_IN_EXTRACT, \
  /* States involving I6 in an extract and without a routine context but with embedded I7 */ \
  I7_IN_I6_IN_EXTRACT, /* can take I7 comment levels (occurrence 7) */ \
  I7_STRING_IN_I6_IN_EXTRACT, \
  I7_SUBSTITUTION_IN_I6_IN_EXTRACT, \
  I7_IN_I6_COMMENT_IN_EXTRACT, /* can take movable I7 comment levels (occurrences 8 and 2) */ \
  I7_STRING_IN_I6_COMMENT_IN_EXTRACT, \
  I7_SUBSTITUTION_IN_I6_COMMENT_IN_EXTRACT, \
  /* States involving I6 in an extract with a routine context and embedded I7 */ \
  I7_IN_I6_IN_ROUTINE_IN_EXTRACT, /* can take I7 comment levels (occurrence 9) */ \
  I7_STRING_IN_I6_IN_ROUTINE_IN_EXTRACT, \
  I7_SUBSTITUTION_IN_I6_IN_ROUTINE_IN_EXTRACT, \
  I7_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, /* can take movable I7 comment levels (occurrences 10 and 3) */ \
  I7_STRING_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT, \
  I7_SUBSTITUTION_IN_I6_COMMENT_IN_ROUTINE_IN_EXTRACT

enum lexical_superstate { LEXICAL_SUPERSTATE_LIST, LEXICAL_SUPERSTATE_COUNT };

static const unsigned COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS = 11;
static const unsigned COUNT_OF_LEXICAL_SUPERSTATES_WITH_MOVABLE_I7_COMMENT_LEVELS = 4;

std::ostream&operator <<(std::ostream&out, lexical_superstate superstate);

/* A lexical state completes a lexical superstate by adding comment nesting
 * information.  At the moment, nesting levels are actually bounded, but not
 * severely.
 *
 * Lexical state implicitly cast to and from numbers; as numbers, they match
 * lexical superstates in the range [0..LEXICAL_SUPERSTATE_COUNT).
 */
class lexical_state {
protected:
  // For space-saving and alignment purposes, we ask that exactly eight bits be
  // used to store the superstate and exactly eight bits be used to store the
  // comment depth.
  uint8_t				superstate;
  uint8_t				comment_depth;
public:
  lexical_state(lexical_superstate superstate) :
    superstate{superstate},
    comment_depth{0} {}
  lexical_state(lexical_superstate superstate, unsigned comment_depth) :
    superstate{superstate},
    comment_depth{static_cast<uint8_t>(comment_depth)} {}
  lexical_superstate get_superstate() const { return static_cast<lexical_superstate>(superstate); }
  unsigned get_comment_depth() const { return comment_depth; }
  bool operator ==(const lexical_state&other) const { return superstate == other.superstate && comment_depth == other.comment_depth; }
  bool operator !=(const lexical_state&other) const { return superstate != other.superstate || comment_depth != other.comment_depth; }
  lexical_state&operator --() { --superstate; return *this; }
  lexical_state&operator ++() { ++superstate; return *this; }
  lexical_state operator --(int) { return {static_cast<lexical_superstate>(superstate--), comment_depth}; }
  lexical_state operator ++(int) { return {static_cast<lexical_superstate>(superstate++), comment_depth}; }
};

std::ostream&operator <<(std::ostream&out, lexical_state state);

/* Just as all groups can be modeled as a set of permutations on a common set,
 * all monoids can be modeled as a set of endomorphisms.  In this particular
 * monoid, the elements are functions on objects of type lexical_state.  Or,
 * from the pushdown automaton perspective, each element of this type describes
 * the automaton's transitions under some input string.
 */
class lexer_monoid {
protected:
  // The images array maps each superstate to the lexical_state that will be
  // reached after applying this element at a comment depth of zero.
  lexical_state				images[LEXICAL_SUPERSTATE_COUNT];
  // The comment_depth_change tells how much the element will change a
  // lexical_state's comment depth provided that the depth starts high enough.
  int8_t				comment_depth_change;
  // The comment_images vectors describe what lexical_states are reached from
  // lower comment depths.  The array is indexed by occurrence numbers among the
  // superstates that can take comments, while the vectors are indexed by
  // comment depth minus one.
  std::vector<lexical_state>		comment_images[COUNT_OF_LEXICAL_SUPERSTATES_WITH_I7_COMMENT_LEVELS];

protected:
  // A helper function abstracting repeated code in the () operator.
  void compose(std::vector<lexical_state>&composition_comment_images, lexical_superstate superstate, const std::vector<lexical_state>&own_comment_images, const lexer_monoid&other, const std::vector<lexical_state>&other_comment_images) const;

public:
  lexer_monoid(int8_t comment_depth_change);
  lexer_monoid(lexical_superstate from, lexical_superstate to, bool also_in_reverse = false);
  lexer_monoid(const lexer_monoid&copy);
  lexer_monoid&operator =(const lexer_monoid&other);
  lexical_state operator ()(lexical_state state) const;
  // Compose two lexer_monoids.  ``f + g'' is interpreted as ``g of f'', since that works best with the monoid sequence data structure.
  lexer_monoid operator +(const lexer_monoid&other) const;
  lexer_monoid&operator +=(const lexer_monoid&other);
  friend std::ostream&operator <<(std::ostream&out, const lexer_monoid&element);
};

std::ostream&operator <<(std::ostream&out, const lexer_monoid&element);

extern const lexical_state INITIAL_LEXICAL_STATE;

extern lexer_monoid plain_text;			/* (anything not appearing below) */
extern lexer_monoid double_quote;		/* " */
extern lexer_monoid left_bracket;		/* [ */
extern lexer_monoid right_bracket;		/* ] */
extern lexer_monoid documentation_break;	/* ^J---- DOCUMENTATION ----^J */
extern lexer_monoid documentation_break_followed_by_indentation;
						/* ^J---- DOCUMENTATION ----^J^I or ...^J^I^I or ... */
extern lexer_monoid indentation;		/* ^J^I*: or ^J^I or ^J^I^I or ... */
extern lexer_monoid bare_newline;		/* ^J */
extern lexer_monoid left_cyclops;		/* (- */
extern lexer_monoid right_cyclops;		/* -) */
extern lexer_monoid single_quote;		/* ' (when not the middle ' in ''') */
extern lexer_monoid bang;			/* ! */
extern lexer_monoid left_crosseyed_cyclops;	/* (+ */
extern lexer_monoid right_crosseyed_cyclops;	/* +) */

#endif
