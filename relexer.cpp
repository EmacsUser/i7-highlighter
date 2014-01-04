#include <cassert>

#include "relexer.hpp"

#define BACKUP_OVER(string) \
  if (iterator->get_text() != string) return; \
  --iterator;

static void backup_to_look_for_documentation_break(token_iterator&iterator, i7_codepoint first_codepoint_of_altered_text, bool have_already_effectively_backed_up_once) {
  static const i7_string*HYPHEN = &vocabulary.acquire(ENCODE("-"));
  static const i7_string*SPACE = &vocabulary.acquire(ENCODE(" "));
  static const i7_string*DOCUMENTATION = &vocabulary.acquire(ENCODE("DOCUMENTATION"));
  // While this function is called to look for documentation breaks, it is
  // always possible that the codepoint will combine in some more mundane way,
  // and checking for that requires at least one backup.  We get it out of the
  // way.
  --iterator;
  // Now, typing in the middle of a token can only form a documentation break
  // with earlier codepoints if it inserts letters to form DOCUMENTATION in the
  // middle.  In that case, we've already backed up past the other letters, so
  // we can fall through and pretend that the insertion wasn't in the middle.
  // Otherwise we're done.
  if (have_already_effectively_backed_up_once && !is_i7_lexical_delimiter_letter(first_codepoint_of_altered_text)) {
    return;
  }
  // For insertions not in the middle of a token, there are four cases.
  if (is_i7_lexical_delimiter_letter(first_codepoint_of_altered_text)) {
    // First, suppose the insertion begins with letters, which might be
    // DOCUMENTATION.  Then we still need to rewind past the prior space and the
    // four hyphens and get to the newline.
    goto from_documentation;
  }
  switch (first_codepoint_of_altered_text) {
  case '\n':
    // Newlines are similar, except that we have more tokens to cross.
    goto from_newline;
  case ' ':
    // Now we get into more complicated cases.  If we have a space, we don't
    // know which one it might be, so we have to consider both possibilities.
    if (iterator->get_text() == DOCUMENTATION) {
      goto from_second_space;
    }
    goto from_first_space;
  case '-':
    // And finally, if we have a hyphen, it could be in any of eight positions.
    // We backup over up to three prior hyphens, which will either be enough or
    // will get us to the second space, depending on which hyphen group we're
    // in.
    if (iterator->get_text() == HYPHEN) {
      --iterator;
      if (iterator->get_text() == HYPHEN) {
	--iterator;
	if (iterator->get_text() == HYPHEN) {
	  --iterator;
	}
      }
    }
    goto from_hyphens;
  }
  assert(false);
 from_newline:
  BACKUP_OVER(HYPHEN);
  BACKUP_OVER(HYPHEN);
  BACKUP_OVER(HYPHEN);
  BACKUP_OVER(HYPHEN);
 from_hyphens:
  BACKUP_OVER(SPACE);
 from_second_space:
  BACKUP_OVER(DOCUMENTATION);
 from_documentation:
  BACKUP_OVER(SPACE);
 from_first_space:
  BACKUP_OVER(HYPHEN);
  BACKUP_OVER(HYPHEN);
  BACKUP_OVER(HYPHEN);
  BACKUP_OVER(HYPHEN);
}

static void backup_to_relexing_point(token_iterator&iterator, i7_codepoint first_codepoint_of_altered_text, bool have_already_effectively_backed_up_once) {
  static const i7_string*SINGLE_QUOTE = &vocabulary.acquire(ENCODE("'"));
  // First we deal with anything that might combine to form a documentation break; this is the most complicated.
  if (first_codepoint_of_altered_text == ' ' || first_codepoint_of_altered_text == '-' || first_codepoint_of_altered_text == '\n' || is_i7_lexical_delimiter_letter(first_codepoint_of_altered_text)) {
    backup_to_look_for_documentation_break(iterator, first_codepoint_of_altered_text, have_already_effectively_backed_up_once);
    return;
  }
  // Then we dispatch other combining punctuation.
  switch (first_codepoint_of_altered_text) {
  case '+':  // We need to decide between + and (+.
  case ')':  // We need to decide between ) and -) and +).
    // Backup once to look for a preceding parenthesis, hyphen, or plus.
    if (!have_already_effectively_backed_up_once) {
      --iterator;
    }
    return;
  case '\'':
    // Single quotes do not combine, but, because of the I6 parsing rule for a
    // single quote character literal (written '''), they do change lexical
    // effect depending on the number of immediately preceding occurrences.  So
    // we need to backup accoss any earlier single quotes to make sure that we
    // count correctly.
    //
    // The have_already_effectively_backed_up_once flag means that we began in
    // the middle of a multicodepoint token, which means that there aren't any
    // preceding single quotes because they're always lexed into one-codepoint
    // tokens.  Thereore, we only look behind when the flag is cleared.
    if (!have_already_effectively_backed_up_once) {
      while (iterator.can_decrement()) {
	token_iterator probe = iterator;
	if ((--probe)->get_text() != SINGLE_QUOTE) {
	  break;
	}
	--iterator;
      }
    }
    return;
  }
  // And everything else can combine at most once, unless it's punctuation, in which case it can't combine at all.
  if (!have_already_effectively_backed_up_once && !is_i7_punctuation(first_codepoint_of_altered_text)) {
    --iterator;
  }
}

static lexical_reference_points_from_edit no_reference_points_from_edit(token_sequence&source_text) {
  // The use of INITIAL_LEXICAL_STATE here may well be a lie.  However, because
  // the interval is empty, at the end, and marked with identical states, it
  // should be a benign one.
  return { INITIAL_LEXICAL_STATE, source_text.end(), source_text.end(), INITIAL_LEXICAL_STATE };
}

