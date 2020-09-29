// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Provides a set with O(log n) insertion, removal and random access, i.e. access to the i-th lowest key.
// Useful for a random selection in an ordered list with variable size.
// Implemented as a map with a null value.

#pragma once

#include "order_statistic_map.hpp"

namespace maplib {

// Precondition: elements of type Key have full order.
template <class Key, std::size_t chunk_size = 64>
class OrderStatisticSet {
private:
  struct Null {
    Null() = default;
  };

public:
  using Node = details::Node<Key, Null>;
  using const_iterator = MapIterator<Node, true>;
  using iterator = MapIterator<Node, false>;

  OrderStatisticSet() = default;
  OrderStatisticSet(const std::initializer_list<Key>& list);
  OrderStatisticSet(const std::vector<Key>& linearized);

  OrderStatisticSet(const OrderStatisticSet& rhs) = default;
  OrderStatisticSet(OrderStatisticSet&& rhs) = default;
  OrderStatisticSet& operator=(const OrderStatisticSet& rhs) = default;
  OrderStatisticSet& operator=(OrderStatisticSet&& rhs) = default;

  auto begin() const noexcept {
    return map_.begin();
  };
  auto end() const noexcept {
    return map_.end();
  };

  auto begin() noexcept {
    return map_.begin();
  };
  auto end() noexcept {
    return map_.end();
  };

  // Insert new key. Returns false if the key is already present.
  auto insert(const Key& key) noexcept -> std::pair<iterator, bool>;

  // Remove the node relative to key. Returns true if the key was present.
  // Returns false and leave the container unchanged otherwise.
  bool erase(const Key& key) noexcept;

  // Returns true if the key is present.
  bool contains(const Key& key) const noexcept;
  bool count(const Key& key) const noexcept {
    return contains(key);
  }

  // Returns the "index"-th lowest key.
  // Precondition: 0 <= index < size()
  const Key& findByIndex(const std::size_t index) const noexcept;

  // Number of keys stored in the map.
  std::size_t size() const noexcept {
    return map_.size();
  }

  // Returns an array of ordered keys.
  std::vector<Key> linearize() const noexcept;

  bool checkConsistency() const noexcept {
    return map_.checkConsistency();
  }

private:
  OrderStatisticMap<Key, Null, chunk_size> map_;
};

template <class Key, std::size_t chunk_size>
OrderStatisticSet<Key, chunk_size>::OrderStatisticSet(const std::initializer_list<Key>& list) {
  for (const auto& k : list)
    map_.insert(k, {});
}

template <class Key, std::size_t chunk_size>
OrderStatisticSet<Key, chunk_size>::OrderStatisticSet(const std::vector<Key>& linearized) {
  for (const auto& k : linearized)
    map_.insert(k, {});
}

template <class Key, std::size_t chunk_size>
auto OrderStatisticSet<Key, chunk_size>::insert(const Key& key) noexcept
    -> std::pair<iterator, bool> {
  return map_.insert(key, {});
}

template <class Key, std::size_t chunk_size>
bool OrderStatisticSet<Key, chunk_size>::erase(const Key& key) noexcept {
  return map_.erase(key);
}

template <class Key, std::size_t chunk_size>
bool OrderStatisticSet<Key, chunk_size>::contains(const Key& key) const noexcept {
  return map_.contains(key);
}

template <class Key, std::size_t chunk_size>
const Key& OrderStatisticSet<Key, chunk_size>::findByIndex(const std::size_t index) const noexcept {
  auto it = map_.findByIndex(index);
  assert(it);
  return it->first;
}

template <class Key, std::size_t chunk_size>
std::vector<Key> OrderStatisticSet<Key, chunk_size>::linearize() const noexcept {
  std::vector<Key> result;
  result.reserve(size());

  std::function<void(const Node*)> inorder = [&](const Node* node) {
    if (node->left)
      inorder(node->left);

    result.push_back(node->data.first);

    if (node->right)
      inorder(node->right);
  };

  if (map_.root_)
    inorder(map_.root_);

  return result;
}

}  // namespace maplib
