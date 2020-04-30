// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Bidirectional iterator for the map classes.

#pragma once

#include <functional>

namespace ramlib {

template <class Node, bool is_const>
class MapIterator {
  template <class A, class B>
  using Conditional = std::conditional_t<is_const, A, B>;

public:
  using Key = typename Node::Key;
  using Value = typename Node::Value;

  using iteator_category = std::bidirectional_iterator_tag;
  using value_type = std::pair<const Key, Value>;
  using pointer = Conditional<const value_type*, value_type>;
  using reference = Conditional<const value_type&, value_type&>;
  using difference_type = std::size_t;

  MapIterator(Conditional<const Node*, Node*> node) : node_(node) {}

  // Convert non-const to const
  template <bool c = is_const, typename = std::enable_if_t<c>>
  MapIterator(const MapIterator<Node, false>& rhs) : node_(rhs.node_) {}

  auto operator*() const {
    assert(node_);
    return std::make_pair(std::cref(node_->data.first), std::cref(node_->data.second));
  }
  template <bool c = is_const, typename = std::enable_if_t<!c>>
  auto operator*() {
    assert(node_);
    return std::make_pair(std::cref(node_->data.first), std::ref(node_->data.second));
  }

  const std::pair<const Key, Value>* operator->() const {
    assert(node_);
    return reinterpret_cast<const std::pair<const Key, Value>*>(&node_->data);
  }
  template <bool c = is_const, typename = std::enable_if_t<!c>>
  std::pair<const Key, Value>* operator->() {
    assert(node_);
    return reinterpret_cast<std::pair<const Key, Value>*>(&node_->data);
  }

  void next();

  void prev();

  MapIterator& operator++() {
    next();
    return *this;
  }

  MapIterator& operator--() {
    prev();
    return *this;
  }

  operator bool() const {
    return static_cast<bool>(node_);
  }

  bool operator==(const MapIterator& rhs) const {
    return node_ == rhs.node_;
  }
  bool operator!=(const MapIterator& rhs) const {
    return node_ != rhs.node_;
  }

  // Grant access of the node to the container.
  template <class K, class V, std::size_t s>
  friend class RandomAccessMap;
  // Grant access to the const or non-const version.
  template <class N, bool c>
  friend class MapIterator;
  template <class N, bool c>
  friend class SamplingMapIterator;

private:
  Conditional<const Node*, Node*> node_ = nullptr;
};

template <class Node, bool is_const>
void MapIterator<Node, is_const>::next() {
  if (!node_)
    throw(std::logic_error("Advancing end iterator."));

  auto is_right_child = [&]() { return node_->parent && node_->parent->right == node_; };

  if (node_->right) {  // Minimum of right subtree.
    node_ = node_->right;
    while (node_->left)
      node_ = node_->left;
  }
  else {  // Go up until we go up and left.
    while (is_right_child())
      node_ = node_->parent;
    node_ = node_->parent;
  }
}

template <class Node, bool is_const>
void MapIterator<Node, is_const>::prev() {
  if (!node_)
    throw(std::logic_error("Decrementing null iterator."));

  auto is_left_child = [&]() { return node_->parent && node_->parent->left == node_; };

  if (node_->left) {  // Maximum of left subtree.
    node_ = node_->left;
    while (node_->right)
      node_ = node_->right;
  }
  else {  // Go up until we go up and right.
    while (is_left_child())
      node_ = node_->parent;
    node_ = node_->parent;
  }
}

}  // namespace ramlib
