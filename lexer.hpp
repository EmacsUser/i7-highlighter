#ifndef LEXER_HEADER
#define LEXER_HEADER

#include <vector>

#include "codepoints.hpp"
#include "lexer_monoid.hpp"
#include "token.hpp"

class lexer {
protected:
  using codepoint_handler = void (lexer::*)(i7_codepoint);
  codepoint_handler			state;
  bool					documentation_break_inhibited;
  // The match count is only meaningful when the state is
  // in_documentation_break.
  unsigned				documentation_break_match_count;
  i7_string_stream			accumulator;
  std::vector<token>			results;

public:
  lexer();
  void operator <<(i7_codepoint codepoint);
  bool most_recent_codepoint_did_not_combine() const;
  const std::vector<token>&get_results() const;

protected:
  void undecided(i7_codepoint codepoint);
  void after_single_quote(i7_codepoint codepoint);
  void after_two_single_quotes(i7_codepoint codepoint);
  void in_whitespace(i7_codepoint codepoint);
  void after_carriage_return(i7_codepoint codepoint);
  void after_line_feed(i7_codepoint codepoint);
  void after_newline(i7_codepoint codepoint);
  void in_indentation(i7_codepoint codepoint);
  void in_word(i7_codepoint codepoint);
  void after_open_parenthesis(i7_codepoint codepoint);
  void after_hyphen(i7_codepoint codepoint);
  void after_plus(i7_codepoint codepoint);
  void in_documentation_break(i7_codepoint codepoint);
  void after_documentation_break_ended_by_carriage_return(i7_codepoint codepoint);
  void after_documentation_break_ended_by_line_feed(i7_codepoint codepoint);
  void after_documentation_break_ended_by_newline(i7_codepoint codepoint);
  void in_indentation_after_documentation_break(i7_codepoint codepoint);
};

#endif
