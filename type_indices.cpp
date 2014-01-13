#include "type_indices.hpp"

#include "codepoints.hpp"
#include "session.hpp"
#include "parser.hpp"

using namespace std;

static struct do_not_use_this_struct {
  typename ::session			session_instance;
  buffer*				buffer_instance;
  monoid_sequence<token>		token_sequence_instance;
  next_token*				next_token_instance;
  match*				match_instance;
  do_not_use_this_struct() {
    session_instance.introduce_buffer(0);
    for (auto&i : session_instance.get_buffers()) {
      buffer_instance = i.second;
    }
    monoid_sequence<token>::iterator first_token_iterator_instance = token_sequence_instance.insert(token_sequence_instance.end(), token{});
    monoid_sequence<token>::iterator second_token_iterator_instance = token_sequence_instance.insert(token_sequence_instance.end(), token{});
    next_token_instance = new next_token{session_instance, first_token_iterator_instance, second_token_iterator_instance};
    const i7_string*i7_string_instance = &vocabulary.acquire({ENCODE("")});
    vocabulary.release(*i7_string_instance);
    nonterminal nonterminal_instance{ENCODE(""), 0};
    production production_instance{session_instance, nonterminal_instance};
    production_instance.justify();
    match_instance = new match{session_instance, buffer_instance, production_instance, 0, first_token_iterator_instance, second_token_iterator_instance};
  }
} do_not_use_this_struct;

type_index next_token_type_index{typeid(*do_not_use_this_struct.next_token_instance)};
type_index match_type_index{typeid(*do_not_use_this_struct.match_instance)};
