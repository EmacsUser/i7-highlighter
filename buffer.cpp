#include <vector>

#include "buffer.hpp"
#include "lexical_highlights.hpp"
#include "io.hpp"
#include "parser.hpp"

using namespace std;

static bool is_plain_i7_or_documentation(lexical_state state) {
  if (!state.get_comment_depth()) {
    switch (state.get_superstate()) {
    case I7:
    case I7_EXTENSION_DOCUMENTATION:
    case I7_IN_EXTRACT:
      return true;
    }
  }
  return false;
}

static bool begins_with_a_digit(const i7_string*text) {
  return text->size() && is_i7_digit((*text)[0]);
}

static bool ends_with_a_digit(const i7_string*text) {
  return text->size() && is_i7_digit((*text)[text->size() - 1]);
}

static bool should_be_a_sentence_end(token_iterator position, token_iterator next) {
  static const i7_string*FULL_STOP = &vocabulary.acquire(ENCODE("."));
  static const i7_string*SEMICOLON = &vocabulary.acquire(ENCODE(";"));
  static const i7_string*COLON = &vocabulary.acquire(ENCODE(":"));
  if (next.can_increment() && (position->get_line_count() == 1) && next->get_line_count()) {
    // Found a paragraph break.
    return true;
  }
  const i7_string*text = position->get_text();
  if (text == FULL_STOP || text == SEMICOLON || text == COLON) {
    // Check that the sentence-ending punctuation is not an intra-KOV delimiter.
    if (!next.can_increment() || !begins_with_a_digit(next->get_text()) || !position.can_decrement()) {
      return true;
    }
    token_iterator previous = position;
    --previous;
    return !ends_with_a_digit(previous->get_text());
  }
  return false;
}

static token_iterator previous_by_skipping_whitespace(token_iterator position) {
  token_iterator result = position;
  do {
    if (!result.can_decrement()) {
      return result.move_to_end();
    }
    --result;
  } while (result->is_only_whitespace());
  return result;
}

static token_iterator next_by_skipping_whitespace(token_iterator position) {
  token_iterator result = position;
  while ((++result).can_increment() && result->is_only_whitespace());
  return result;
}

void buffer::parser_rehighlight_handler(lexical_state beginning_state, token_iterator beginning, token_iterator end) {
  // Step I: Make sure that negative and combined annotation facts are false.
  for (token_iterator i = beginning; i != end; ++i) {
    token_available available{owner, this, i};
    available.surreptitiously_make_false();
    assert(i->has_annotation(available));
    end_of_sentence not_an_end{owner, this, i, false};
    not_an_end.justify(); // Justify the negation.
    assert(i->has_annotation(not_an_end));
  }
  // Step II: Correct observations on the edges.
  if (beginning.can_decrement()) {
    token_iterator previous = beginning;
    --previous;
    ::end_of_sentence end_of_sentence{owner, this, previous, true};
    if (should_be_a_sentence_end(previous, beginning)) {
      if (!end_of_sentence) {
	end_of_sentence.justify();
      }
    } else if (end_of_sentence) {
      end_of_sentence.unjustify();
    }
  }
  if (end.can_increment()) {
    token_iterator next = end;
    --next;
    ::end_of_sentence end_of_sentence{owner, this, end, true};
    if (should_be_a_sentence_end(end, next)) {
      if (!end_of_sentence) {
	end_of_sentence.justify();
      }
    } else if (end_of_sentence) {
      end_of_sentence.unjustify();
    }
  }
  // Step IIIa: Make end-of-sentence observations true.  (We do these first for performance reasons.)
  lexical_state state = beginning_state;
  for (monoid_sequence<token>::iterator i = beginning, j = i; i != end; i = j) {
    ++j;
    // end_of_sentence
    if (is_plain_i7_or_documentation(state)) {
      if (should_be_a_sentence_end(i, j)) {
	::end_of_sentence end_of_sentence{owner, this, i, true};
	end_of_sentence.justify();
      }
    }
    state = i->get_lexical_effect()(state);
  }
  // Step IIIb: Make other observations true.
  state = beginning_state;
  token_iterator previous = previous_by_skipping_whitespace(beginning);
  for (monoid_sequence<token>::iterator i = beginning, j = i; i != end; i = j) {
    ++j;
    // token_available
    token_available available{owner, this, i};
    available.justify();
    // next_token
    if (!i->is_only_whitespace()) {
      ::next_token next_token{owner, this, previous, i};
      next_token.justify();
      assert(::previous(i) == previous);
      previous = i;
    }
    state = i->get_lexical_effect()(state);
  }
  if (end.can_decrement() && previous != beginning) {
    token_iterator inclusive_end = end;
    --inclusive_end;
    token_iterator next = next_by_skipping_whitespace(inclusive_end);
    if (previous.can_increment() || next.can_increment()) {
      ::next_token next_token{owner, this, previous, next};
      next_token.justify();
      assert(!next.can_increment() || ::previous(next) == previous);
    }
  }
}

namespace {
  struct highlight {
    unsigned				beginning_codepoint_index;
    unsigned				end_codepoint_index;
    ::highlight_code			highlight_code;
  };
}

static const highlight_code INVALID_HIGHLIGHT = 0xFFFFFFFF;

