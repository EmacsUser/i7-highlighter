#ifndef SESSION_HEADER
#define SESSION_HEADER

#include <unordered_set>

#include "custom_multimap.hpp"
#include "internalizer.hpp"
#include "buffer.hpp"
#include "deduction.hpp"

class parseme;
class production;

class session : public context {
protected:
  using buffer_map = std::unordered_map<unsigned, buffer*>;
  using production_set = std::unordered_set<const production*>;
  using production_map = custom_multimap<const parseme*, const production*>;
  buffer_map				buffers;
  production_set			productions;
  production_map			productions_by_beginnings;

public:
  // Destructor intentionally omitted, as, at the moment, the only instance is a
  // static global, which does not need to clean up.
  // ~session();

  const buffer_map&get_buffers() const;

  const production_set&get_productions() const;
  const production_set&get_productions(const parseme*beginning) const;
  void add_production(const ::production&production);
  void remove_production(const ::production&production);

  void discard_buffer(unsigned buffer_number);
  void introduce_buffer(unsigned buffer_number);
  void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end);
  void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion);
};

extern ::session session;

#endif
