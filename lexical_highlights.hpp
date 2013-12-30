#ifndef LEXICAL_HIGHLIGHTS_HEADER
#define LEXICAL_HIGHLIGHTS_HEADER

#include "codepoints.hpp"
#include "lexer_monoid.hpp"

// Because our communication with the editor is over a codepoint stream,
// highlight codes need to have the same width as codepoints.
using highlight_code = i7_codepoint;

highlight_code get_highlight_code(lexical_state before, lexical_state after);

#endif
