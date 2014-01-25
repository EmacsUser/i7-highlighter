#ifndef BUFFER_HEADER
#define BUFFER_HEADER

#include <unordered_set>

#include "codepoints.hpp"
#include "token.hpp"
#include "monoid_sequence.hpp"
#include "custom_multimap.hpp"
#include "relexer.hpp"

class session;
class parseme;
class nonterminal;
class match;

enum buffer_type {
  UNDECIDED_BUFFER,
  STORY_BUFFER,
  EXTENSION_BUFFER
};

class buffer {
protected:
  using token_sequence = monoid_sequence<token>;
  using token_iterator = typename token_sequence::iterator;
  typename ::session&			owner;
  unsigned				buffer_number;
  buffer_type				type;
  i7_string				includable_file_name;
  token_sequence			source_text;
  custom_multimap<const parseme*, token_iterator>
					parseme_beginnings;
  std::unordered_set<token_iterator>	sentence_endings;
  custom_multimap<const nonterminal*, const match*>
					partial_matches_by_need;

public:
  buffer(typename ::session&owner, unsigned buffer_number) :
    owner(owner),
    buffer_number{buffer_number},
    type{UNDECIDED_BUFFER} {}

protected:
  void parser_rehighlight_handler(token_iterator beginning, token_iterator end);
  void rehighlight(const lexical_reference_points_from_edit&reference_points_from_edit);

public:
  const std::unordered_set<token_iterator>&get_parseme_beginnings(const parseme&terminal);
  void add_terminal_beginning(token_iterator beginning);
  void remove_terminal_beginning(token_iterator beginning);
  void add_parseme_beginning(const ::parseme&parseme, token_iterator beginning);
  void remove_parseme_beginning(const ::parseme&parseme, token_iterator beginning);

  const std::unordered_set<token_iterator>&get_sentence_endings() const;
  void add_sentence_ending(token_iterator position);
  void remove_sentence_ending(token_iterator position);

  const std::unordered_set<const match*>&get_partial_matches_needing(const nonterminal*result) const;
  void add_partial_match(const match*partial_match);
  void remove_partial_match(const match*partial_match);

  void remove_codepoints(unsigned beginning, unsigned end);
  void add_codepoints(unsigned beginning, const i7_string&insertion);
};

#endif
