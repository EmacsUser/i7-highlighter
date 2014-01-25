#include "codepoints.hpp"
#include "parser.hpp"
#include "session.hpp"

using namespace std;

// Nonterminals.
#define N(kind, tier) nonterminal{ENCODE("|" kind "|"), tier}

// Token terminals.
#define T(text) token_terminal{ENCODE(text)}

// Macros used directly in the code below.
#define RESULT(level, kind, tier) { level current{*this, N(kind, tier)};
#define SLOT current.add_slot();
#define TERMINAL(text) current.add_alternative_to_last_slot(T(text));
#define NONTERMINAL(kind, tier) current.add_alternative_to_last_slot(N(kind, tier));
#define OTHER(parseme) current.add_alternative_to_last_slot(parseme);
#define DONE current.justify(); }

session::session() {}
