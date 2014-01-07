#ifndef TOKEN_HEADER
#define TOKEN_HEADER

#include <iostream>

#include "codepoints.hpp"
#include "lexer_monoid.hpp"
#include "internalizer.hpp"
#include "annotation.hpp"

extern internalizer<i7_string>vocabulary;

/* The token class represents lexical tokens as elements of a product monoid for
   storage in a monoid_sequence. */
class token : public annotatable {
protected:
  unsigned				codepoint_count;
  unsigned				line_count;
  // Text may be null; it is not preserved by addition.
  const i7_string*			text;
  lexer_monoid				lexical_effect;

  // The addition constructor.
  token(const token&left, const token&right);

public:
  token();
  token(unsigned codepoint_count);
  token(const i7_string&text, const lexer_monoid&lexical_effect, unsigned line_count);
  token(const token&copy);
  ~token();

  token&operator =(const token&copy);

  unsigned get_codepoint_count() const;
  unsigned get_line_count() const;
  const i7_string*get_text() const;
  const lexer_monoid&get_lexical_effect() const;

  bool operator <(const token&other) const;

  token operator +(const token&other) const;
  token&operator +=(const token&other);

  friend std::ostream&operator <<(std::ostream&out, const ::token&token);
};

#endif

