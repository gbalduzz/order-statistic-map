// Copyright (C) 2020 Giovanni Balduzzi
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Internal node for RandomAccessMap.

#pragma once

#include "color.hpp"

namespace ramlib {
namespace details {

template <class _Key, class _Value, class _Weight>
struct WeightedNode {
  using Key = _Key;
  using Value = _Value;
  using Weight = _Weight;

  WeightedNode(const Key& k, const Value& v, const Weight w, WeightedNode* p)
      : parent(p), weight(w), subtree_weight(w), data(k, v) {}

  void updateSubtreeWeight();
  void swapMetadata(WeightedNode& rhs) {}

  WeightedNode* left = nullptr;
  WeightedNode* right = nullptr;
  WeightedNode* parent = nullptr;

  Weight weight;
  Weight subtree_weight;

  std::pair<Key, Value> data;

  Color color = RED;
};

template <class Key, class Value, class Weight>
void WeightedNode<Key, Value, Weight>::updateSubtreeWeight() {
  subtree_weight = weight;
  if (left)
    subtree_weight += left->subtree_weight;
  if (right)
    subtree_weight += right->subtree_weight;
}

}  // namespace details
}  // namespace ramlib
