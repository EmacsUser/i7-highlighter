#ifndef UTILITY_HEADER
#define UTILITY_HEADER

#include <unordered_set>

template<typename T>class intersection {
protected:
  const std::unordered_set<T>&smaller;
  const std::unordered_set<T>&larger;

public:
  intersection(const std::unordered_set<T>&left, const std::unordered_set<T>&right) :
    smaller((left.size() < right.size()) ? left : right),
    larger((left.size() < right.size()) ? right : left) {}

  class iterator {
  protected:
    typename std::unordered_set<T>::const_iterator iterator_over_smaller;
    typename std::unordered_set<T>::const_iterator end_of_smaller;
    const std::unordered_set<T>&larger;

  public:
    iterator(typename std::unordered_set<T>::const_iterator iterator_over_smaller, typename std::unordered_set<T>::const_iterator end_of_smaller, const std::unordered_set<T>&larger) :
      iterator_over_smaller{iterator_over_smaller},
      end_of_smaller{end_of_smaller},
      larger{larger} {
      while (iterator_over_smaller != end_of_smaller && larger.find(*iterator_over_smaller) == larger.end()) {
	++iterator_over_smaller;
      }
    }

    bool operator ==(const iterator&other) const {
      return iterator_over_smaller == other.iterator_over_smaller;
    }
    bool operator !=(const iterator&other) const {
      return iterator_over_smaller != other.iterator_over_smaller;
    }

    const T&operator *() const {
      return *iterator_over_smaller;
    }
    const T*operator ->() const {
      return &*iterator_over_smaller;
    }

    iterator&operator ++() {
      do {
	++iterator_over_smaller;
      } while (iterator_over_smaller != end_of_smaller && larger.find(*iterator_over_smaller) == larger.end());
      return *this;
    }
    iterator&operator ++(int) {
      iterator result = *this;
      operator ++();
      return result;
    }
  };

  iterator begin() const {
    return iterator{smaller.begin(), smaller.end(), larger};
  }
  iterator end() const {
    return iterator{smaller.end(), smaller.end(), larger};
  }
};

#endif
