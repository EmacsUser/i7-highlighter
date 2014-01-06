#ifndef SESSION_HEADER
#define SESSION_HEADER

#include "deduction.hpp"
#include "buffer.hpp"

class session : public context {
protected:
  using buffer_map = std::unordered_map<unsigned, buffer>;
  buffer_map buffers;

public:
  void discard_buffer(unsigned buffer_number);
  void introduce_buffer(unsigned buffer_number);
  void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end);
  void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion);
};

extern ::session session;

#endif
