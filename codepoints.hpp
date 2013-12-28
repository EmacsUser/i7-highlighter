#ifndef CODEPOINTS
#define CODEPOINTS

// The highlighter processes all text as sequences of codepoints, effectively
// working in UCS-4/UTF-32 (the differences between the two not being relevant
// here).  This simplifies the code a lot, but also means that you can do some
// strange things, like make identifiers out of only combining diacritics.
//
// We classify codepoints into four categories: whitespace is defined exactly as
// in the Unicode standard, I7 punctuation comprises the characters that can
// participate in lexical delimiters other than a documentation break, and I7
// letters are everything else save TERMINATOR_CODEPOINT, which is reserved as
// an internal delimiter.
//
// If you want to write a string literal that the compiler will treat as a
// sequence of codepoints, use the ENCODE macro: ENCODE("string here").  There
// is also a function ASSUME_EIGHT_BIT that takes an i7_string and truncates all
// the characters to one byte.  This is to be used *only* for debugging.
//
// Finally, one of I7's goals is to account for idiosyncrasies that word
// processors might introduce if used to edit the source text.  This header also
// includes a function that reimplements its counter–word processor
// normalization.  (But note that we don't normalize newlines—that could give us
// a different codepoint count than our client, which would be all sorts of
// headaches.  Instead, we tell the lexer how to tokenize line breaks according
// to ni's feed_file_into_lexer.)

#include <string>
#include <sstream>

using i7_codepoint = char32_t;
using i7_string = std::u32string;
using i7_string_stream = std::basic_stringstream<i7_codepoint>;

// 0xFFFF is one of the codepoints that Unicode reserves for an application's
// internal use.
static const i7_codepoint TERMINATOR_CODEPOINT = 0xFFFF;
#define ENCODE(string_literal) U ## string_literal
std::string ASSUME_EIGHT_BIT(const i7_string&text);

bool is_whitespace(i7_codepoint codepoint);
bool is_i7_punctuation(i7_codepoint codepoint);
bool is_i7_letter(i7_codepoint codepoint);
bool is_i7_lexical_delimiter_letter(i7_codepoint codepoint);
i7_codepoint i7_normalize(i7_codepoint codepoint);

#endif
