#ifndef BUFFER_HEADER
#define BUFFER_HEADER

#include <unordered_set>

#include "codepoints.hpp"
#include "token.hpp"
#include "monoid_sequence.hpp"
#include "custom_multimap.hpp"
#include "relexer.hpp"

class parseme;

enum buffer_type {
  UNDECIDED_BUFFER,
  STORY_BUFFER,
  EXTENSION_BUFFER
};

class buffer {
protected:
  using token_sequence = monoid_sequence<token>;
  using token_iterator = typename token_sequence::iterator;
  unsigned				buffer_number;
  buffer_type				type;
  i7_string				includable_file_name;
  token_sequence			source_text;
  custom_multimap<const parseme*, token_iterator>
					parseme_beginnings;

public:
  buffer(unsigned buffer_number) :
    buffer_number{buffer_number},
    type{UNDECIDED_BUFFER} {}

protected:
  void rehighlight(const lexical_reference_points_from_edit&reference_points_from_edit);

public:
  void add_terminal_beginning(token_iterator beginning);
  void remove_terminal_beginning(token_iterator beginning);
  void add_parseme_beginning(const ::parseme&parseme, token_iterator beginning);
  void remove_parseme_beginning(const ::parseme&parseme, token_iterator beginning);
  const std::unordered_set<token_iterator>&get_parseme_beginnings(const parseme&terminal);

  void remove_codepoints(unsigned beginning, unsigned end);
  void add_codepoints(unsigned beginning, const i7_string&insertion);
};

#endif
