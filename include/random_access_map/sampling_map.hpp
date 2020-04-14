// Copyright (C) 2020 Giovanni Balduzzi
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Provides a map with O(log n) insertion, removal and random access, i.e. access of the value
// relative to the i-th lowest key.
// Useful for a random selection in an ordered list with variable size.
// Implemented as an augmented red-black tree.

#pragma once

#include <cassert>
#include <initializer_list>
#include <random>
#include <stack>
#include <stdexcept>
#include <vector>
#include <tuple>

#include "sampling_map_iterator.hpp"
#include "details/fixed_size_allocator.hpp"
#include "details/node_operations.hpp"
#include "details/weighted_node.hpp"

namespace ramlib {

// Precondition: elements of type Key have full order.
template <class Key, class Value, class Weight, std::size_t chunk_size = 64>
class SamplingMap {
public:
  using Node = details::WeightedNode<Key, Value, Weight>;
  using const_iterator = SamplingMapIterator<Node, true>;
  using iterator = SamplingMapIterator<Node, false>;

  SamplingMap() = default;
  SamplingMap(const std::initializer_list<std::tuple<Key, Value, Weight>>& list);
  SamplingMap(const std::vector<std::tuple<Key, Value, Weight>>& linearized);

  SamplingMap(const SamplingMap& rhs);
  SamplingMap(SamplingMap&& rhs);

  SamplingMap& operator=(const SamplingMap& rhs);
  SamplingMap& operator=(SamplingMap&& rhs);

  ~SamplingMap();

  auto begin() const noexcept -> const_iterator;
  auto end() const noexcept -> const_iterator;

  auto begin() noexcept -> iterator;
  auto end() noexcept -> iterator;

  // Insert new key, value pair if key is not already present, and returns an iterator to the node
  // and true.
  // If the key is already present, update the value and returns an iterator to the node and false.
  auto insert(const Key& key, const Value& value, const Weight& weight) noexcept
      -> std::pair<iterator, bool>;
  auto insert(const std::tuple<Key, Value, Weight>& values) noexcept {
    insert(std::get<0>(values), std::get<1>(values), std::get<2>(values));
  }

  // Remove the node relative to key.
  // Returns: true if the key is found and removed. False if no operation is performed.
  bool erase(const Key& key) noexcept;

  // Remove the node.
  // Precondition: the node is in the map.
  void erase(iterator it);

  // Returns the iterator associated with key.
  // If the key is not in the map, returns a null iterator.
  auto findByKey(const Key& key) const noexcept -> const_iterator;
  auto findByKey(const Key& key) noexcept -> iterator;

  // Returns true if a stored key compares equal to the argument.
  bool contains(const Key& key) const noexcept;
  bool count(const Key& key) const noexcept {
    return contains(key);
  }

  // Returns an iterator relative to a node sampled with probability proportional to its weight.
  template <class Rng>
  auto sample(Rng& rng) const noexcept -> const_iterator;
  template <class Rng>
  auto sample(Rng& rng) noexcept -> iterator;

  // Returns an array of ordered keys and value pairs.
  std::vector<std::tuple<Key, Value, Weight>> linearize() const noexcept;

  std::size_t size() const noexcept {
    return size_;
  }

  Weight totalWeight() const noexcept {
    return root_ ? root_->subtree_weight : 0;
  }

  // For testing purposes.
  bool checkConsistency() const noexcept;

private:
  constexpr static auto BLACK = details::BLACK;
  constexpr static auto RED = details::RED;

