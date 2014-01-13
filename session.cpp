#include <cassert>

#include "io.hpp"
#include "session.hpp"
#include "parser.hpp"

using namespace std;

const session::buffer_map&session::get_buffers() const {
  return buffers;
}

const session::production_set&session::get_productions() const {
  return productions;
}


const session::production_set&session::get_productions(const parseme*beginning) const {
  return productions_by_beginnings[beginning];
}

void session::add_production(const ::production&production) {
  const ::production*internalization = &production_bank.acquire(production);
  assert(productions.find(internalization) == productions.end());
  productions.insert(internalization);
  for (const production::possible_beginning&beginning : internalization->get_beginnings()) {
    productions_by_beginnings.insert(beginning.parseme, internalization);
  }
}

void session::remove_production(const ::production&production) {
  const ::production*internalization = production_bank.lookup(production);
  assert(internalization);
  assert(productions.find(internalization) != productions.end());
  productions.erase(internalization);
  for (const production::possible_beginning&beginning : internalization->get_beginnings()) {
    productions_by_beginnings.erase(beginning.parseme, internalization);
  }
  production_bank.release(*internalization);
}

void session::discard_buffer(unsigned buffer_number) {
  buffer_map::iterator iterator = buffers.find(buffer_number);
  if (iterator != buffers.end()) {
    delete iterator->second;
    buffers.erase(iterator);
  }
}

void session::introduce_buffer(unsigned buffer_number) {
  buffers.insert({buffer_number, new buffer{*this, buffer_number}});
}

void session::remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end) {
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second->remove_codepoints(beginning, end);
}

void session::add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion) {
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second->add_codepoints(beginning, insertion);
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
