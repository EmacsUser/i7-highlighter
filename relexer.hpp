#ifndef RELEXER_HEADER
#define RELEXER_HEADER

#include "codepoints.hpp"
#include "lexer_monoid.hpp"
#include "token.hpp"
#include "monoid_sequence.hpp"
#include "lexer.hpp"

using token_sequence = monoid_sequence<token>;
using token_iterator = typename token_sequence::iterator;

struct lexical_reference_points_from_edit {
  lexical_state pre_relex_state;
  token_iterator start_of_relexed_text;
  token_iterator end_of_relexed_text;
  lexical_state old_post_relex_state;
};

lexical_reference_points_from_edit remove_text(token_sequence&source_text, unsigned beginning_codepoint_index, unsigned end_codepoint_index);
lexical_reference_points_from_edit insert_text(token_sequence&source_text, unsigned beginning_codepoint_index, const i7_string&insertion);

#endif
