#ifndef SESSION_HEADER
#define SESSION_HEADER

#include "internalizer.hpp"
#include "buffer.hpp"
#include "deduction.hpp"

class parseme;
class production;

class session : public context {
protected:
  using buffer_map = std::unordered_map<unsigned, buffer>;
  using production_map = std::unordered_multimap<const parseme*, const production*>;
  buffer_map				buffers;
  production_map			productions;

public:
  void discard_buffer(unsigned buffer_number);
  void introduce_buffer(unsigned buffer_number);
  void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end);
  void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion);

  production_map&get_productions();
};

extern ::session session;

#endif
