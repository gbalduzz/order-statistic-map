// Copyright (C) 2020 Giovanni Balduzzi
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

#include "random_access_map.hpp"

namespace ramlib {

// Precondition: elements of type Key have full order.
template <class Key, std::size_t chunk_size = 64>
class RandomAccessSet {
public:
  RandomAccessSet() = default;
  RandomAccessSet(const std::initializer_list<Key>& list);
  RandomAccessSet(const std::vector<Key>& linearized);

  RandomAccessSet(const RandomAccessSet& rhs) = default;
  RandomAccessSet(RandomAccessSet&& rhs) = default;
  RandomAccessSet& operator=(const RandomAccessSet& rhs) = default;
  RandomAccessSet& operator=(RandomAccessSet&& rhs) = default;

  // Insert new key. Returns false if the key is already present.
  bool insert(const Key& key) noexcept;

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
  const Key& findByIndex(const std::size_t index) const;

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
  struct Null {
    Null() = default;
  };

  RandomAccessMap<Key, Null, chunk_size> map_;
};

template <class Key, std::size_t chunk_size>
RandomAccessSet<Key, chunk_size>::RandomAccessSet(const std::initializer_list<Key>& list) {
  for (const auto& k : list)
    map_.insert(k, {});
}

template <class Key, std::size_t chunk_size>
RandomAccessSet<Key, chunk_size>::RandomAccessSet(const std::vector<Key>& linearized) {
  for (const auto& k : linearized)
    map_.insert(k, {});
}

template <class Key, std::size_t chunk_size>
bool RandomAccessSet<Key, chunk_size>::insert(const Key& key) noexcept {
  return map_.insert(key, {}).second;
}

template <class Key, std::size_t chunk_size>
bool RandomAccessSet<Key, chunk_size>::erase(const Key& key) noexcept {
  return map_.erase(key);
}

template <class Key, std::size_t chunk_size>
bool RandomAccessSet<Key, chunk_size>::contains(const Key& key) const noexcept {
  return map_.contains(key);
}

template <class Key, std::size_t chunk_size>
const Key& RandomAccessSet<Key, chunk_size>::findByIndex(const std::size_t index) const {
  auto it = map_.findByIndex(index);
  assert(it);
  return it->first;
}

template <class Key, std::size_t chunk_size>
std::vector<Key> RandomAccessSet<Key, chunk_size>::linearize() const noexcept {
  std::vector<Key> result;
  result.reserve(size());

  for (auto it : map_)
    result.push_back(it.first);
  return result;
}

}  // namespace ramlib
