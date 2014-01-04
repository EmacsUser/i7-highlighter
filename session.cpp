#include <cassert>
#include <unordered_map>

#include "io.hpp"
#include "buffer.hpp"

using namespace std;

using buffer_map = unordered_map<unsigned, buffer>;
buffer_map buffers;

void discard_buffer(unsigned buffer_number) {
  buffers.erase(buffer_number);
}

void introduce_buffer(unsigned buffer_number) {
  buffers.insert(make_pair(buffer_number, buffer{buffer_number}));
}

void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end) {
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second.remove_codepoints(beginning, end);
}

void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion){
  buffer_map::iterator i = buffers.find(buffer_number);
  assert(i != buffers.end());
  i->second.add_codepoints(beginning, insertion);
}
