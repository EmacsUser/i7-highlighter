#include "codepoints.hpp"

using namespace std;

string ASSUME_EIGHT_BIT(const i7_string&text) {
  return {text.begin(), text.end()};
}

// See Unicode 6.0, Chapter 4.6.
bool is_whitespace(i7_codepoint codepoint) {
  switch (codepoint) {
  case 0x0009:
  case 0x000A:
  case 0x000B:
  case 0x000C:
  case 0x000D:
  case 0x0020:
  case 0x007C: // `|', an Inform 7 paragraph break.
  case 0x0085:
  case 0x00A0:
  case 0x1680:
  case 0x180E:
  case 0x2000:
  case 0x2001:
  case 0x2002:
  case 0x2003:
  case 0x2004:
  case 0x2005:
  case 0x2006:
  case 0x2007:
  case 0x2008:
  case 0x2009:
  case 0x200A:
  case 0x2028:
  case 0x2029:
  case 0x202F:
  case 0x205F:
  case 0x3000:
    return true;
  default:
    return false;
  }
}

bool is_i7_punctuation(i7_codepoint codepoint) {
  switch (codepoint) {
  case 0x0021:
  case 0x0022:
  case 0x0027:
  case 0x0028:
  case 0x0029:
  case 0x002A:
  case 0x002B:
  case 0x002D:
  case 0x002E:
  case 0x003A:
  case 0x003B:
  case 0x005B:
  case 0x005C:
  case 0x005D:
    return true;
  default:
    return false;
  }
}

bool is_i7_letter(i7_codepoint codepoint) {
  return !is_whitespace(codepoint) && !is_i7_punctuation(codepoint) && codepoint != TERMINATOR_CODEPOINT;
}

bool is_i7_digit(i7_codepoint codepoint) {
  return ('0' <= codepoint) && (codepoint <= '9');
}

bool is_i7_lexical_delimiter_letter(i7_codepoint codepoint) {
  switch (codepoint) {
  case 'D':
  case 'O':
  case 'C':
  case 'U':
  case 'M':
  case 'E':
  case 'N':
  case 'T':
  case 'A':
  case 'I':
    return true;
  default:
    return false;
  }
}

// This bit gotten by trawling the internet until I found ni's @<Return Unicode
// fancy equivalents as simpler literals@>, and then adjusted it to use a switch
// and therefore return slightly faster in the common case.
i7_codepoint i7_normalize(i7_codepoint codepoint) {
  switch (codepoint) {
    case 0x85:
      return '\n';
    case 0xA0:
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200a:
      return ' ';
    case 0x2010:
    case 0x2011:
    case 0x2012:
    case 0x2013:
    case 0x2014:
      return '-';
    case 0x2018:
    case 0x2019:
      return '\'';
    case 0x201c:
    case 0x201d:
      return '"';
    case 0x2028:
    case 0x2029:
      return '\n';
  }
  return codepoint;
}
