#include <cassert>

#include "io.hpp"
#include "session.hpp"
#include "parser.hpp"

using namespace std;

const production*session::add_production(const ::production&production) {
  const ::production*internalization = &production_bank.acquire(production);
  for (const production::possible_beginning&beginning : internalization->get_beginnings()) {
    productions_by_beginnings.insert(beginning.parseme, internalization);
  }
  productions_by_results.insert(internalization->get_result(), internalization);
  return internalization;
}

const production*session::remove_production(const ::production&production) {
  const ::production*internalization = production_bank.lookup(production);
  assert(internalization);
  for (const production::possible_beginning&beginning : internalization->get_beginnings()) {
    productions_by_beginnings.erase(beginning.parseme, internalization);
  }
  productions_by_results.erase(internalization->get_result(), internalization);
  production_bank.release(*internalization);
  return internalization;
}

const session::buffer_map&session::get_buffers() const {
  return buffers;
}

const unordered_set<const wording*>&session::get_wordings() const {
  return wordings;
}

const unordered_set<const sentence*>&session::get_sentences() const {
  return sentences;
}

const unordered_set<const passage*>&session::get_passages() const {
  return passages;
}

void session::add_wording(const ::wording&wording) {
  const ::wording*internalization = dynamic_cast<const ::wording*>(add_production(wording));
  assert(wordings.find(internalization) == wordings.end());
  wordings.insert(internalization);
}

void session::add_sentence(const ::sentence&sentence) {
  const ::sentence*internalization = dynamic_cast<const ::sentence*>(add_production(sentence));
  assert(sentences.find(internalization) == sentences.end());
  sentences.insert(internalization);
}

void session::add_passage(const ::passage&passage) {
  const ::passage*internalization = dynamic_cast<const ::passage*>(add_production(passage));
  assert(passages.find(internalization) == passages.end());
  passages.insert(internalization);
}

void session::remove_wording(const ::wording&wording) {
  const ::wording*internalization = dynamic_cast<const ::wording*>(remove_production(wording));
  assert(wordings.find(internalization) != wordings.end());
  wordings.erase(internalization);
}

void session::remove_sentence(const ::sentence&sentence) {
  const ::sentence*internalization = dynamic_cast<const ::sentence*>(remove_production(sentence));
  assert(sentences.find(internalization) != sentences.end());
  sentences.erase(internalization);
}

void session::remove_passage(const ::passage&passage) {
  const ::passage*internalization = dynamic_cast<const ::passage*>(remove_production(passage));
  assert(passages.find(internalization) != passages.end());
  passages.erase(internalization);
}

const session::production_set&session::get_productions_beginning_with(const parseme*beginning) const {
  return productions_by_beginnings[beginning];
}

const session::production_set&session::get_productions_resulting_in(const parseme*result) const {
  return productions_by_results[result];
}

bool session::can_begin_descendant(const production*key, const production*root, unordered_set<const production*>&visited) const {
  if (key == root) {
    return true;
  }
  if (visited.insert(root).second) {
    for (const production::possible_beginning&beginning : root->get_beginnings()) {
      for (const production*production : get_productions_resulting_in(beginning.parseme)) {
	if (can_begin_descendant(key, production, visited)) {
	  return true;
	}
      }
    }
  }
  return false;
}

void session::get_additional_beginnings(const production*root, unordered_set<const production*>&visited) const {
  if (visited.insert(root).second) {
    for (const production::possible_beginning&beginning : root->get_beginnings()) {
      for (const production*production : get_productions_resulting_in(beginning.parseme)) {
	get_additional_beginnings(production, visited);
      }
    }
  }
}

void session::get_additional_beginnings_relying_on(const production*root, unordered_map<const production*, unsigned>&visited, const production&crux, const function<bool(const production*)>&exemption) const {
  if (exemption(root)) {
    return;
  }
  unordered_map<const production*, unsigned>::iterator iterator = visited.find(root);
  if (iterator == visited.end()) {
    visited[root] = 1;
    for (const production::possible_beginning&beginning : root->get_beginnings()) {
      for (const production*production : get_productions_resulting_in(beginning.parseme)) {
	get_additional_beginnings_relying_on(production, visited, crux, exemption);
      }
    }
  } else {
    ++iterator->second;
  }
}

unordered_set<const production*>session::get_beginnings_relying_on(const production&crux, const function<bool(const production*)>&exemption) const {
  unordered_map<const production*, unsigned>visited;
  get_additional_beginnings_relying_on(&crux, visited, crux, exemption);
  visited.erase(&crux);
  unordered_set<const production*>result;
  result.insert(&crux);
  for (const auto&visitation : visited) {
    if (visitation.second == productions_by_beginnings[visitation.first->get_result()].size()) {
      result.insert(visitation.first);
    }
  }
  return result;
}

bool session::can_begin_sentence_with(const production*key) const {
  if (dynamic_cast<const passage*>(key)) {
    return true;
  }
  unordered_set<const production*>visited;
  for (const production*root : sentences) {
    if (can_begin_descendant(key, root, visited)) {
      return true;
    }
  }
  return false;
}

unordered_set<const production*>session::get_sentence_beginnings() const {
  unordered_set<const production*>result;
  for (const production*root : passages) {
    result.insert(root);
  }
  for (const production*root : sentences) {
    get_additional_beginnings(root, result);
  }
  return result;
}

unordered_set<const production*>session::get_sentence_beginnings_relying_on(const production&crux) const {
  return get_beginnings_relying_on(crux, [] (const production*point) {
      return dynamic_cast<const sentence*>(point) || dynamic_cast<const passage*>(point);
    });
}

unordered_set<const production*>session::get_result_beginnings_relying_on(const production&crux) const {
  const nonterminal*exempt_result = crux.get_result();
  return get_beginnings_relying_on(crux, [exempt_result] (const production*point) {
      return point->get_result() == exempt_result;
    });
}

unordered_set<const production*>session::get_continuing_beginnings(const match&partial_match) const {
  unordered_set<const production*>result;
  for (const parseme*alternative : partial_match.get_continuing_alternatives()) {
    for (const production*root : get_productions_resulting_in(alternative)) {
      get_additional_beginnings(root, result);
    }
  }
  return result;
}

unordered_set<const production*>session::get_continuing_beginnings(token_iterator inclusive_end_of_matches) const {
  unordered_set<const production*>result;
  for (const annotation_wrapper&wrapper : inclusive_end_of_matches->get_annotations(typeid(match))) {
    const match&partial_match = dynamic_cast<const match&>(static_cast<const annotation&>(wrapper));
    if (!partial_match.is_complete() && partial_match.get_inclusive_end() == inclusive_end_of_matches) {
      for (const parseme*alternative : partial_match.get_continuing_alternatives()) {
	for (const production*root : get_productions_resulting_in(alternative)) {
	  get_additional_beginnings(root, result);
	}
      }
    }
  }
  return result;  
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

typename ::session*session = nullptr;

void discard_buffer(unsigned buffer_number) {
  session->discard_buffer(buffer_number);
}
void introduce_buffer(unsigned buffer_number) {
  session->introduce_buffer(buffer_number);
}
void remove_codepoints(unsigned buffer_number, unsigned beginning, unsigned end) {
  session->remove_codepoints(buffer_number, beginning, end);
}
void add_codepoints(unsigned buffer_number, unsigned beginning, const i7_string&insertion) {
  session->add_codepoints(buffer_number, beginning, insertion);
}
