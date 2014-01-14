#include <vector>

#include "buffer.hpp"
#include "lexical_highlights.hpp"
#include "io.hpp"
#include "parser.hpp"

using namespace std;

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
  for (monoid_sequence<token>::iterator j = reference_points_from_edit.start_of_relexed_text; j != i; ++j) {
    token_available available{owner, this, j};
    available.surreptitiously_make_false();
    assert(j->has_annotation(available));
  }
  if (reference_points_from_edit.start_of_relexed_text.can_decrement()) {
    monoid_sequence<token>::iterator k = reference_points_from_edit.start_of_relexed_text;
    --k;
    next_token next{owner, k, reference_points_from_edit.start_of_relexed_text};
    next.justify();
  }
  for (monoid_sequence<token>::iterator j = reference_points_from_edit.start_of_relexed_text, k = j; j != i; j = k) {
    ++k;
    token_available available{owner, this, j};
    available.justify();
    next_token next{owner, j, k};
    next.justify();
    assert(!k.can_increment() || previous(k) == j);
  }
  //
  if (highlight_codepoint_index_before < codepoint_index_before) {
    new_highlights.push_back({ highlight_codepoint_index_before, codepoint_index_before, highlight_before });
  }
  remove_highlights(buffer_number, initial_codepoint_index, codepoint_index_before);
  for (const ::highlight&highlight : new_highlights) {
    add_highlight(buffer_number, highlight.beginning_codepoint_index, highlight.end_codepoint_index, highlight.highlight_code);
  }
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

const unordered_set<token_iterator>&buffer::get_parseme_beginnings(const parseme&terminal) {
  return parseme_beginnings[parseme_bank.lookup(terminal)];
}

void buffer::remove_codepoints(unsigned beginning, unsigned end) {
  rehighlight(::remove_codepoints(source_text, beginning, end));
}

void buffer::add_codepoints(unsigned beginning, const i7_string&insertion) {
  rehighlight(::add_codepoints(source_text, beginning, insertion));
}
