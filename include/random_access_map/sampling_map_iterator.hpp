// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Bidirectional iterator for the sampling map class.

#pragma once

#include <functional>

#include "map_iterator.hpp"

namespace ramlib {

template <class Node, bool is_const>
class SamplingMapIterator final : public MapIterator<Node, is_const> {
public:
  using Base = MapIterator<Node, is_const>;
  using Weight = typename Node::Weight;
  using Base::node_;

  SamplingMapIterator(std::conditional_t<is_const, const Node*, Node*> node = nullptr) : Base(node) {}

  template <bool other_c>
  SamplingMapIterator(const SamplingMapIterator<Node, other_c>& rhs) : Base(rhs) {}

  Weight getWeight() const {
    return Base::node_->weight;
  }

  Weight getSubtreeWeight() const {
    return Base::node_->subtree_weight;
  }

  void setWeight(const Weight weight) {
    const Weight diff = weight - node_->weight;
    if (diff) {
      node_->weight = weight;

      auto ancestor = node_;
      while (ancestor) {
        ancestor->subtree_weight += diff;
        ancestor = ancestor->parent;
      }
    }
  }
};

}  // namespace ramlib
