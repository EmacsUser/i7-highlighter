#ifndef BUFFER_HEADER
#define BUFFER_HEADER

#include "codepoints.hpp"
#include "token.hpp"
#include "monoid_sequence.hpp"
#include "relexer.hpp"

enum buffer_type {
  UNDECIDED_BUFFER,
  STORY_BUFFER,
  EXTENSION_BUFFER
};

class buffer {
protected:
  unsigned				buffer_number;
  buffer_type				type;
  i7_string				includable_file_name;
  monoid_sequence<token>		source_text;

public:
  buffer(unsigned buffer_number) :
    buffer_number{buffer_number},
    type{UNDECIDED_BUFFER} {}

protected:
  void rehighlight(const lexical_reference_points_from_edit&reference_points_from_edit);

public:
  void remove_codepoints(unsigned beginning, unsigned end);
  void add_codepoints(unsigned beginning, const i7_string&insertion);
};

#endif
