// Copyright (C) 2020 ETH Zurich
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Internal node for RandomAccessMap.

#pragma once

namespace ramlib {
namespace details {

enum Color : std::uint8_t { RED, BLACK };

template <class Key, class Value>
struct Node {
  Node(const Key& k, const Value& v, Node* p) : parent(p), data(k, v) {}

  Node* left = nullptr;
  Node* right = nullptr;
  Node* parent = nullptr;

  std::size_t subtree_size = 1;

  std::pair<Key, Value> data;

  Color color = RED;
};

}  // namespace details
}  // namespace ramlib
