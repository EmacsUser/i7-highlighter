#ifndef DELIMITERS_HEADER
#define DELIMITERS_HEADER

#include <cassert>
#include <vector>

#include "codepoints.hpp"
#include "lexer_monoid.hpp"
#include "token.hpp"
#include "paralleling_monoid.hpp"
#include "stack_monoid.hpp"

enum delimiter_class {
  I7_STRING_DELIMITER,	/*  " ... "  */
  I7_COMMENT_DELIMITER,	/*  [ ... ]  */
  I6_DELIMITER,		/* (- ... -) */
  I6_COMMENT_DELIMITER,	/*  ! ... \n */
  I7_DELIMITER		/* (+ ... +) */
};

class delimiter {
protected:
  using iterator_type = typename monoid_sequence<token>::iterator;
  ::delimiter_class			delimiter_class;
  iterator_type				position;
public:
  delimiter(::delimiter_class delimiter_class, const iterator_type&position) :
    delimiter_class{delimiter_class},
    position{position} {}

  ::delimiter_class get_delimiter_class() const {
    return delimiter_class;
  }
  const iterator_type&get_position() const {
    return position;
  }

  bool operator ==(const delimiter&other) const {
    return
      delimiter_class == other.delimiter_class &&
      position == other.position;
  }
};

class delimiter_monoid {
protected:
  using iterator_type = typename monoid_sequence<token>::iterator;
  paralleling_monoid<token>		position;
  stack_monoid<delimiter>		delimiter_effect;
  delimiter_monoid*			match;

public:
  delimiter_monoid(unsigned zero) :
    position{0},
    delimiter_effect{0},
    match{nullptr} { assert(!zero); }
  delimiter_monoid(const lexical_state&before, const iterator_type&position, const lexical_state&after);

  ////
};

#endif
