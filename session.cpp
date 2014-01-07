#include <cassert>
#include <unordered_map>

#include "io.hpp"
#include "session.hpp"

using namespace std;

void ::session::discard_buffer(unsigned buffer_number) {
  buffers.erase(buffer_number);
}

void ::session::introduce_buffer(unsigned buffer_number) {
  buffers.insert(make_pair(buffer_number, buffer{buffer_number}));
}

void ::session::remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end) {
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second.remove_codepoints(beginning, end);
}

void ::session::add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion) {
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second.add_codepoints(beginning, insertion);
}

session::production_map&::session::get_productions() {
  return productions;
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