static lexical_reference_points_from_edit add_codepoints(token_sequence&source_text, token_iterator insertion_point, unsigned insertion_offset, const i7_string&insertion, lexical_state old_post_relex_state) {
  assert(insertion.size());
  lexer insertion_lexer;
  token_iterator relexing_point = insertion_point;
  if (!source_text.empty()) {
    backup_to_relexing_point(relexing_point, insertion[0], insertion_offset);
  }
  token prior_sum = source_text.sum_over_interval(source_text.begin(), relexing_point);
  lexical_state pre_relex_state = prior_sum.get_lexical_effect()(INITIAL_LEXICAL_STATE);
  for (; relexing_point != insertion_point; relexing_point = source_text.erase(relexing_point)) {
    for (i7_codepoint codepoint : *relexing_point->get_text()) {
      insertion_lexer << codepoint;
    }
  }
  if (insertion_offset) {
    const i7_string&surrounding_text = *insertion_point->get_text();
    for (unsigned index = 0; index < insertion_offset; ++index) {
      insertion_lexer << surrounding_text[index];
    }
    for (i7_codepoint codepoint : insertion) {
      insertion_lexer << codepoint;
    }
    for (unsigned index = insertion_offset, end = surrounding_text.size(); index < end; ++index) {
      insertion_lexer << surrounding_text[index];
    }
    old_post_relex_state = insertion_point->get_lexical_effect()(old_post_relex_state);
    insertion_point = source_text.erase(insertion_point);
  } else {
    for (i7_codepoint codepoint : insertion) {
      insertion_lexer << codepoint;
    }
  }
  bool needs_terminator = true;
  while (insertion_point.can_increment()) {
    const i7_string&suffix = *insertion_point->get_text();
    assert(suffix.size());
    insertion_lexer << suffix[0];
    if (insertion_lexer.most_recent_codepoint_did_not_combine()) {
      needs_terminator = false;
      break;
    }
    old_post_relex_state = insertion_point->get_lexical_effect()(old_post_relex_state);
    for (unsigned index = 1, end = suffix.size(); index < end; ++index) {
      insertion_lexer << suffix[index];
    }    
    insertion_point = source_text.erase(insertion_point);
  }
  if (needs_terminator) {
    insertion_lexer << TERMINATOR_CODEPOINT;
  }
  bool is_first_change = true;
  token_iterator first_change = source_text.end();
  for (const token&result : insertion_lexer.get_results()) {
    if (is_first_change) {
      first_change = source_text.insert(insertion_point, result);
      is_first_change = false;
    } else {
      source_text.insert(insertion_point, result);
    }
  }
  return { pre_relex_state, first_change, insertion_point, old_post_relex_state };
}

lexical_reference_points_from_edit remove_codepoints(token_sequence&source_text, unsigned beginning_codepoint_index, unsigned end_codepoint_index) {
  assert(beginning_codepoint_index <= end_codepoint_index);
  if (beginning_codepoint_index == end_codepoint_index) {
    return no_reference_points_from_edit(source_text);
  }
  token_iterator beginning_removal_point = source_text.find({beginning_codepoint_index});
  assert(beginning_removal_point != source_text.end());
  unsigned beginning_removal_offset = beginning_codepoint_index - source_text.sum_over_interval(source_text.begin(), beginning_removal_point).get_codepoint_count();
  token_iterator end_removal_point = source_text.find({end_codepoint_index});
  unsigned end_removal_offset = end_codepoint_index - source_text.sum_over_interval(source_text.begin(), end_removal_point).get_codepoint_count();
  i7_string remaining_text = beginning_removal_point->get_text()->substr(0, beginning_removal_offset);
  if (end_removal_point != source_text.end()) {
    remaining_text += end_removal_point->get_text()->substr(end_removal_offset);
  }
  token_iterator reinsertion_point = end_removal_point;
  ++reinsertion_point;
  token prior_sum = source_text.sum_over_interval(source_text.begin(), reinsertion_point);
  lexical_state old_post_relex_state = prior_sum.get_lexical_effect()(INITIAL_LEXICAL_STATE);
  for (token_iterator removal_point = beginning_removal_point; removal_point != reinsertion_point; removal_point = source_text.erase(removal_point));
  if (remaining_text.size()) {
    return add_codepoints(source_text, reinsertion_point, 0, remaining_text, old_post_relex_state);
  }
  prior_sum = source_text.sum_over_interval(source_text.begin(), reinsertion_point);
  lexical_state pre_relex_state = prior_sum.get_lexical_effect()(INITIAL_LEXICAL_STATE);
  return { pre_relex_state, reinsertion_point, reinsertion_point, old_post_relex_state };
}

lexical_reference_points_from_edit add_codepoints(token_sequence&source_text, unsigned beginning_codepoint_index, const i7_string&insertion) {
  if (!insertion.size()) {
    return no_reference_points_from_edit(source_text);
  }
  token_iterator insertion_point = source_text.find({beginning_codepoint_index});
  unsigned insertion_offset = beginning_codepoint_index - source_text.sum_over_interval(source_text.begin(), insertion_point).get_codepoint_count();
  assert(insertion_point != source_text.end() || !insertion_offset);
  token prior_sum = source_text.sum_over_interval(source_text.begin(), insertion_point);
  lexical_state old_post_relex_state = prior_sum.get_lexical_effect()(INITIAL_LEXICAL_STATE);
  return add_codepoints(source_text, insertion_point, insertion_offset, insertion, old_post_relex_state);
}