  // Members
  Node* root_ = nullptr;
  std::size_t size_ = 0;
  details::FixedSizeAllocator<Node, chunk_size> allocator_;
};

template <class Key, class Value, class Weight, std::size_t chunk_size>
SamplingMap<Key, Value, Weight, chunk_size>::SamplingMap(
    const std::initializer_list<std::tuple<Key, Value, Weight>>& list) {
  for (const auto& elem : list)
    insert(elem);
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
SamplingMap<Key, Value, Weight, chunk_size>::SamplingMap(
    const std::vector<std::tuple<Key, Value, Weight>>& linearized) {
  for (const auto& elem : linearized)
    insert(elem);
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
SamplingMap<Key, Value, Weight, chunk_size>::~SamplingMap() {
  std::stack<Node*> to_delete;
  if (root_)
    to_delete.push(root_);

  while (!to_delete.empty()) {
    auto node = to_delete.top();
    to_delete.pop();

    if (node->left)
      to_delete.push(node->left);
    if (node->right)
      to_delete.push(node->right);

    allocator_.destroy(node);
  }
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
SamplingMap<Key, Value, Weight, chunk_size>::SamplingMap(const SamplingMap& rhs) {
  (*this) = rhs;
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
SamplingMap<Key, Value, Weight, chunk_size>::SamplingMap(SamplingMap&& rhs) {
  (*this) = std::move(rhs);
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
SamplingMap<Key, Value, Weight, chunk_size>& SamplingMap<Key, Value, Weight, chunk_size>::operator=(
    const SamplingMap<Key, Value, Weight, chunk_size>& rhs) {
  if (this != &rhs) {
    *this = std::move(SamplingMap());  // clear content.

    for (auto it = rhs.begin(); it != rhs.end(); ++it)
      insert(it->first, it->second, it.getWeight());
  }
  return *this;
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
SamplingMap<Key, Value, Weight, chunk_size>& SamplingMap<Key, Value, Weight, chunk_size>::operator=(
    SamplingMap<Key, Value, Weight, chunk_size>&& rhs) {
  std::swap(root_, rhs.root_);
  std::swap(allocator_, rhs.allocator_);
  return *this;
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
auto SamplingMap<Key, Value, Weight, chunk_size>::insert(const Key& key, const Value& val,
                                                         const Weight& weight) noexcept
    -> std::pair<iterator, bool> {
  if (!root_) {
    root_ = allocator_.create(key, val, weight, nullptr);
    root_->color = BLACK;
    ++size_;
    return {iterator(root_), true};
  }

  Node* node = root_;
  bool done = false;

  while (!done) {
    if (key == get_key(node)) {  // Key is already present. Undo changes and return.
      node->data.second = val;
      iterator return_it = iterator(node);

      node = node->parent;
      while (node) {
        node->subtree_weight -= weight;
        node = node->parent;
      }

      return {return_it, false};
    }
    node->subtree_weight += weight;

    if (key < get_key(node)) {
      if (node->left == nullptr) {
        node->left = allocator_.create(key, val, weight, node);
        done = true;
      }
      node = node->left;
    }
    else {
      if (node->right == nullptr) {
        node->right = allocator_.create(key, val, weight, node);
        done = true;
      }
      node = node->right;
    }
  }

  // Check colors
  details::fixRedRed(node, root_);

  //  assert(checkConsistency());
  ++size_;
  return {iterator(node), true};
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
bool SamplingMap<Key, Value, Weight, chunk_size>::erase(const Key& key) noexcept {
  if (!root_)
    return false;
  Node* to_delete = root_;

  // Search while updating subtree count.
  bool found = false;

  while (true) {
    if (key == get_key(to_delete)) {
      found = true;
      break;
    }

    if (key < get_key(to_delete)) {
      if (!to_delete->left)
        break;
      to_delete = to_delete->left;
    }
    else {
      if (!to_delete->right)
        break;
      to_delete = to_delete->right;
    }
  }

  if (!found) {
    return false;
  }

  erase(iterator(to_delete));

  return true;
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
void SamplingMap<Key, Value, Weight, chunk_size>::erase(const iterator it) {
  Node* to_delete = it.node_;

  // Update upstream weights
  Node* const original = to_delete;
  const Weight w_original = original->weight;
  bool double_children = false;

  if (to_delete->left != nullptr && to_delete->right != nullptr) {  // to_delete has two children.
    double_children = true;
    Node* const original = to_delete;
    to_delete = to_delete->right;
    while (to_delete->left) {
      to_delete = to_delete->left;
    }
  }

  const Weight w = to_delete->weight;

  // Update upstream weight.
  Node* ancestor = original;
  while (ancestor) {
    ancestor->subtree_weight -= w_original;
    ancestor = ancestor->parent;
  }

  if (double_children) {  // Update original and downstream.
    original->data = std::move(to_delete->data);
    original->weight = to_delete->weight;

    Node* ancestor = to_delete;
    while (ancestor != original) {
      ancestor->subtree_weight -= w;
      ancestor = ancestor->parent;
    }
  }

  removeNoDoubleChild(to_delete, root_);

  --size_;
  allocator_.destroy(to_delete);

  //  assert(checkConsistency());
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
template <class Rng>
auto SamplingMap<Key, Value, Weight, chunk_size>::sample(Rng& rng) noexcept -> iterator {
  if (!root_)
    return iterator(nullptr);

  Weight scaled;
  const Weight total_weight = root_->subtree_weight;

  if constexpr (std::is_floating_point_v<Weight>) {
    scaled = std::uniform_real_distribution<Weight>(0, total_weight)(rng);
  }
  else {  // is integer
    scaled = std::uniform_int_distribution<Weight>(0, total_weight - 1)(rng);
  }

  Weight on_the_left(0);
  Node* node = root_;

  while (true) {
    assert(node);

    auto new_on_the_left = on_the_left;
    if (node->left)
      new_on_the_left += node->left->subtree_weight;

    if (scaled >= new_on_the_left && scaled < new_on_the_left + node->weight) {
      return iterator(node);
    }
    else if (scaled < new_on_the_left) {  // go left
      node = node->left;
    }
    else {  // go right
      on_the_left = new_on_the_left + node->weight;
      node = node->right;
    }
  }
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
template <class Rng>
auto SamplingMap<Key, Value, Weight, chunk_size>::sample(Rng& rng) const noexcept -> const_iterator {
  return const_cast<SamplingMap&>(*this).sample(rng);
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
auto SamplingMap<Key, Value, Weight, chunk_size>::findByKey(const Key& key) noexcept -> iterator {
  Node* node = root_;
  while (node) {
    if (get_key(node) == key)
      return iterator(node);
    else if (key < get_key(node))
      node = node->left;
    else
      node = node->right;
  }

  // Key not found.
  return iterator(nullptr);
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
auto SamplingMap<Key, Value, Weight, chunk_size>::findByKey(const Key& key) const noexcept
    -> const_iterator {
  // Avoid code duplication with a cast to non-const (const iterator does not allow data modification).
  return const_iterator(const_cast<SamplingMap&>(*this).findByKey(key));
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
bool SamplingMap<Key, Value, Weight, chunk_size>::contains(const Key& key) const noexcept {
  const Node* node = root_;
  while (node) {
    if (get_key(node) == key)
      return true;
    else if (key < get_key(node))
      node = node->left;
    else
      node = node->right;
  }

  return false;
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
std::vector<std::tuple<Key, Value, Weight>> SamplingMap<Key, Value, Weight, chunk_size>::linearize() const
    noexcept {
  std::vector<std::tuple<Key, Value, Weight>> result;
  result.reserve(size());

  for (auto it = begin(); it != end(); ++it)
    result.emplace_back(it->first, it->second, it.getWeight());

  return result;
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
bool SamplingMap<Key, Value, Weight, chunk_size>::checkConsistency() const noexcept {
  bool child_parent_violation = false;
  bool red_red_violation = false;
  bool black_count_violation = false;
  bool subtree_weight_violation = false;

  // Returns size of subtree.
  std::function<Weight(const Node*)> subtree_weight = [&](const Node* node) -> Weight {
    if (!node)
      return 0;
    return node->weight + subtree_weight(node->left) + subtree_weight(node->right);
  };

  auto similar = [](Weight a, Weight b) {
    if constexpr (std::is_floating_point_v<Weight>) {
      return std::abs(a - b) < std::numeric_limits<Weight>::epsilon() * 100;
    }
    else {
      return a == b;
    }
  };

  // Check node consistency and returns number of black nodes in [node, leaves].
  std::function<int(const Node*)> check = [&](const Node* node) {
    if (node == nullptr)
      return 1;

    // Check parent-child relationship.
    if (node->left && node->left->parent != node)
      child_parent_violation = true;
    if (node->right && node->right->parent != node)
      child_parent_violation = true;

    // Check subtree size
    if (!similar(node->subtree_weight, subtree_weight(node)))
      subtree_weight_violation = true;

    // Check double red
    auto color = [&](const Node* n) { return n ? n->color : BLACK; };
    if (node->color == RED && (color(node->left) == RED || color(node->right) == RED))
      red_red_violation = true;

    // Check black count
    int count_left = check(node->left);
    int count_right = check(node->right);

    if (count_left != count_right)
      black_count_violation = true;

    return count_left + node->color == BLACK ? 1 : 0;
  };

  check(root_);

  return !black_count_violation && !red_red_violation && !child_parent_violation &&
         !subtree_weight_violation;
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
auto SamplingMap<Key, Value, Weight, chunk_size>::begin() noexcept -> iterator {
  if (!root_)
    return iterator{nullptr};

  Node* node = root_;
  while (node->left)
    node = node->left;

  return iterator(node);
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
auto SamplingMap<Key, Value, Weight, chunk_size>::end() noexcept -> iterator {
  return iterator{nullptr};
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
auto SamplingMap<Key, Value, Weight, chunk_size>::begin() const noexcept -> const_iterator {
  return const_cast<SamplingMap&>(*this).begin();
}

template <class Key, class Value, class Weight, std::size_t chunk_size>
auto SamplingMap<Key, Value, Weight, chunk_size>::end() const noexcept -> const_iterator {
  return iterator{nullptr};
}

}  // namespace ramlib
