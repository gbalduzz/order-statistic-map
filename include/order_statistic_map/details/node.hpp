// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Internal node for RandomAccessMap.

#pragma once

#include "color.hpp"

namespace maplib {
namespace details {

template <class _Key, class _Value>
struct Node {
  using Key = _Key;
  using Value = _Value;

  Node(const Key& k, const Value& v, Node* p) : parent(p), data(k, v) {}

  void updateSubtreeWeight();
  void swapMetadata(Node& rhs) {
    std::swap(subtree_size, rhs.subtree_size);
  }

  Node* left = nullptr;
  Node* right = nullptr;
  Node* parent = nullptr;

  std::size_t subtree_size = 1;

  std::pair<Key, Value> data;

  Color color = RED;
};

template <class Key, class Value>
void Node<Key, Value>::updateSubtreeWeight() {
  subtree_size = 1;
  if (left)
    subtree_size += left->subtree_size;
  if (right)
    subtree_size += right->subtree_size;
}

}  // namespace details
}  // namespace maplib
