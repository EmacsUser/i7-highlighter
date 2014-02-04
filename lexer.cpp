#include "lexer.hpp"

using namespace std;

#define UPDATE_SIDE_STATE {						\
    if (state == &lexer::in_documentation_break) {			\
      documentation_break_match_count = 2; } }

#define ACCUMULATE() {				\
    accumulator << codepoint;			\
    return; }

#define ACCUMULATE_TO_STATE(next_state) {	\
    accumulator << codepoint;			\
    state = &lexer::next_state;			\
    UPDATE_SIDE_STATE;				\
    return; }

#define FLUSH(only_whitespace, effect, newlines) {					\
    results.push_back(token{accumulator.str(), only_whitespace, effect, newlines});	\
    accumulator.str(i7_string{}); }

#define ACCUMULATE_AND_FLUSH(only_whitespace, effect, newlines) {	\
    accumulator << codepoint;						\
    FLUSH(only_whitespace, effect, newlines);				\
    state = &lexer::undecided;						\
    return; }

#define FLUSH_AND_ACCUMULATE(only_whitespace, effect, newlines) {	\
    FLUSH(only_whitespace, effect, newlines);				\
    undecided(codepoint);						\
    return; }

#define RELEX(inhibitor) {					\
    inhibitor = true;						\
    i7_string earlier_codepoints = accumulator.str();		\
    accumulator.str(i7_string{});				\
    state = &lexer::undecided;					\
    for (i7_codepoint earlier_codepoint : earlier_codepoints) { \
      (*this) << earlier_codepoint;				\
    }								\
    (*this) << codepoint;					\
    return; }

lexer::lexer() :
  state(&lexer::undecided),
  documentation_break_inhibited{false},
  documentation_break_match_count{0} {}

void lexer::operator <<(i7_codepoint codepoint) {
  (this->*state)(codepoint);
}

bool lexer::most_recent_codepoint_did_not_combine() const {
  return const_cast<i7_string_stream&>(accumulator).tellp() == 1;
}

const vector<token>&lexer::get_results() const {
  return results;
}

void lexer::undecided(i7_codepoint codepoint) {
  switch (codepoint) {
  case TERMINATOR_CODEPOINT:
    return;
  case '\n':
    ACCUMULATE_TO_STATE(after_line_feed);
  case '\r':
    ACCUMULATE_TO_STATE(after_carriage_return);
  case '(':
    ACCUMULATE_TO_STATE(after_open_parenthesis);
  case '-':
    ACCUMULATE_TO_STATE(after_hyphen);
  case '+':
    ACCUMULATE_TO_STATE(after_plus);
  case '\'':
    ACCUMULATE_TO_STATE(after_single_quote);
  case '"':
    ACCUMULATE_AND_FLUSH(false, double_quote, 0);
  case '[':
    ACCUMULATE_AND_FLUSH(false, left_bracket, 0);
  case ']':
    ACCUMULATE_AND_FLUSH(false, right_bracket, 0);
  case '!':
    ACCUMULATE_AND_FLUSH(false, bang, 0);
  }
  if (is_i7_whitespace(codepoint)) {
    ACCUMULATE_TO_STATE(in_whitespace);
  }
  if (is_i7_punctuation(codepoint)) {
    ACCUMULATE_AND_FLUSH(false, plain_text, 0);
  }
  ACCUMULATE_TO_STATE(in_word);
}

void lexer::after_single_quote(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\'':
    FLUSH(false, single_quote, 0);
    ACCUMULATE_TO_STATE(after_two_single_quotes);
  }
  FLUSH_AND_ACCUMULATE(false, single_quote, 0);
}

void lexer::after_two_single_quotes(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\'':
    FLUSH(false, plain_text, 0);
    ACCUMULATE_AND_FLUSH(false, single_quote, 0);
  }
  FLUSH_AND_ACCUMULATE(false, single_quote, 0);
}

void lexer::in_whitespace(i7_codepoint codepoint) {
  if (is_i7_whitespace(codepoint) && codepoint != '\r' && codepoint != '\n') {
    ACCUMULATE();
  }
  FLUSH_AND_ACCUMULATE(true, plain_text, 0);
}

