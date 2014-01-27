#ifndef SESSION_HEADER
#define SESSION_HEADER

#include <functional>
#include <unordered_set>
#include <unordered_map>

#include "custom_multimap.hpp"
#include "buffer.hpp"
#include "deduction.hpp"

class parseme;
class production;
class wording;
class sentence;
class passage;
class match;

class session : public context {
protected:
  using buffer_map = std::unordered_map<unsigned, buffer*>;
  using production_map = custom_multimap<const parseme*, const production*>;
  using production_set = typename production_map::value_set_type;
  buffer_map				buffers;
  std::unordered_set<const wording*>	wordings;
  std::unordered_set<const sentence*>	sentences;
  std::unordered_set<const passage*>	passages;
  production_map			productions_by_beginnings;
  production_map			productions_by_results;

public:
  session(); // Defined in language.cpp rather than session.cpp.
  // Destructor intentionally omitted, as, at the moment, the only instance is a
  // static global, which does not need to clean up.
  // ~session();

protected:
  const production*add_production(const ::production&production);
  const production*remove_production(const ::production&production);

public:
  const buffer_map&get_buffers() const;

  const std::unordered_set<const wording*>&get_wordings() const;
  const std::unordered_set<const sentence*>&get_sentences() const;
  const std::unordered_set<const passage*>&get_passages() const;

  void add_wording(const ::wording&wording);
  void add_sentence(const ::sentence&sentence);
  void add_passage(const ::passage&passage);

  void remove_wording(const ::wording&wording);
  void remove_sentence(const ::sentence&sentence);
  void remove_passage(const ::passage&passage);

  const production_set&get_productions_beginning_with(const parseme*beginning) const;
  const production_set&get_productions_resulting_in(const parseme*result) const;

protected:
  bool can_begin_descendant(const production*key, const production*root, std::unordered_set<const production*>&visited) const;
  void get_additional_beginnings(const production*root, std::unordered_set<const production*>&visited) const;
  void get_additional_beginnings_relying_on(const production*root, std::unordered_map<const production*, unsigned>&visited, const production&crux, const std::function<bool(const production*)>&exemption) const;
  std::unordered_set<const production*>get_beginnings_relying_on(const production&crux, const std::function<bool(const production*)>&exemption) const;

public:
  bool can_begin_sentence_with(const production*key) const;
  std::unordered_set<const production*>get_sentence_beginnings() const;
  std::unordered_set<const production*>get_sentence_beginnings_relying_on(const production&crux) const;
  std::unordered_set<const production*>get_result_beginnings_relying_on(const production&crux) const;
  std::unordered_set<const production*>get_continuing_beginnings(const match&partial_match) const;
  std::unordered_set<const production*>get_continuing_beginnings(token_iterator inclusive_end_of_matches) const;

  void discard_buffer(unsigned buffer_number);
  void introduce_buffer(unsigned buffer_number);
  void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end);
  void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion);
};

extern ::session*session;

#endif
