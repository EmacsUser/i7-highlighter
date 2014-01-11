#include <cassert>

#include "io.hpp"
#include "session.hpp"
#include "parser.hpp"

using namespace std;

const session::production_set&session::get_productions() const {
  return productions;
}

pair<typename unordered_multimap<const parseme*, const production*>::const_iterator, typename unordered_multimap<const parseme*, const production*>::const_iterator> session::get_productions(const parseme*beginning) const {
  return productions_by_beginnings.equal_range(beginning);
}

void session::add_production(const ::production&production) {
  const ::production*internalization = &production_bank.acquire(production);
  assert(productions.find(internalization) == productions.end());
  productions.insert(internalization);
  for (const parseme*key : internalization->get_beginnings()) {
    productions_by_beginnings.insert({key, internalization});
  }
}

void session::remove_production(const ::production&production) {
  const ::production*internalization = production_bank.lookup(production);
  assert(internalization);
  assert(productions.find(internalization) != productions.end());
  productions.erase(internalization);
  for (const parseme*key : internalization->get_beginnings()) {
    auto range = productions_by_beginnings.equal_range(key);
    for (auto i = range.first; i != range.second; ++i) {
      if (i->second == internalization) {
	productions_by_beginnings.erase(i);
	break;
      }
    }
  }
  production_bank.release(*internalization);
}

void session::discard_buffer(unsigned buffer_number) {
  buffers.erase(buffer_number);
}

void session::introduce_buffer(unsigned buffer_number) {
  buffers.insert(make_pair(buffer_number, buffer{buffer_number}));
}

void session::remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end) {
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second.remove_codepoints(beginning, end);
}

void session::add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion) {
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second.add_codepoints(beginning, insertion);
}

typename ::session session;

void discard_buffer(unsigned buffer_number) {
  session.discard_buffer(buffer_number);
}
void introduce_buffer(unsigned buffer_number) {
  session.introduce_buffer(buffer_number);
}
void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end) {
  session.remove_codepoints(buffer_number, beginning, end);
}
void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion) {
  session.add_codepoints(buffer_number, beginning, insertion);
}