void buffer::rehighlight(const lexical_reference_points_from_edit&reference_points_from_edit) {
  if (reference_points_from_edit.start_of_relexed_text == source_text.end()) {
    return;
  }
  unsigned initial_codepoint_index = source_text.sum_over_interval(source_text.begin(), reference_points_from_edit.start_of_relexed_text).get_codepoint_count();
  vector<highlight>new_highlights;
  bool done_with_relexed_portion = false;
  //
  unsigned codepoint_index_before = initial_codepoint_index;
  lexical_state old_lexical_state_before = reference_points_from_edit.old_post_relex_state;
  lexical_state lexical_state_before = reference_points_from_edit.pre_relex_state;
  unsigned highlight_codepoint_index_before = codepoint_index_before;
  uint32_t highlight_before = INVALID_HIGHLIGHT;
  monoid_sequence<token>::iterator i = reference_points_from_edit.start_of_relexed_text;
  for (; i != source_text.end(); ++i) {
    const lexer_monoid&lexical_effect = i->get_lexical_effect();
    done_with_relexed_portion |= (i == reference_points_from_edit.end_of_relexed_text);
    if (done_with_relexed_portion) {
      if (old_lexical_state_before == lexical_state_before) {
	break;
      }
      old_lexical_state_before = lexical_effect(old_lexical_state_before);
    }
    lexical_state lexical_state_after = lexical_effect(lexical_state_before);
    uint32_t highlight_after = get_highlight_code(lexical_state_before, lexical_state_after);
    if (highlight_before != highlight_after) {
      if (highlight_before != INVALID_HIGHLIGHT) {
	new_highlights.push_back({ highlight_codepoint_index_before, codepoint_index_before, highlight_before });
      }
      highlight_codepoint_index_before = codepoint_index_before;
    }
    codepoint_index_before += i->get_codepoint_count();
    lexical_state_before = lexical_state_after;
    highlight_before = highlight_after;
  }
  //
  parser_rehighlight_handler(reference_points_from_edit.pre_relex_state, reference_points_from_edit.start_of_relexed_text, i);
  //
  if (highlight_codepoint_index_before < codepoint_index_before) {
    new_highlights.push_back({ highlight_codepoint_index_before, codepoint_index_before, highlight_before });
  }
  remove_highlights(buffer_number, initial_codepoint_index, codepoint_index_before);
  for (const ::highlight&highlight : new_highlights) {
    add_highlight(buffer_number, highlight.beginning_codepoint_index, highlight.end_codepoint_index, highlight.highlight_code);
  }
}

const unordered_set<token_iterator>&buffer::get_parseme_beginnings(const parseme&terminal) {
  return parseme_beginnings[parseme_bank.lookup(terminal)];
}

void buffer::add_terminal_beginning(token_iterator beginning) {
  parseme_beginnings.insert(&parseme_bank.acquire(token_terminal{*beginning->get_text()}), beginning);
}

void buffer::remove_terminal_beginning(token_iterator beginning) {
  const parseme*key = parseme_bank.lookup(token_terminal{*beginning->get_text()});
  parseme_beginnings.erase(key, beginning);
  parseme_bank.release(*key);
}

void buffer::add_parseme_beginning(const ::parseme&parseme, token_iterator beginning) {
  parseme_beginnings.insert(&parseme_bank.acquire(parseme), beginning);
}

void buffer::remove_parseme_beginning(const ::parseme&parseme, token_iterator beginning) {
  const ::parseme*key = parseme_bank.lookup(parseme);
  parseme_beginnings.erase(key, beginning);
  parseme_bank.release(*key);
}

const unordered_set<token_iterator>&buffer::get_sentence_endings() const {
  return sentence_endings;
}

void buffer::add_sentence_ending(token_iterator position) {
  sentence_endings.insert(position);
}

void buffer::remove_sentence_ending(token_iterator position) {
  sentence_endings.erase(position);
}

const unordered_set<const match*>&buffer::get_partial_matches_needing(const nonterminal*result) const {
  return partial_matches_by_need[result];
}

void buffer::add_partial_match(const match*partial_match) {
  for (const parseme*alternative : partial_match->get_continuing_alternatives()) {
    const nonterminal*need = dynamic_cast<const nonterminal*>(alternative);
    if (need) {
      partial_matches_by_need.insert(need, partial_match);
    }
  }
}

void buffer::remove_partial_match(const match*partial_match) {
  for (const parseme*alternative : partial_match->get_continuing_alternatives()) {
    const nonterminal*need = dynamic_cast<const nonterminal*>(alternative);
    if (need) {
      partial_matches_by_need.erase(need, partial_match);
    }
  }
}

void buffer::remove_codepoints(unsigned beginning, unsigned end) {
  rehighlight(::remove_codepoints(source_text, beginning, end));
}

void buffer::add_codepoints(unsigned beginning, const i7_string&insertion) {
  rehighlight(::add_codepoints(source_text, beginning, insertion));
}

ostream&operator <<(ostream&out, const ::buffer&buffer) {
  out << "BEGIN Buffer " << buffer.buffer_number << endl;
  for (auto i = buffer.source_text.begin(), end = buffer.source_text.end(); i != end; ++i) {
    out << "Annotations on " << *i << ":" << endl;
    i->dump(out);
  }
  return out << "END Buffer " << buffer.buffer_number << endl;
}
