#include "token.hpp"

using namespace std;

internalizer<i7_string>vocabulary;

// The addition constructor.
token::token(const token&left, const token&right) :
  codepoint_count{left.codepoint_count + right.codepoint_count},
  line_count{left.line_count + right.line_count},
  text{nullptr},
  only_whitespace{left.only_whitespace && right.only_whitespace},
  lexical_effect{left.lexical_effect + right.lexical_effect} {}

token::token() :
  codepoint_count{0},
  line_count{0},
  text{nullptr},
  only_whitespace{true},
  lexical_effect{0} {}

token::token(unsigned codepoint_count) :
  codepoint_count{codepoint_count},
  line_count{0},
  text{nullptr},
  only_whitespace{false},
  lexical_effect{0} {}

token::token(const i7_string&text, bool only_whitespace, const lexer_monoid&lexical_effect, unsigned line_count) :
  codepoint_count{static_cast<unsigned>(text.size())},
  line_count{line_count},
  text{&vocabulary.acquire(text)},
  only_whitespace{only_whitespace},
  lexical_effect{lexical_effect} {}

token::token(const token&copy) :
  codepoint_count{copy.codepoint_count},
  line_count{copy.line_count},
  text{copy.text ? &vocabulary.acquire(*copy.text) : nullptr},
  only_whitespace{copy.only_whitespace},
  lexical_effect{copy.lexical_effect} {}

token::~token() {
  unjustify_all_annotation_facts();
  if (text) {
    vocabulary.release(*text);
  }
}

token&token::operator =(const token&copy) {
  if (&copy == this) {
    return *this;
  }
  codepoint_count = copy.codepoint_count;
  line_count = copy.line_count;
  if (text != copy.text) {
    if (text) {
      vocabulary.release(*text);
    }
    if (copy.text) {
      text = &vocabulary.acquire(*copy.text);
    } else {
      text = nullptr;
    }
  }
  only_whitespace = copy.only_whitespace;
  lexical_effect = copy.lexical_effect;
  return *this;
}

unsigned token::get_codepoint_count() const {
  return codepoint_count;
}

unsigned token::get_line_count() const {
  return line_count;
}

const i7_string*token::get_text() const {
  return text;
}

bool token::is_only_whitespace() const {
  return only_whitespace;
}

const lexer_monoid&token::get_lexical_effect() const {
  return lexical_effect;
}

bool token::operator <(const token&other) const {
  return codepoint_count < other.codepoint_count;
}

token token::operator +(const token&other) const {
  return token{*this, other};
}

token&token::operator +=(const token&other) {
  codepoint_count += other.codepoint_count;
  line_count += other.line_count;
  if (text) {
    vocabulary.release(*text);
  }
  text = nullptr;
  lexical_effect += other.lexical_effect;
  return *this;
}

ostream&operator <<(ostream&out, const ::token&token) {
  if (token.get_text()) {
    out << "`" << ASSUME_EIGHT_BIT(*token.get_text()) << "'_";
  }
  return out << token.get_codepoint_count() << ": " << token.get_lexical_effect();
}
