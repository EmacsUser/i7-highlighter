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

session::session() {
  // Parentheses
  RESULT(wording, "left parenthesis", 0);
  SLOT TERMINAL("(");
  DONE;
  RESULT(wording, "right parenthesis", 0);
  SLOT TERMINAL(")");
  DONE;

  // Digits
  RESULT(wording, "digits", 0);
  SLOT OTHER(digits_terminal{});
  DONE;

  // Words
  RESULT(wording, "word", 0);
  SLOT OTHER(word_terminal{});
  DONE;

  // Name words
  RESULT(wording, "name word", 0);
  SLOT OTHER(name_word_terminal{});
  DONE;

  // Sentence endings
  RESULT(wording, "end of a sentence", 0);
  SLOT OTHER(end_of_sentence_terminal{});
  DONE;

  // Name words (internal)
  RESULT(wording, "name words (internal)", 0);
  SLOT NONTERMINAL("name word", 0);
  DONE;
  RESULT(wording, "name words (internal)", 0);
  SLOT NONTERMINAL("name words (internal)", 0);
  SLOT NONTERMINAL("name word", 0);
  DONE;

  // Nonterminal declarations
  RESULT(sentence, "a nonterminal declaration", 0);
  SLOT TERMINAL("#");
  SLOT NONTERMINAL("name words (internal)", 0);
  SLOT TERMINAL("is");
  SLOT TERMINAL("a");
  SLOT TERMINAL("wording") TERMINAL("line") TERMINAL("sentence") TERMINAL("passage");
  SLOT TERMINAL(".");
  SLOT OTHER(end_of_sentence_terminal{});
  DONE;
}
