#ifndef MONOID_SEQUENCE_HEADER
#define MONOID_SEQUENCE_HEADER

#include <cassert>

/*
 * A sorted sequence of monoid elements that allows fast lookups for sums over
 * intervals.  (``Fast'' here means amortized O(ln(n)^2) in the non-abelian
 * case, O(ln(n)) in the abelian case.  An alternative design could give
 * amortized O(ln(n)) all around, but at the expense of memory.)
 *
 * T is the type summed.  It must have a zero element and support an associative
 * binary operator + such that (T, +, 0) is a monoid.  If + is commutative,
 * MONOID_IS_ABELIAN should be set to true, to speed up the query operations.
 * Furthermore, as the seqence is indexed by partial sums from the beginning, T
 * must also offer an operator < such that, for any values satisfying A + B = C,
 * C < A does not hold.  Indexing is easier to think about when < with
 * reflexivity is total, but this is not a requirement.
 *
 * As a concession to efficiency, T is also expected to support a += operator,
 * such that X += Y has the same semantics as X = X + Y.  (Note the operand
 * order, which is important for non-abelian monoids).  This is because, for
 * some types, it is possible to make += faster than + followed by =.
 */
template<typename T, bool MONOID_IS_ABELIAN = false>class monoid_sequence {
protected:
  /* Internally, a monoid_sequence is represented as an AVL tree with leaves
   * storing the elements and non-leaves storing partial sums.  The vertex class
   * represents the AVL tree vertices.
   */
  class vertex {
  protected:
    // If the vertex is a leaf, this is its contribution.  Otherwise, this is
    // the total contribution of the left subtree.
    T					difference;
    // The vertex's immediate relatives.
    vertex*				parent;
    vertex*				left;
    vertex*				right;
    // Cached information about the vertex's relatives.
    unsigned				size_of_subtree;

  protected:
    // This constructor creates a new non-leaf, which is automatically attached
    // by replacing and demoting one of its children.
    //
    // Arguments left and right are its children; one should be a leaf created
    // by the public constructor, while the other should be the vertex that this
    // one will replace.
    //
    // Argument replaced is the non-leaf child, either left or right.
    //
    // The root pointer is passed by reference so that it can be updated if the
    // root is replaced.
    vertex(vertex*left, vertex*right, vertex*replaced, vertex*&root) :
      difference{left->difference},
      parent{replaced->parent},
      left{left},
      right{right},
      size_of_subtree{3} {
      vertex*added = (replaced == left ? right : left);
      assert(left);
      assert(right);
      assert(left != right);
      assert(replaced == left || replaced == right);
      assert(!replaced->left);
      assert(!replaced->right);
      assert(!added->parent);
      assert(!added->left);
      assert(!added->right);
      assert(!parent || parent->left == replaced || parent->right == replaced);
      if (parent) {
	if (parent->left == replaced) {
	  parent->left = this;
	} else {
	  parent->right = this;
	}
      }
      left->parent = this;
      right->parent = this;
      if (MONOID_IS_ABELIAN) {
	increase_ancestor_differences_and_recompute_subtree_sizes(added->difference);
      } else {
	recompute_ancestor_differences_and_subtree_sizes();
      }
      if (parent) {
	parent->balance(root);
      }
      if (root == replaced) {
	root = this;
      }
    }

  public:
    // This constructor creates a new leaf, which should be attached either by
    // making it the root or by passing it to the constructor above.
    vertex(const T&difference) :
      difference{difference},
      parent{nullptr},
      left{nullptr},
      right{nullptr},
      size_of_subtree{1} {}

    virtual ~vertex() {
      if (!is_leaf()) {
	delete left;
	delete right;
      }
    }

  protected:
    bool is_leaf() const {
      if (left) {
	assert(right);
	return false;
      }
      assert(!right);
      return true;
    }

    vertex*left_turn_above() const {
      const vertex*below = this;
      for (vertex*above = parent; above; below = above, above = above->parent) {
	if (below == above->right) {
	  return above;
	}
      }
      return nullptr;
    }

    vertex*right_turn_above() const {
      const vertex*below = this;
      for (vertex*above = parent; above; below = above, above = above->parent) {
	if (below == above->left) {
	  return above;
	}
      }
      return nullptr;
    }

    unsigned get_depth() const {
      unsigned result = 0;
      for (vertex*ancestor = this->parent; ancestor; ancestor = ancestor->parent) {
	++result;
      }
      return result;
    }

    vertex*get_common_ancestor(const vertex*other) const {
      if (!other) {
	return nullptr;
      }
      const vertex*ancestor = this;
      unsigned ancestor_depth = get_depth();
      const vertex*other_ancestor = other;
      unsigned other_ancestor_depth = other->get_depth();
      for(; ancestor_depth > other_ancestor_depth; --ancestor_depth) {
	ancestor = ancestor->parent;
      }
      for(; other_ancestor_depth > ancestor_depth; --other_ancestor_depth) {
	other_ancestor = other_ancestor->parent;
      }
      while (ancestor != other_ancestor) {
	ancestor = ancestor->parent;
	other_ancestor = other_ancestor->parent;
      }
      return const_cast<vertex*>(ancestor);
    }

    bool operator <(const vertex&other) const {
      if (&other == this) {
	return false;
      }
      bool result;
      const vertex*ancestor = this;
      unsigned ancestor_depth = get_depth();
      const vertex*other_ancestor = other;
      unsigned other_ancestor_depth = other->get_depth();
      for(; ancestor_depth > other_ancestor_depth; --ancestor_depth) {
	vertex*new_ancestor = ancestor->parent;
	result = (ancestor == new_ancestor->left);
	ancestor = new_ancestor;
      }
      for(; other_ancestor_depth > ancestor_depth; --other_ancestor_depth) {
	vertex*new_other_ancestor = other_ancestor->parent;
	result = (other_ancestor == new_other_ancestor->right);
	other_ancestor = new_other_ancestor;
      }
      while (ancestor != other_ancestor) {
	vertex*new_ancestor = ancestor->parent;
	result = (ancestor == new_ancestor->left);
	ancestor = new_ancestor;
	other_ancestor = other_ancestor->parent;
      }
      return result;
    }

    void recompute_ancestor_differences_and_subtree_sizes() {
      for (vertex*below = this, *above = parent; above; below = above, above = above->parent) {
	if (below == above->left) {
	  above->difference = below->difference;
	  for (vertex*addend = below->right; addend; addend = addend->right) {
	    above->difference += addend->difference;
	  }
	}
	above->size_of_subtree = 1 + above->left->size_of_subtree + above->right->size_of_subtree;
      }
    }

    void increase_ancestor_differences_and_recompute_subtree_sizes(const T&addend) {
      for (vertex*below = this, *above = parent; above; below = above, above = above->parent) {
	if (below == above->left) {
	  above->difference += addend;
	}
	above->size_of_subtree = 1 + above->left->size_of_subtree + above->right->size_of_subtree;
      }
    }

    void rotate_left(vertex*&root) {
      assert(!is_leaf());
      assert(!right->is_leaf());
      vertex*grandchild = right->left;
      if (parent) {
	if (parent->left == this) {
	  parent->left = right;
	} else {
	  assert(parent->right == this);
	  parent->right = right;
	}
      } else {
	root = right;
      }
      right->parent = parent;
      right->left = this;
      right->difference = difference + right->difference;
      parent = right;
      right = grandchild;
      grandchild->parent = this;
    }

    void rotate_right(vertex*&root) {
      assert(!is_leaf());
      assert(!left->is_leaf());
      vertex*grandchild = left->right;
      if (parent) {
	if (parent->left == this) {
	  parent->left = left;
	} else {
	  assert(parent->right == this);
	  parent->right = left;
	}
      } else {
	root = left;
      }
      left->parent = parent;
      left->right = this;
      parent = left;
      left = grandchild;
      grandchild->parent = this;
      difference = 0;
      for (vertex*addend = grandchild; addend; addend = addend->right) {
	difference += addend->difference;
      }
    }

    void balance(vertex*&root) {
      vertex*above = parent;
      if (left->size_of_subtree > 2 * right->size_of_subtree) {
	rotate_right(root);
      } else if (right->size_of_subtree > 2 * left->size_of_subtree) {
	rotate_left(root);
      }
      if (above) {
	above->balance(root);
      }
    }

  public:
    const T&get_difference() const {
      return difference;
    }

    vertex*get_parent() const {
      return parent;
    }

    vertex*get_previous() const {
      vertex*turn = left_turn_above();
      if (turn) {
	assert(turn->left);
	return turn->left->get_rightmost_descendant();
      }
      return nullptr;
    }

    vertex*get_next() const {
      vertex*turn = right_turn_above();
      if (turn) {
	assert(turn->right);
	return turn->right->get_leftmost_descendant();
      }
      return nullptr;
    }

    vertex*get_leftmost_descendant() {
      vertex*result = this;
      for (vertex*below; (below = result->left); result = below);
      return result;
    }

    vertex*get_rightmost_descendant() {
      vertex*result = this;
      for (vertex*below; (below = result->right); result = below);
      return result;
    }

    T get_sum_of_children() const {
      T result = difference;
      for (vertex*descendant = right; descendant; descendant = descendant->right) {
	result += descendant->difference;
      }
      return result;
    }

    T get_sum_until(const vertex*right_endpoint) const {
      if (this == right_endpoint) {
	return 0;
      }
      vertex*ancestor = get_common_ancestor(right_endpoint);
      assert((ancestor != nullptr) == (right_endpoint != nullptr));
      T left_sum = difference;
      for (const vertex*below = this, *above = parent; above != ancestor; below = above, above = above->parent) {
	if (below == above->left) {
	  left_sum += above->right->get_sum_of_children();
	}
      }
      if (!right_endpoint) {
	return left_sum;
      }
      T right_sum = 0;
      for (const vertex*below = right_endpoint, *above = right_endpoint->parent; above != ancestor; below = above, above = above->parent) {
	if (below == above->right) {
	  right_sum = above->difference + right_sum;
	}
      }
      return left_sum + right_sum;
    }

  protected:
    vertex*get_leftmost_strictly_to_right(const T&sum_of_strictly_left, const T&target) {
      T sum = sum_of_strictly_left + difference;
      if (is_leaf()) {
	if (target < sum) {
	  return this;
	}
	return nullptr;
      }
      if (target < sum) {
	return left->get_leftmost_strictly_to_right(sum_of_strictly_left, target);
      }
      return right->get_leftmost_strictly_to_right(sum, target);
    }

  public:
    vertex*get_leftmost_strictly_to_right(const T&target) {
      static T zero = 0;
      return get_leftmost_strictly_to_right(zero, target);
    }

    const vertex*get_leftmost_strictly_to_right(const T&target) const {
      return const_cast<vertex*>(this)->get_leftmost_strictly_to_right(target);
    }

    // The monoid sequence is passed by reference first so that its root can be
    // updated if the root is replaced and second so that we can construct the
    // return value.
    typename monoid_sequence::iterator insert_before(const T&difference, monoid_sequence&sequence) {
      assert(is_leaf());
      vertex*result = new vertex{difference};
      new vertex{result, this, this, sequence.root};
      return {&sequence, result};
    }

    // The monoid sequence is passed by reference first so that its root can be
    // updated if the root is replaced and second so that we can construct the
    // return value.
    typename monoid_sequence::iterator insert_after(const T&difference, monoid_sequence&sequence) {
      assert(is_leaf());
      vertex*result = new vertex{difference};
      new vertex{this, result, this, sequence.root};
      return {&sequence, result};
    }

    // The monoid sequence is passed by reference first so that its root can be
    // updated if the root is replaced and second so that we can construct the
    // return value.
    void remove(monoid_sequence&sequence) {
      assert(is_leaf());
      if (parent) {
	vertex*grandparent = parent->parent;
	vertex*sibling = (this == parent->left) ? parent->right : parent->left;
	if (grandparent) {
	  if (grandparent->left == parent) {
	    grandparent->left = sibling;
	  } else {
	    assert(grandparent->right == parent);
	    grandparent->right = sibling;
	  }
	}
	sibling->parent = grandparent;
	sibling->recompute_ancestor_differences_and_subtree_sizes();
	if (grandparent) {
	  grandparent->balance(sequence.root);
	}
	if (sequence.root == parent) {
	  sequence.root = sibling;
	}
	parent->left = nullptr;
	parent->right = nullptr;
	delete parent;
      } else {
	sequence.root = nullptr;
      }
      delete this;
    }
  }*					root;

public:
  /* And this is a saturating bidirectional forward iterator over the AVL tree
   * leaves.
   */
  class iterator {
    friend class monoid_sequence;
  protected:
    const monoid_sequence*		sequence;
    vertex*				position;

  public:
    iterator(const monoid_sequence*sequence, vertex*position) : sequence(sequence), position(position) {}

    const T&operator *() const {
      return position->get_difference();
    }
    const T*operator ->() const {
      return &(position->get_difference());
    }

    // const monoid_sequence*get_owner() const {
    //   return sequence;
    // }

    bool can_decrement() const {
      return position->get_previous();
    }
    iterator&operator --() {
      if (position) {
	vertex*candidate = position->get_previous();
	// Forbid decrements beyond the beginning.
	if (candidate) {
	  position = candidate;
	}
      } else if (sequence->root) {
	position = sequence->root->get_rightmost_descendant();
      }
      return *this;
    }
    iterator operator --(int) {
      iterator copy = *this;
      --*this;
      return copy;
    }

    bool can_increment() const {
      return position;
    }
    iterator&operator ++() {
      if (position) {
	position = position->get_next();
      } // No else; forbid increments beyond the end.
      return *this;
    }
    iterator operator ++(int) {
      iterator copy = *this;
      ++*this;
      return copy;
    }

    bool operator ==(const iterator&other) const {
      return (position == other.position) && (sequence == other.sequence);
    }
    bool operator !=(const iterator&other) const {
      return !(*this == other);
    }
    bool operator <(const iterator&other) const {
      if (position) {
	if (other.position) {
	  return *position < *other.position;
	}
	return true;
      }
      return false;
    }
  };

public:
  monoid_sequence() :
    root{nullptr} {}
  ~monoid_sequence() {
    if (root) {
      delete root;
    }
  }

  bool empty() const {
    return !root;
  }

  iterator begin() const {
    if (root) {
      return iterator(this, root->get_leftmost_descendant());
    }
    return iterator(this, nullptr);
  }
  iterator end() const {
    return iterator(this, nullptr);
  }

  iterator find(const T&target) const {
    if (!root) {
      return iterator(this, nullptr);
    }
    return iterator(this, root->get_leftmost_strictly_to_right(target));
  }

  iterator insert(const iterator&position, const T&difference) {
    assert(position.sequence == this);
    if (position.position) {
      iterator result = position.position->insert_before(difference, *this);
      for (vertex*parent; (parent = root->get_parent()); root = parent);
      return result;
    }
    if (root) {
      iterator result = root->get_rightmost_descendant()->insert_after(difference, *this);
      for (vertex*parent; (parent = root->get_parent()); root = parent);
      return result;
    }
    return {this, root = new vertex{difference}};
  }

  iterator erase(const iterator&position) {
    assert(position.sequence == this);
    assert(position.position);
    iterator result = position;
    ++result;
    position.position->remove(*this);
    if (position.position == root) {
      assert(!result.position);
      root = nullptr;
    }
    return result;
  }

  T sum_over_interval(const iterator&left_inclusive, const iterator&right_exclusive) const {
    assert(left_inclusive.sequence == this);
    assert(right_exclusive.sequence == this);
    if (root && left_inclusive.position) {
      return left_inclusive.position->get_sum_until(right_exclusive.position);
    }
    return 0;
  }
};

#endif