void lexer::after_carriage_return(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\n':
    ACCUMULATE_TO_STATE(after_newline);
  case '\t':
    ACCUMULATE_TO_STATE(in_indentation);
  case '-':
    if (documentation_break_inhibited) {
      documentation_break_inhibited = false;
    } else {
      ACCUMULATE_TO_STATE(in_documentation_break);
    }
    break;
  }
  FLUSH_AND_ACCUMULATE(true, bare_newline, 1);
}

void lexer::after_line_feed(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\r':
    ACCUMULATE_TO_STATE(after_newline);
  case '\t':
    ACCUMULATE_TO_STATE(in_indentation);
  case '-':
    if (documentation_break_inhibited) {
      documentation_break_inhibited = false;
    } else {
      ACCUMULATE_TO_STATE(in_documentation_break);
    }
    break;
  }
  FLUSH_AND_ACCUMULATE(true, bare_newline, 1);
}

void lexer::after_newline(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\t':
    ACCUMULATE_TO_STATE(in_indentation);
  case '-':
    if (documentation_break_inhibited) {
      documentation_break_inhibited = false;
    } else {
      ACCUMULATE_TO_STATE(in_documentation_break);
    }
    break;
  }
  FLUSH_AND_ACCUMULATE(true, bare_newline, 1);
}

void lexer::in_indentation(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\t':
    ACCUMULATE();
  }
  FLUSH_AND_ACCUMULATE(true, indentation, 1);
}

void lexer::in_word(i7_codepoint codepoint) {
  if (is_i7_letter(codepoint)) {
    ACCUMULATE();
  }
  FLUSH_AND_ACCUMULATE(false, plain_text, 0);
}

void lexer::after_open_parenthesis(i7_codepoint codepoint) {
  switch (codepoint) {
  case '-':
    ACCUMULATE_AND_FLUSH(false, left_cyclops, 0);
  case '+':
    ACCUMULATE_AND_FLUSH(false, left_crosseyed_cyclops, 0);
  }
  FLUSH_AND_ACCUMULATE(false, plain_text, 0);
}

void lexer::after_hyphen(i7_codepoint codepoint) {
  switch (codepoint) {
  case ')':
    ACCUMULATE_AND_FLUSH(false, right_cyclops, 0);
  }
  FLUSH_AND_ACCUMULATE(false, plain_text, 0);
}

void lexer::after_plus(i7_codepoint codepoint) {
  switch (codepoint) {
  case ')':
    ACCUMULATE_AND_FLUSH(false, right_crosseyed_cyclops, 0);
  }
  FLUSH_AND_ACCUMULATE(false, plain_text, 0);
}

void lexer::in_documentation_break(i7_codepoint codepoint) {
  if (documentation_break_match_count == 24) {
    switch (codepoint) {
    case '\n':
      ACCUMULATE_TO_STATE(after_documentation_break_ended_by_line_feed);
    case '\r':
      ACCUMULATE_TO_STATE(after_documentation_break_ended_by_carriage_return);
    }
  } else if (codepoint == static_cast<i7_codepoint>("\n---- DOCUMENTATION ----\n"[documentation_break_match_count])) {
    ++documentation_break_match_count;
    ACCUMULATE();
  }
  RELEX(documentation_break_inhibited);
}

void lexer::after_documentation_break_ended_by_carriage_return(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\n':
    ACCUMULATE_TO_STATE(after_documentation_break_ended_by_newline);
  case '\t':
    ACCUMULATE_TO_STATE(in_indentation_after_documentation_break);
  }
  FLUSH_AND_ACCUMULATE(false, documentation_break, 2);
}

void lexer::after_documentation_break_ended_by_line_feed(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\r':
    ACCUMULATE_TO_STATE(after_documentation_break_ended_by_newline);
  case '\t':
    ACCUMULATE_TO_STATE(in_indentation_after_documentation_break);
  }
  FLUSH_AND_ACCUMULATE(false, documentation_break, 2);
}

void lexer::after_documentation_break_ended_by_newline(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\t':
    ACCUMULATE_TO_STATE(in_indentation_after_documentation_break);
  }
  FLUSH_AND_ACCUMULATE(false, documentation_break, 2);
}

void lexer::in_indentation_after_documentation_break(i7_codepoint codepoint) {
  switch (codepoint) {
  case '\t':
    ACCUMULATE();
  }
  FLUSH_AND_ACCUMULATE(false, documentation_break_followed_by_indentation, 2);
}
