#include "type_indices.hpp"

#include "session.hpp"
#include "parser.hpp"

using namespace std;

static typename ::session do_not_use_this_session;
static nonterminal do_not_use_this_nonterminal{ENCODE(""), 0};
static production do_not_use_this_production{do_not_use_this_session, do_not_use_this_nonterminal};
static monoid_sequence<token>do_not_use_this_token_sequence;
static next_token do_not_use_this_next_token{do_not_use_this_session, do_not_use_this_token_sequence.end(), do_not_use_this_token_sequence.end()};
static match do_not_use_this_match{do_not_use_this_session, nullptr, do_not_use_this_production, 0, do_not_use_this_token_sequence.end(), do_not_use_this_token_sequence.end()};

type_index next_token_type_index{typeid(do_not_use_this_next_token)};
type_index match_type_index{typeid(do_not_use_this_match)};
